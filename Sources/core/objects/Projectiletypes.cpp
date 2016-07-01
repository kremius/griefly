#include "Projectiletypes.h"

Bullet::Bullet(size_t id) : Projectile(id)
{
    damage = 20;
    SetState("bullet");
    name = "Bullet";
}

Laser::Laser(size_t id) : Projectile(id)
{
    damage = 10;
    SetState("laser");
    name = "Laser";
}
