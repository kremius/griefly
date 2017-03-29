#include "Weapons.h"
#include "Tile.h"

LaserGun::LaserGun(quint32 id) : Gun(id)
{
    SetState("energykill100");

    name = "Laser Gun";
    
    ammunition_ = 20;
    max_ammunition_ = 20;
}

void LaserGun::Shoot(const VDir &target)
{
    unsigned int value = GetRand() % 2;
    QString snd;
    if (value == 0)
    {
        snd = "laser3.wav";
    }
    if (value == 1)
    {
        snd = "Laser.wav";
    }
    ShootImpl(target, snd, Laser::GetTypeStatic(), "");
}
Revolver::Revolver(quint32 id) : Gun(id)
{
    SetState("revolver");

    name = "Revolver";
    
    ammunition_ = 6;
    max_ammunition_ = 6;
}

void Revolver::Shoot(const VDir &target) 
{
    QString snd;
    snd = "Gunshot.wav";
    ShootImpl(target, snd, Bullet::GetTypeStatic(), BulletCasing::GetTypeStatic());
}
