#include "Movable.h"

#include "../Helpers.h"
#include "Mob.h"

#include "../Game.h"

#include <cassert>

IMovable::IMovable(quint32 id) : IOnMapObject(id)
{
    lastMove = 0;
    tickSpeed = 1;
    pixSpeed = 1;
    dMove = D_DOWN;
    anchored = false;
    force_.x = 0;
    force_.y = 0;
    force_.z = 0;
}

bool IMovable::TryMove(Dir direct)
{
    if (!CheckMoveTime())
    {
        return false;
    }
    if (anchored)
    {
        return false;
    }
    if (!Rotate(direct))
    {
        return false;
    }
    if (!CheckPassable())
    {
        return false;
    }
    return MainMove();
}

void IMovable::ProcessForce()
{
    Dir step = VDirToDir(force_);

    TryMove(step);

    if (!IsNonZero(force_))
    {
        return;
    }

    VDir vstep = DirToVDir[step];
    force_.x -= (vstep.x * Friction::CombinedFriction(GetTurf())) / Friction::BASE_FRICTION;
    force_.y -= (vstep.y * Friction::CombinedFriction(GetTurf())) / Friction::BASE_FRICTION;
    force_.z -= (vstep.z * Friction::CombinedFriction(GetTurf())) / Friction::BASE_FRICTION;
}

void IMovable::ApplyForce(VDir force)
{
    if (!IsNonZero(force))
        return;
    if (!IsNonZero(force_))
        ForceManager::Get().Add(GetId());

    force_.x += force.x;
    force_.y += force.y;
    force_.z += force.z;
}

void IMovable::LoadInForceManager()
{
    if (IsNonZero(force_))
    {
        ForceManager::Get().Add(GetId());
    }
}

bool IMovable::CheckMoveTime()
{
    if ((static_cast<int>(MAIN_TICK) - lastMove) < tickSpeed)
    {
        return false;
    }
    return true;
}

bool IMovable::CheckPassable()
{
    PassableLevel loc = GetPassable(dMove);
    if (loc != Passable::FULL)
    {
        SetPassable(dMove, Passable::FULL);
    }
    if (!CanPass(owner->GetPassable(dMove), passable_level))
    {
        owner->Bump(GetId());
        force_.x = 0;
        force_.y = 0;
        force_.z = 0;
        if (loc != Passable::FULL)
        {
            SetPassable(dMove, loc);
        }
        return false;
    }
    if (loc != Passable::FULL)
    {
        SetPassable(dMove, loc);
    }

    auto neighbour = owner->GetNeighbour(dMove);
    if (   !CanPass(neighbour->GetPassable(D_ALL), passable_level)
        || !CanPass(neighbour->GetPassable(helpers::revert_dir(dMove)), passable_level))
    {
        neighbour->Bump(GetId());
        force_.x = 0;
        force_.y = 0;
        force_.z = 0;
        return false;
    }
    
    return true;
}

bool IMovable::Rotate(Dir dir)
{
    dMove = dir;
    return true;
}

bool IMovable::MainMove()
{
    auto new_owner = owner->GetNeighbour(dMove);
    if (new_owner == owner)
    {
        return false;
    }

    owner->RemoveItem(GetId());
    new_owner->AddItem(GetId());

    lastMove = static_cast<int>(MAIN_TICK);
    return true;
}

void IMovable::Represent()
{
    Representation::Entity ent;
    ent.id = GetId();
    ent.pos_x = GetX();
    ent.pos_y = GetY();
    ent.vlevel = v_level;
    ent.view = *GetView();
    ent.dir = GetDir();
    GetRepresentation().AddToNewFrame(ent);
}

void IMovable::Bump(IdPtr<IMovable> item)
{
    if (IdPtr<IMob> m = item)
    {
        ApplyForce(DirToVDir[m->dMove]);
    }
}

void IMovable::BumpByGas(Dir dir, bool inside)
{
    ApplyForce(DirToVDir[dir]);
}

ForceManager& ForceManager::Get()
{
    static ForceManager* fm = new ForceManager;
    return *fm;
}

void ForceManager::Add(IdPtr<IMovable> movable)
{
    to_add_.push_back(movable);
}

unsigned int ForceManager::Hash()
{
    unsigned int hash = 0;
    int i = 1;
    for (auto movable = under_force_.begin(); movable != under_force_.end(); ++movable)
    {
        hash += movable->Id() * i;
        ++i;
    }
    return hash;
}

void ForceManager::Process()
{
    for (auto movable = to_add_.begin(); movable != to_add_.end(); ++movable)
    {
        if (!(*movable).IsValid())
        {
            continue;
        }
        if (!IsNonZero((*movable)->force_))
        {
            continue;
        }

        auto to_add = std::find(under_force_.begin(), under_force_.end(), *movable);
        if (to_add == under_force_.end())
        {
            under_force_.push_back(*movable);
        }
    }
    if (to_add_.size())
    {
        std::sort(under_force_.begin(), under_force_.end(),
        [](IdPtr<IMovable> item1, IdPtr<IMovable> item2)
        {
            return item1.Id() < item2.Id();
        });
        to_add_.clear();
    }

    for (auto movable = under_force_.begin(); movable != under_force_.end(); ++movable)
    {
        if (   !(*movable)
            || !IsNonZero((*movable)->force_))
        {
            continue;
        }
        (*movable)->ProcessForce();
    }
}

void ForceManager::Clear()
{
    std::vector<IdPtr<IMovable>> to_remove;
    for (auto movable = under_force_.begin(); movable != under_force_.end(); ++movable)
    {
        if (   !(*movable)
            || !IsNonZero((*movable)->force_))
        {
            to_remove.push_back(*movable);
        }
    }
    for (auto it = to_remove.begin(); it != to_remove.end(); ++it)
    {
        under_force_.erase(std::find(under_force_.begin(), under_force_.end(), *it));
    }
}
