#pragma once

#include "Projectiles.h"

class Bullet: public Projectile
{
public:
    DECLARE_SAVED(Bullet, Projectile);
    DECLARE_GET_TYPE_ITEM(Bullet);
    Bullet(quint32 id);
};
ADD_TO_TYPELIST(Bullet);

class Laser: public Projectile
{
public:
    DECLARE_SAVED(Laser, Projectile);
    DECLARE_GET_TYPE_ITEM(Laser);
    Laser(quint32 id);

    virtual bool CheckPassable() override;
};
ADD_TO_TYPELIST(Laser);

