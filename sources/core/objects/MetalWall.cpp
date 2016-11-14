#include "MetalWall.h"

#include "Weldingtool.h"
#include "Girder.h"
#include "Floor.h"
#include "Materials.h"

#include "representation/Sound.h"

#include "../ObjectFactory.h"
#include "../Game.h"

MetalWall::MetalWall(quint32 id) : ITurf(id)
{
    transparent = false;
    SetPassable(D_ALL, Passable::EMPTY);

    v_level = 2;

    SetSprite("icons/walls.dmi");
    SetState("metal0");

    name = "Metal wall";
}

void MetalWall::AttackBy(IdPtr<Item> item)
{
    if (IdPtr<Weldingtool> wtool = item)
    {
        if (wtool->Working())
        {
            PlaySoundIfVisible("Welder.ogg", owner.Id());
            Create<IOnMapObject>(Girder::T_ITEM_S(), GetOwner());
            Create<IOnMapObject>(Metal::T_ITEM_S(), GetOwner());
            Create<ITurf>(Plating::T_ITEM_S(), GetOwner());
            Delete();
        }
    }
}

ReinforcedWall::ReinforcedWall(quint32 id) : ITurf(id)
{
    transparent = false;
    SetPassable(D_ALL, Passable::EMPTY);

    v_level = 2;

    SetSprite("icons/walls.dmi");
    SetState("r_wall");

    name = "Reinforced wall";
}
