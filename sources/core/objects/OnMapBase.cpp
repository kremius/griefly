#include "OnMapBase.h"
#include "Turf.h"
#include "Movable.h"
#include "Item.h"
#include "../ObjectFactory.h"

using namespace kv;

IdPtr<ITurf> MapObject::GetTurf()
{
    auto owner = GetOwner();
    if (owner.IsValid())
    {
        return owner->GetTurf();
    }
    return 0;
}

IdPtr<MapObject> MapObject::GetRoot()
{
    if (!GetOwner().IsValid())
    {
        return GetId();
    }
    return GetOwner()->GetRoot();
}

void MapObject::PlaySoundIfVisible(const QString& name)
{
    if (GetGame().GetMap().IsTileVisible(GetRoot().Id()))
    {
        GetGame().AddSound(name);
    }
}

void MapObject::SetTurf(IdPtr<ITurf> turf)
{
    auto owner = GetOwner();
    if (owner.IsValid())
    {
        owner->SetTurf(turf);
    }
}

void MapObject::Bump(IdPtr<IMovable> item)
{
   // nothing
}

void MapObject::BumpByGas(Dir dir, bool inside)
{
   // nothing
}

void MapObject::AttackBy(IdPtr<Item> item)
{
    // nothing
}
