#include "OnMapBase.h"
#include "Turf.h"
#include "Movable.h"
#include "Item.h"
#include "../ObjectFactory.h"
#include "Projectiles.h"

id_ptr_on<ITurf> IOnMapBase::GetTurf()
{
    if (owner.valid())
    {
        return owner->GetTurf();
    }
    return 0;
}

void IOnMapBase::SetTurf(id_ptr_on<ITurf> turf)
{
    if (owner.valid())
    {
        owner->SetTurf(turf);
    }
}

void IOnMapBase::Bump(id_ptr_on<IMovable> item)
{
   // nothing
}

void IOnMapBase::BumpByGas(Dir dir, bool inside)
{
   // nothing
}

void IOnMapBase::AttackBy(id_ptr_on<Item> item)
{
    // nothing
}
