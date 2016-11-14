#include "SpaceTurf.h"

#include "../SyncRandom.h"

#include "Item.h"
#include "Materials.h"
#include "Lattice.h"
#include "../ObjectFactory.h"
#include "../Game.h"

Space::Space(quint32 id) : ITurf(id)
{
    SetAtmosState(SPACE);

    SetPassable(D_ALL, Passable::FULL);
    transparent = true;
    
    SetFriction(0);

    SetSprite("icons/space.png"); 
    SetState("10");
    name = "Space";
}

void Space::AfterWorldCreation()
{
    quint32 value = GetRand() % 25 + 1;
    SetState(QString::number(value));
}

void Space::AttackBy(IdPtr<Item> item)
{
    if (IdPtr<Rod> rod = item)
    {
        rod->Delete();
        Create<IOnMapObject>(Lattice::T_ITEM_S(), GetOwner());
    }
}
