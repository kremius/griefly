#pragma once

#include <QDebug>

#include "AtmosConstants.h"
#include "KVAbort.h"
#include "Interfaces.h"

namespace atmos
{
    const int DIRS_SIZE = 4;

    const IAtmosphere::Flags FULL = 0;
    const IAtmosphere::Flags UP = 1;
    const IAtmosphere::Flags DOWN = 2;
    const IAtmosphere::Flags LEFT = 4;
    const IAtmosphere::Flags RIGHT = 8;
    const IAtmosphere::Flags CENTER = 16;
    const IAtmosphere::Flags SPACE = 32;

    const IAtmosphere::Flags DIRS[DIRS_SIZE]
        = { LEFT, UP, DOWN, RIGHT };
    const IAtmosphere::Flags REVERT_DIRS[DIRS_SIZE]
        = { RIGHT, DOWN, UP, LEFT };
    const int REVERT_DIRS_INDEXES[DIRS_SIZE]
        = { 3, 2, 1, 0 };

    const Dir INDEXES_TO_DIRS[DIRS_SIZE]
        = { D_LEFT, D_UP, D_DOWN, D_RIGHT };
}

class AtmosGrid
{
public:
    struct Cell
    {
        AtmosData data;

        int flows[atmos::DIRS_SIZE];

        IAtmosphere::Flags flags;
        Cell()
        {
            for (int i = 0; i < GASES_NUM; ++i)
            {
                data.gases[i] = 0;
            }
            for (int i = 0; i < atmos::DIRS_SIZE; ++i)
            {
                flows[i] = 0;
            }
            flags = atmos::FULL;
            data.energy = 0;
            data.pressure = 0;
            data.temperature = 0;
            data.volume = 1;
        }
        inline char IsPassable(char dir)
        {
            return ~flags & dir;
        }
        inline void SetUnpassable(char dir)
        {
            flags |= dir;
        }
        inline void ResetPassable()
        {
            flags = atmos::FULL;
        }
        inline void Truncate()
        {
            for (int gas = 0; gas < GASES_NUM; ++gas)
            {
                data.gases[gas] = 0;
            }
            data.energy = 0;
        }
    };
    AtmosGrid(SyncRandom* random, int width, int height)
        : random_(random),
          width_(width),
          height_(height),
          length_(width_ * height_)
    {
        if (width_ < 2)
        {
            qDebug() << "Width too small for AtmosGrid!";
            KvAbort();
        }
        if (height_ < 2)
        {
            qDebug() << "Height too small for AtmosGrid!";
            KvAbort();
        }
        cells_ = new Cell[length_];
    }
    ~AtmosGrid()
    {
        delete[] cells_;
    }

    inline Cell& Get(int pos, IAtmosphere::Flags dir)
    {
        // TODO: Boundaries check?

        switch (dir)
        {
        case atmos::DOWN:  return cells_[pos + 1];
        case atmos::UP:    return cells_[pos - 1];
        case atmos::RIGHT: return cells_[pos + width_];
        case atmos::LEFT:  return cells_[pos - width_];
        default: break;
        }

        qDebug() << "AtmosGrid::Get, Unknown dir:" << dir;
        KvAbort();
        // Not reachable
        return cells_[pos];
    }
    inline Cell& Get(int x, int y, IAtmosphere::Flags dir)
    {
        return Get(y + x * height_, dir);
    }

    inline Cell& At(int x, int y)
    {
        return cells_[y + x * height_];
    }
    void Process();
private:
    void Prepare(int stage);
    void Finalize();

    SyncRandom* random_;

    int width_;
    int height_;
    int length_;
    Cell* cells_;
};
