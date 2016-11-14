#include "Glass.h"

#include "../SyncRandom.h"
#include "../Helpers.h"
#include "Mob.h"
#include "Breakable.h"
#include "Shard.h"

FlatGlass::FlatGlass(quint32 id) : Breakable(id)
{
    transparent = true;

    tickSpeed = 5;
    pixSpeed = 1;
    SetHitPoints(20);

    v_level = 9;

    SetSprite("icons/structures.dmi");
    SetState("window");
    
    name = "Window";
}

void FlatGlass::AfterWorldCreation()
{
    Structure::AfterWorldCreation();

    SetPassable(D_UP, Passable::FULL);
    SetPassable(D_DOWN, Passable::FULL);
    SetPassable(D_LEFT, Passable::FULL);
    SetPassable(D_RIGHT, Passable::FULL);

    SetPassable(GetDir(), Passable::EMPTY);
}

void FlatGlass::Bump(IdPtr<IMovable> item)
{
    if (IdPtr<IMob> mob = item)
    {
        if (item->GetDir() != GetDir())
        {
            if (!CanPass(owner->GetPassable(D_ALL), passable_level) || anchored)
            {
                return;
            }

            Rotate(item->GetDir());
        }
        IMovable::Bump(item);
        return;
    }
    Breakable::Bump(item);
}

bool FlatGlass::Rotate(Dir dir)
{
    SetPassable(GetDir(), Passable::FULL);
    Structure::Rotate(dir);
    SetPassable(GetDir(), Passable::EMPTY);
    return true;
}

void FlatGlass::Break()
{
    QString sound = QString("Glassbr%1.ogg").arg(GetRand() % 3 + 1);
    PlaySoundIfVisible(sound, GetOwner().Id());
    Create<Item>(Shard::T_ITEM_S(), GetOwner());
    Delete();
}

void FlatGlass::PlayOnHitSound()
{
    PlaySoundIfVisible("Glasshit.ogg", GetOwner().Id());
}

void FlatGlass::AttackBy(IdPtr<Item> item)
{
    if (!item.IsValid())
    {
/*      GetGame().GetChat().PostSimpleText(
        name + " knocks on window. " +, GetRoot().Id());*/
        PlaySoundIfVisible("Glasshit.ogg", GetOwner().Id());
        return;
    }
    Breakable::AttackBy(item);
}

ReinforcedFlatGlass::ReinforcedFlatGlass(quint32 id) : FlatGlass(id)
{
    SetState("rwindow");
    name = "Reinforced window";
    SetHitPoints(100);
}
