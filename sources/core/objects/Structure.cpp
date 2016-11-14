#include "Structure.h"

#include "Item.h"
#include "representation/Sound.h"

void Structure::AttackBy(IdPtr<Item> item)
{
    if (IdPtr<Screwdriver> scr = item)
    {
        anchored = !anchored;
        PlaySoundIfVisible("Screwdriver.ogg", owner.Id());
    }
}
