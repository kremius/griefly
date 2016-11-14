#include "AmmunitionBoxes.h"

AmmunitionBox::AmmunitionBox(quint32 id) : Item(id)
{
    SetSprite("icons/ammo.dmi");
    SetState("357");

    name = "Revolver Ammunition",
	
    bullets_ = 50;
}

bool AmmunitionBox::CheckBullets()
{
    if (bullets_ > 0)
    {
        return true;
    }
    return false;
}

void AmmunitionBox::RemoveBullet()
{
    --bullets_;
}
