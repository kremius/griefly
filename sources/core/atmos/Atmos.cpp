#include "Atmos.h"

#include <QElapsedTimer>

#include <algorithm>

#include "../SyncRandom.h"
#include "../Helpers.h"

#include "AtmosGrid.h"

#include "representation/Text.h"

Atmosphere::Atmosphere(SyncRandom* random, IMapMaster* map, TextPainter *texts)
    : random_(random),
      map_(map),
      texts_(texts)
{
    grid_processing_ns_ = 0;
    movement_processing_ns_ = 0;

    x_size_ = 0;
    y_size_ = 0;
    z_size_ = 0;

    (*texts_)["{Perf}AtmosGridProcessing"].SetUpdater
    ([&](QString* str)
    {
        *str = QString("Atmos grid processing: %1 ms")
            .arg((grid_processing_ns_ * 1.0) / 1000000.0);
    }).SetFreq(1000);

    (*texts_)["{Perf}AtmosMove"].SetUpdater
    ([&](QString* str)
    {
        *str = QString("Atmos move processing: %1 ms")
            .arg((movement_processing_ns_ * 1.0) / 1000000.0);
    }).SetFreq(1000);

    qDebug() << "Atmosphere load";
    z_size_ = 1;
    grid_ = nullptr;
}

void Atmosphere::Resize(quint32 x, quint32 y, quint32 z)
{
    x_size_ = x;
    y_size_ = y;
    z_size_ = z;

    delete grid_;
    grid_ = new AtmosGrid(random_, x_size_, y_size_);
}

void Atmosphere::Process()
{
    QElapsedTimer timer;
    timer.start();
    grid_->Process();
    grid_processing_ns_ = (grid_processing_ns_ + timer.nsecsElapsed()) / 2;
    timer.start();
}

const int PRESSURE_MOVE_BORDER = 1000;
const int FLOW_MOVE_BORDER = -15;

void Atmosphere::ProcessTileMove(int x, int y, int z)
{   
    auto tile = map_->GetSquares()[x][y][z];

    if (tile->GetTurf()->GetAtmosState() == NON_SIMULATED)
    {
        return;
    }

    AtmosGrid::Cell& cell = grid_->At(x, y);

    VDir force;

    for (int dir = 0; dir < atmos::DIRS_SIZE; ++dir)
    {
        int flow = cell.flows[dir];
        cell.flows[dir] = 0;
        if (flow  <= FLOW_MOVE_BORDER)
        {
            VDir local = DirToVDir[atmos::INDEXES_TO_DIRS[dir]];
            force.x += local.x;
            force.y += local.y;
        }
    }

    if (IsNonZero(force))
    {
        if (tile->GetInsideList().size())
        {
            auto i = tile->GetInsideList().rbegin();
            while (   (i != tile->GetInsideList().rend())
                   && ((*i)->passable_level == Passable::EMPTY))
            {
                ++i;
            }
            if (i != tile->GetInsideList().rend())
            {
                (*i)->ApplyForce(force);
            }
        }
    }

    // TODO: borders
    if (x == 0 || x >= x_size_ - 1)
    {
        return;
    }
    if (y == 0 || y >= y_size_ - 1)
    {
        return;
    }

    if (!cell.IsPassable(atmos::CENTER))
    {
        return;
    }

    for (int dir = 0; dir < atmos::DIRS_SIZE; ++dir)
    {
        AtmosGrid::Cell& nearby = grid_->Get(x, y, atmos::DIRS[dir]);
        if (  (nearby.data.pressure + PRESSURE_MOVE_BORDER)
            < cell.data.pressure)
        {
            if (!cell.IsPassable(atmos::DIRS[dir]))
            {
                Dir bump_dir = atmos::INDEXES_TO_DIRS[dir];
                tile->BumpByGas(bump_dir, true);
                continue;
            }
        }
        else if (  (cell.data.pressure + PRESSURE_MOVE_BORDER)
                 < nearby.data.pressure)
        {
            if (nearby.IsPassable(atmos::REVERT_DIRS[dir]))
            {
                if (!cell.IsPassable(atmos::DIRS[dir]))
                {
                    Dir revert_dir = atmos::REVERT_DIRS_INDEXES[dir];
                    Dir bump_dir = atmos::INDEXES_TO_DIRS[revert_dir];
                    tile->BumpByGas(bump_dir, true);
                    continue;
                }
            }
        }
    }
}

void Atmosphere::ProcessMove()
{
    QElapsedTimer timer;
    timer.start();

    for (int z = 0; z < z_size_; ++z)
    {
        for (int x = 0; x < x_size_; ++x)
        {
            for (int y = 0; y < y_size_; ++y)
            {
                ProcessTileMove(x, y, z);
            }
        }
    }
    movement_processing_ns_
            = (movement_processing_ns_ + timer.nsecsElapsed()) / 2;
}

void Atmosphere::SetFlags(quint32 x, quint32 y, quint32 z, IAtmosphere::Flags flags)
{
    grid_->At(x, y).flags = flags;
}

void Atmosphere::LoadGrid()
{
    auto& squares = map_->GetSquares();
    for (int z = 0; z < z_size_; ++z)
    {
        for (int x = 0; x < x_size_; ++x)
        {
            for (int y = 0; y < y_size_; ++y)
            {
                auto& tile = squares[x][y][z];
                tile->UpdateAtmosPassable();
                AtmosHolder* holder = tile->GetAtmosHolder();
                AtmosGrid::Cell& cell = grid_->At(x, y);

                cell.data = holder->data_;
                holder->SetAtmosData(&cell.data);
            }
        }
    }
}
