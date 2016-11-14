#include "Lattice.h"

#include "FloorTile.h"
#include "Turf.h"
#include "Floor.h"

#include "../ObjectFactory.h"
#include "../Game.h"
#include "representation/Sound.h"

Lattice::Lattice(quint32 id) : Structure(id)
{
    SetSprite("icons/structures.dmi");
    SetState("latticefull");

    v_level = 1;
    anchored = true;

    name = "Lattice";
}

void Lattice::AttackBy(IdPtr<Item> item)
{
    if (IdPtr<FloorTile> tile = item)
    {
        tile->Delete();

        GetTurf()->Delete();
        Create<ITurf>(Plating::T_ITEM_S(), GetOwner());

        PlaySoundIfVisible("Deconstruct.ogg", owner.Id());

        Delete();
    }
}
