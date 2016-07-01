#include <assert.h>

#include "../Helpers.h"
#include "../Map.h"
#include "../Game.h"
#include "OnMapObject.h"
#include "Tile.h"
#include "Movable.h"

CubeTile::CubeTile(size_t id) : IOnMapBase(id)
{
    turf_ = 0;

    posx_ = -1;
    posy_ = -1;
    posz_ = -1;

    sum_passable_all_ = Passable::FULL;
    sum_passable_up_ = Passable::FULL;
    sum_passable_down_ = Passable::FULL;
    sum_passable_left_ = Passable::FULL;
    sum_passable_right_ = Passable::FULL;
}

bool CubeTile::CanTouch(id_ptr_on<IOnMapBase> item) const
{
    if (!item.valid())
    {
        return false;
    }

    if (!item->GetOwner())
    {
        return false;
    }

    id_ptr_on<CubeTile> cube_tile = item->GetOwner();
    if (!cube_tile.valid())
    {
        return false;
    }

    int cube_tile_posx = cube_tile->posx();
    int cube_tile_posy = cube_tile->posy();

    if (std::abs(posx() - cube_tile_posx) > 1)
    {
        return false;
    }
    if (std::abs(posy() - cube_tile_posy) > 1)
    {
        return false;
    }

    if (   (posx() == cube_tile_posx)
        && (posy() == cube_tile_posy))
    {
        return true;
    }

    if (posx() == cube_tile_posx)
    {
        if (posy() > cube_tile_posy)
        {
            return CanTouch(item, D_UP);
        }
        else
        {
            return CanTouch(item, D_DOWN);
        }
    }
    if (posy() == cube_tile_posy)
    {
        if (posx() > cube_tile_posx)
        {
            return CanTouch(item, D_LEFT);
        }
        else
        {
            return CanTouch(item, D_RIGHT);
        }
    }

    // Up Left
    if (   (posy() > cube_tile_posy)
        && (posx() > cube_tile_posx))
    {
        return    CanTouch(item, D_LEFT, D_UP)
               || CanTouch(item, D_UP, D_LEFT);
    }
    // Down Right
    if (   (posy() < cube_tile_posy)
        && (posx() < cube_tile_posx))
    {
        return    CanTouch(item, D_RIGHT, D_DOWN)
               || CanTouch(item, D_DOWN, D_RIGHT);
    }

    // Up Right
    if (   (posy() > cube_tile_posy)
        && (posx() < cube_tile_posx))
    {
        return    CanTouch(item, D_RIGHT, D_UP)
               || CanTouch(item, D_UP, D_RIGHT);
    }

    // Down Left
    if (   (posy() < cube_tile_posy)
        && (posx() > cube_tile_posx))
    {
        return    CanTouch(item, D_LEFT, D_DOWN)
               || CanTouch(item, D_DOWN, D_LEFT);
    }

    // It should not be reached
    return false;
}


bool CubeTile::CanTouch(id_ptr_on<IOnMapBase> item, Dir dir) const
{
    if (!CanPass(GetPassable(dir), Passable::BIG_ITEM))
    {
        return false;
    }

    // TODO: implementation is not perfect, but fine for a while
    if (!CanPass(item->GetPassable(helpers::revert_dir(dir)), Passable::BIG_ITEM))
    {
        return true;
    }
    if (CanPass(item->GetOwner()->GetPassable(helpers::revert_dir(dir)), Passable::BIG_ITEM))
    {
        return true;
    }
    return false;
}

bool CubeTile::CanTouch(id_ptr_on<IOnMapBase> item, Dir first_dir, Dir second_dir) const
{
    if (!CanPass(GetPassable(first_dir), Passable::BIG_ITEM))
    {
        return false;
    }

    auto tile = GetNeighbour(first_dir);

    if (   !CanPass(tile->GetPassable(helpers::revert_dir(first_dir)), Passable::BIG_ITEM)
        || !CanPass(tile->GetPassable(helpers::revert_dir(D_ALL)), Passable::BIG_ITEM))
    {
        return false;
    }

    if (!CanPass(tile->GetPassable(second_dir), Passable::BIG_ITEM))
    {
        return false;
    }

    if (!CanPass(item->GetPassable(helpers::revert_dir(second_dir)), Passable::BIG_ITEM))
    {
        return true;
    }

    if (CanPass(item->GetOwner()->GetPassable(helpers::revert_dir(second_dir)), Passable::BIG_ITEM))
    {
        return true;
    }

    return false;
}

void CubeTile::MoveToDir(Dir dir, int *x, int *y, int *z) const
{
    if (x)
    {
        *x += DirToVDir[dir].x;
        if (*x >= GetGame().GetMap().GetWidth() ||
            *x <= -1)
        {
            *x -= DirToVDir[dir].x;
        }
    }
    if (y)
    {
        *y += DirToVDir[dir].y;
        if (*y >= GetGame().GetMap().GetHeight() ||
            *y <= -1)
        {
            *y -= DirToVDir[dir].y;
        }
    }
    if (z)
    {
        *z += DirToVDir[dir].z;
        if (*z >= GetGame().GetMap().GetDepth() ||
            *z <= -1)
        {
            *z -= DirToVDir[dir].z;
        }
    }
}

bool CubeTile::Contains(id_ptr_on<IOnMapBase> item) const
{
    for (auto it = inside_list_.begin(); it != inside_list_.end(); ++it)
    {
        if (it->ret_id() == item.ret_id())
        {
            return true;
        }
    }
    return false;
}

void CubeTile::Bump(id_ptr_on<IMovable> item)
{
    if (GetTurf())
    {
        GetTurf()->Bump(item);
    }

    if (item->GetOwner().ret_id() == GetId())
    {
        for (auto it = inside_list_.begin(); it != inside_list_.end(); ++it)
            if (!CanPass((*it)->GetPassable(item->GetDir()), item->passable_level))
            {
                (*it)->Bump(item);
                return;
            }
        return;
    }

    for (auto it = inside_list_.begin(); it != inside_list_.end(); ++it)
        if (!CanPass((*it)->GetPassable(helpers::revert_dir(item->GetDir())), item->passable_level))
        {
            (*it)->Bump(item);
            return;
        }
    for (auto it = inside_list_.begin(); it != inside_list_.end(); ++it)
        if (!CanPass((*it)->GetPassable(D_ALL), item->passable_level))
        {
            (*it)->Bump(item);
            return;
        }
}

void CubeTile::BumpByGas(Dir dir, bool inside)
{
    if (GetTurf())
        GetTurf()->BumpByGas(dir);

    if (inside)
    {
        for (auto it = inside_list_.begin(); it != inside_list_.end(); ++it)
            if (!CanPass((*it)->GetPassable(dir), Passable::AIR))
            {
                (*it)->BumpByGas(dir);
                return;
            }
        return;
    }

    for (auto it = inside_list_.begin(); it != inside_list_.end(); ++it)
        if (!CanPass((*it)->GetPassable(helpers::revert_dir(dir)), Passable::AIR))
        {
            (*it)->BumpByGas(dir);
            return;
        }
    for (auto it = inside_list_.begin(); it != inside_list_.end(); ++it)
        if (!CanPass((*it)->GetPassable(D_ALL), Passable::AIR))
        {
            (*it)->BumpByGas(dir);
            return;
        }
}

bool CubeTile::AddItem(id_ptr_on<IOnMapBase> item_raw)
{
    id_ptr_on<IOnMapObject> item = item_raw;
    if (!item.valid())
    {
        return false;
    }

    inside_list_.push_back(item);
    item->SetOwner(GetId());

    sum_passable_all_ = std::min(sum_passable_all_, item->GetPassable(D_ALL));
    sum_passable_up_ = std::min(sum_passable_up_, item->GetPassable(D_UP));
    sum_passable_down_ = std::min(sum_passable_down_, item->GetPassable(D_DOWN));
    sum_passable_left_ = std::min(sum_passable_left_, item->GetPassable(D_LEFT));
    sum_passable_right_ = std::min(sum_passable_right_, item->GetPassable(D_RIGHT));

    return true;
}
bool CubeTile::RemoveItem(id_ptr_on<IOnMapBase> item_raw)
{
    id_ptr_on<IOnMapObject> item = item_raw;
    if (!item.valid())
    {
        return false;
    }

    if (item == GetTurf())
    {
        SetTurf(0);
        return true;
    }

    auto itr = inside_list_.begin();
    while(itr != inside_list_.end())
    {
        if (itr->ret_id() == item->GetId())
        {
            inside_list_.erase(itr);
            UpdatePassable();
            return true;
        }
        ++itr;
    }
    return false;
}



id_ptr_on<IOnMapBase> CubeTile::GetNeighbour(Dir direct) const
{
    return GetNeighbourImpl(direct);
}

id_ptr_on<CubeTile> CubeTile::GetNeighbourImpl(Dir direct) const
{
    int new_x = posx_;
    int new_y = posy_;
    int new_z = posz_;
    MoveToDir(direct, &new_x, &new_y, &new_z);
    return GetGame().GetMap().GetSquares()[new_x][new_y][new_z];
}

PassableLevel CubeTile::GetPassable(Dir direct) const
{
    switch (direct)
    {
    case D_UP:    return sum_passable_up_;
    case D_DOWN:  return sum_passable_down_;
    case D_LEFT:  return sum_passable_left_;
    case D_RIGHT: return sum_passable_right_;
    case D_ALL:   return sum_passable_all_;
    }
    return Passable::FULL;
}

void CubeTile::UpdatePassable()
{
    sum_passable_all_ = Passable::FULL;
    sum_passable_up_ = Passable::FULL;
    sum_passable_down_ = Passable::FULL;
    sum_passable_left_ = Passable::FULL;
    sum_passable_right_ = Passable::FULL;

    if (turf_.valid())
    {
        sum_passable_all_ = std::min(sum_passable_all_, turf_->GetPassable(D_ALL));
        sum_passable_up_ = std::min(sum_passable_up_, turf_->GetPassable(D_UP));
        sum_passable_down_ = std::min(sum_passable_down_, turf_->GetPassable(D_DOWN));
        sum_passable_left_ = std::min(sum_passable_left_, turf_->GetPassable(D_LEFT));
        sum_passable_right_ = std::min(sum_passable_right_, turf_->GetPassable(D_RIGHT));
    }
    for (auto it = inside_list_.begin(); it != inside_list_.end(); ++it)
    {
        sum_passable_all_ = std::min(sum_passable_all_, (*it)->GetPassable(D_ALL));
        sum_passable_up_ = std::min(sum_passable_up_, (*it)->GetPassable(D_UP));
        sum_passable_down_ = std::min(sum_passable_down_, (*it)->GetPassable(D_DOWN));
        sum_passable_left_ = std::min(sum_passable_left_, (*it)->GetPassable(D_LEFT));
        sum_passable_right_ = std::min(sum_passable_right_, (*it)->GetPassable(D_RIGHT));
    }
}

bool CubeTile::IsTransparent() const
{
    if (turf_.valid() && !turf_->IsTransparent())
    {
        return false;
    }
    for (auto it = inside_list_.begin(); it != inside_list_.end(); ++it)
    {
        if (!(*it)->IsTransparent())
        {
            return false;
        }
    }
    return true;
}

size_t CubeTile::GetItemImpl(unsigned int hash)
{
    for (auto it = inside_list_.rbegin(); it != inside_list_.rend(); ++it)
    {
        if (FastIsType(hash, (*it)->RT_ITEM()))
        {
            return it->ret_id();
        }
    }
    return 0;
}

void CubeTile::ForEach(std::function<void(id_ptr_on<IOnMapBase>)> callback)
{
    InsideType copy_vector = inside_list_;

    for (auto it = copy_vector.begin(); it != copy_vector.end(); ++it)
    {
        callback(*it);
    }
}

void CubeTile::LoadInMap()
{
    GetGame().GetMap().GetSquares()[posx_][posy_][posz_] = GetId();
}

