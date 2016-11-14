#include <cmath>
#include <sstream>
#include <iostream>
#include <cassert>

#include <QDebug>

#include "Map.h"
#include "objects/MainObject.h"
#include "objects/Tile.h"
#include "Game.h"
#include "ObjectFactory.h"
#include "Helpers.h"
#include "AutogenMetadata.h"
#include "representation/Representation.h"

void MapMaster::FillAtmosphere()
{
    for (int z = 0; z < GetDepth(); ++z)
    {
        for (int x = 0; x < GetWidth(); ++x)
        {
            for (int y = 0; y < GetHeight(); ++y)
            {
                if (   squares_[x][y][z]->GetTurf()
                    && squares_[x][y][z]->GetTurf()->GetAtmosState() != SPACE
                    && squares_[x][y][z]->GetTurf()->GetAtmosState() != NON_SIMULATED)
                {
                    // It is not passable then still fill with air cause of doors
                    auto holder = squares_[x][y][z]->GetAtmosHolder();
                    AddDefaultValues(holder);
                }
            }
        }
    }
}

void MapMaster::Represent()
{
    if (!GetVisiblePoints())
    {
        return;
    }

    for (int i = 0; i < MAX_LEVEL; ++i)
    {
        auto it2 = GetVisiblePoints()->begin();
        while (it2 != GetVisiblePoints()->end())
        {
            auto sq = squares_[it2->posx][it2->posy][it2->posz];
            auto& in_list = sq->GetInsideList();

            for (auto list_it = in_list.begin(); list_it != in_list.end(); ++list_it)
            {
                if ((*list_it)->v_level == i)
                {
                    (*list_it)->Represent();
                }
            }

            auto trf = squares_[it2->posx][it2->posy][it2->posz]->GetTurf();
            if (trf.IsValid() && trf->v_level == i)
            {
                trf->Represent();
            }
            ++it2;
        }
    }
    auto it2 = GetVisiblePoints()->begin();
    while (it2 != GetVisiblePoints()->end())
    {
        auto sq = squares_[it2->posx][it2->posy][it2->posz];
        auto& in_list = sq->GetInsideList();

        for (auto list_it = in_list.begin(); list_it != in_list.end(); ++list_it)
        {
            if ((*list_it)->v_level >= MAX_LEVEL)
            {
                (*list_it)->Represent();
            }
        }

        auto trf = squares_[it2->posx][it2->posy][it2->posz]->GetTurf();
        if (trf.IsValid() && trf->v_level >= MAX_LEVEL)
        {
            trf->Represent();
        }
        ++it2;
    }
}

void MapMaster::ResizeMap(int new_map_x, int new_map_y, int new_map_z)
{
    squares_.resize(new_map_x);
    for (int x = 0; x < new_map_x; ++x)
    {
        squares_[x].resize(new_map_y);
        for (int y = 0; y < new_map_y; ++y)
        {
            squares_[x][y].resize(new_map_z);
        }
    }
    atmosphere_->Resize(new_map_x, new_map_y, new_map_z);
}

MapMaster::MapMaster(SyncRandom* sync_random, TextPainter* texts)
    : losf_(this),
      atmosphere_(new Atmosphere(sync_random, this, texts))
{
    visible_points_ = new std::list<PosPoint>;
}

MapMaster::~MapMaster()
{
    delete atmosphere_;
    delete visible_points_;
}

IAtmosphere& MapMaster::GetAtmosphere()
{
    return *atmosphere_;
}

std::vector<std::vector<std::vector<MapMaster::SqType>>>& MapMaster::GetSquares()
{
    return squares_;
}

const std::vector<std::vector<std::vector<MapMaster::SqType>>>& MapMaster::GetSquares() const
{
    return squares_;
}

int MapMaster::GetWidth() const
{
    return squares_.size();
}

int MapMaster::GetHeight() const
{
    return squares_[0].size();
}

int MapMaster::GetDepth() const
{
    return squares_[0][0].size();
}

bool MapMaster::CheckBorders(const int* x, const int* y, const int* z) const
{
    if (x)
    {
        if (*x >= GetWidth())
        {
            return false;
        }
        if (*x < 0)
        {
            return false;
        }
    }
    if (y)
    {
        if (*y >= GetHeight())
        {
            return false;
        }
        if (*y < 0)
        {
            return false;
        }
    }
    if (z)
    {
        if (*z >= GetDepth())
        {
            return false;
        }
        if (*z < 0)
        {
            return false;
        }
    }
    return true;
}

bool MapMaster::IsTransparent(int posx, int posy, int posz)
{
    if (!CheckBorders(&posx, &posy, &posz))
    {
        return false;
    }
    return squares_[posx][posy][posz]->IsTransparent();
}

bool MapMaster::IsTileVisible(quint32 tile_id)
{
    auto l = GetVisiblePoints();
    if (!l)
    {
        return false;
    }
    for (auto it = l->begin(); it != l->end(); ++it)
    {
        if (tile_id == squares_[it->posx][it->posy][it->posz].Id())
        {
            return true;
        }
    }
    return false;
}

std::list<PosPoint>* MapMaster::GetVisiblePoints()
{
    return visible_points_;
}

void MapMaster::CalculateVisisble(std::list<PosPoint> *retval, int posx, int posy, int posz)
{
    losf_.CalculateVisisble(retval, posx, posy, posz);
}

const int RAY_MULTIPLIER = 2;

int LOSfinder::pos2corner(int pos)
{
    return pos * RAY_MULTIPLIER;
}

int LOSfinder::corner2pos(int corner)
{
    return corner / RAY_MULTIPLIER;
}

int LOSfinder::sign(int value)
{
    if (value > 0)
    {
        return 1;
    }
    else if (value < 0)
    {
        return -1;
    }
    return 0;
}

bool LOSfinder::check_corner(PosPoint p)
{
    int x = corner2pos(p.posx);
    int y = corner2pos(p.posy);
    int z = corner2pos(p.posz);
    return map_->CheckBorders(&x, &y, &z);
}


PosPoint LOSfinder::corner_point2point(PosPoint p)
{
    PosPoint retval(corner2pos(p.posx), corner2pos(p.posy), corner2pos(p.posz));
    return retval;
}

bool LOSfinder::is_transparent(PosPoint p)
{
    PosPoint tilePoint = corner_point2point(p);
    return map_->IsTransparent(tilePoint.posx, tilePoint.posy, tilePoint.posz);
}

bool LOSfinder::bresen_x(PosPoint source, PosPoint target)
{
    int y = source.posy;
    int error = 0;
    int delta_x = std::abs(source.posx - target.posx);
    int deltaerr = std::abs(source.posy - target.posy);
    int deltastep = sign(target.posy - source.posy);
    int incrstep = sign(target.posx - source.posx);
    for (int x = source.posx; x != target.posx; x += incrstep)
    {
        if ((x % RAY_MULTIPLIER) == 0 && (y % RAY_MULTIPLIER) == 0)
        {
            // when in corner check side neighbours
            // if both of them are not transparent then corner is not transparent
            PosPoint left_neighbour(x + incrstep, y, source.posz);
            PosPoint right_neighbour(x, y + deltastep, source.posz);
            if (!is_transparent(left_neighbour) && !is_transparent(right_neighbour))
            {
                return false;
            }
        }
        else if (x % RAY_MULTIPLIER == 0)
        {
            // when ray hits an edge check both tiles - current and previous. Since ray travels through edge both of them 
            // must be transparent
            PosPoint left_neighbour(x - incrstep, y, source.posz);
            PosPoint right_neighbour(x + incrstep, y, source.posz);
            if (!is_transparent(left_neighbour) || !is_transparent(right_neighbour))
            {
                return false;
            }
        }
        else if (y % RAY_MULTIPLIER == 0)
        {
            // second case of edge handling
            PosPoint left_neighbour(x, y - deltastep, source.posz);
            PosPoint right_neighbour(x, y + deltastep, source.posz);
            if (!is_transparent(left_neighbour) || !is_transparent(right_neighbour))
            {
                return false;
            }
        }
        else
        {
            PosPoint new_point(x, y, source.posz);
            if (!is_transparent(new_point))
            {
                return false;
            }
        }

        error += deltaerr;
        if (error >= delta_x)
        {
            y += deltastep;
            error -= delta_x;
        }
    }

    return true;
}

bool LOSfinder::bresen_y(PosPoint source, PosPoint target)
{
    int x = source.posx;
    int error = 0;
    int delta_y = std::abs(source.posy - target.posy);
    int deltaerr = std::abs(source.posx - target.posx);
    int deltastep = sign(target.posx - source.posx);
    int incrstep = sign(target.posy - source.posy);
    for (int y = source.posy; y != target.posy; y += incrstep)
    {
        if ((x % RAY_MULTIPLIER) == 0 && (y % RAY_MULTIPLIER) == 0)
        {
            // when in corner check side neighbours
            // if both of them are not transparent then corner is not transparent
            PosPoint left_neighbour(x + deltastep, y, source.posz);
            PosPoint right_neighbour(x, y + incrstep, source.posz);
            if (!is_transparent(left_neighbour) && !is_transparent(right_neighbour))
            {
                return false;
            }
        }
        else if (x % RAY_MULTIPLIER == 0)
        {
            // when ray hits an edge check both tiles. Since ray travels through edge both of them 
            // must be transparent
            PosPoint left_neighbour(x - deltastep, y, source.posz);
            PosPoint right_neighbour(x + deltastep, y, source.posz);
            if (!is_transparent(left_neighbour) || !is_transparent(right_neighbour))
            {
                return false;
            }
        }
        else if (y % RAY_MULTIPLIER == 0)
        {
            // second case of edge handling
            PosPoint left_neighbour(x, y - incrstep, source.posz);
            PosPoint right_neighbour(x, y + incrstep, source.posz);
            if (!is_transparent(left_neighbour) || !is_transparent(right_neighbour))
            {
                return false;
            }
        }
        else
        {
            PosPoint new_point(x, y, source.posz);

            if (!is_transparent(new_point))
            {
                return false;
            }
        }

        error += deltaerr;
        if (error >= delta_y)
        {
            x += deltastep;
            error -= delta_y;
        }
    }

    return true;
}

bool LOSfinder::ray_trace(PosPoint source, PosPoint target)
{
    // run Bresenham's line algorithm
    if (std::abs(source.posx - target.posx) > std::abs(source.posy - target.posy))
    {
        return bresen_x(source, target);
    }
    else
    {
        return bresen_y(source, target);
    }

    return false;
}

void LOSfinder::mark_tiles_of_corner_as_visible(
        std::list<PosPoint>* retlist,
        PosPoint at,
        PosPoint center,
        char visibility[])
{
    for (int dx = -1; dx <= 0; dx++)
    {
        for (int dy = -1; dy <= 0; dy++)
        {
            PosPoint p(at.posx + dx, at.posy + dy, at.posz);
            if (!map_->CheckBorders(&p.posx, &p.posy, &p.posz))
            {
                continue;
            }

            int vis_x = (p.posx - center.posx + SIZE_W_SQ);
            int vis_y = (p.posy - center.posy + SIZE_H_SQ);
            int vis_idx = 2 * SIZE_H_SQ * vis_x + vis_y;

            if (vis_idx < 0)
            {
                continue;
            }

            if (visibility[vis_idx] == 1)
            {
                continue;
            }

            retlist->push_back(p);
            visibility[vis_idx] = 1;
        }
    }
}

LOSfinder::LOSfinder(IMapMaster* map)
{
    map_ = map;
}

// LOSfinder::calculateVisisble calculates visibility list of map from given map point
// The line-of-sigh check works as follows:
// from the center of given tile to every tile corner of the viewport ray is casted
// ray is casted from center of tile to corners, so it is never aligned to either axis
// if ray passes only transparent tiles on its way to given corner then the corner is visible
// if ray passes through corner then it checks sibling tiles. If both of them are not transparent then ray blocks
// if ray passes through edge it checks both adjasent tiles. They both must be transparent, otherwise ray blocks
// if tile has at least one visible corner then this tile is visible
// otherwise tile is invisible
std::list<PosPoint>* LOSfinder::CalculateVisisble(std::list<PosPoint>* retlist, int posx, int posy, int posz)
{
    Clear();

    const int VISIBLE_TILES_SIZE = 4 * (SIZE_H_SQ + 2) * (SIZE_W_SQ + 2);
    char* visible_tiles = new char[VISIBLE_TILES_SIZE];
    for (int i = 0; i < VISIBLE_TILES_SIZE; ++i)
    {
        visible_tiles[i] = 0;
    }

    PosPoint source(
          pos2corner(posx) + RAY_MULTIPLIER / 2,
          pos2corner(posy) + RAY_MULTIPLIER / 2,
          pos2corner(posz) + 1);
    for (int i = -SIZE_W_SQ; i < SIZE_W_SQ; ++i)
    {
        for (int j = -SIZE_H_SQ; j < SIZE_H_SQ; ++j)
        {
            PosPoint p(pos2corner(posx + i), pos2corner(posy + j), pos2corner(posz));
            if (!check_corner(p))
            {
                continue;
            }

            // TODO: we can check that all siblings of this corner are visible
            // so check is unnessesary

            if (ray_trace(source, p))
            {
                // add all tiles with this corner to visible list
                mark_tiles_of_corner_as_visible(
                    retlist, corner_point2point(p), corner_point2point(source), visible_tiles);
            }
        }
    }

    delete[] visible_tiles;
    return retlist;
}

void LOSfinder::Clear()
{
    worklist.clear();
}
