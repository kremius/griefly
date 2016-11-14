#include "Materials.h"

#include "Girder.h"
#include "../ObjectFactory.h"
#include "../Game.h"
#include "Floor.h"
#include "Grille.h"

Rod::Rod(quint32 id) : Item(id)
{
    SetState("rods");

    name = "Rods";
}

void Rod::AttackBy(IdPtr<Item> item)
{
    if (item.Id() == GetId()) // Attack self
    {
        Create<IOnMapObject>(Grille::T_ITEM_S(), GetTurf());
        Delete();
    }
}

Metal::Metal(quint32 id) : Item(id)
{
    SetState("sheet-metal");

    name = "Metal";
}

void Metal::AttackBy(IdPtr<Item> item)
{
    if (item.Id() == GetId()) // Attack self
    {
        if (IdPtr<Floor> floor = GetTurf())
        {
            Create<IOnMapObject>(Girder::T_ITEM_S(), GetTurf());
            Delete();
        }
    }
}
