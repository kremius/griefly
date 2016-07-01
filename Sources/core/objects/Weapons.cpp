#include "Weapons.h"
#include "representation/Sound.h"
#include "Tile.h"


LaserGun::LaserGun(size_t id) : Gun(id)
{
    SetState("energykill100");

    name = "Laser Gun";
    
    ammunition_ = 20;
    max_ammunition_ = 20;
}

void LaserGun::Shoot(VDir target)
{
    id_ptr_on<CubeTile> tile = GetOwner()->GetOwner();
    if (tile.valid())
    {
        if(UseAmmo())
        {
            id_ptr_on<Projectile> p = Create<Projectile>(Laser::T_ITEM_S(),tile.ret_id());
            p->MakeMovementPattern(target,GetOwner());
            unsigned int value = GetRand() % 2;
            std::string snd;
            if (value == 0)
                snd = "laser3.ogg";
            if (value == 1)
                snd = "Laser.ogg";
            PlaySoundIfVisible(snd, tile.ret_id());
        }
		//a counter that tells the number of bullets in magazine/clip
        else
        {
          PlaySoundIfVisible("empty.ogg", tile.ret_id());
		//the *click* text from ss13
       }
    }	
}
void LaserGun::AttackBy(id_ptr_on<Item> item)
{
    if (id_ptr_on<AmmunitionBox> r = item)
    {
	if(r->CheckBullets())
        {
		if(AddAmmo())
                {
		    r->RemoveBullet();
	        }
	}
	else
	{
	// message which tells the player that the box is empty 
	}
    }
}


Revolver::Revolver(size_t id) : Gun(id)
{
    SetState("revolver");

    name = "Revolver";
    
    ammunition_ = 999;
    max_ammunition_ = 999;
}

void Revolver::Shoot(VDir target)
{
    id_ptr_on<CubeTile> tile = GetOwner()->GetOwner();
    if (tile.valid())
    {	
        if (UseAmmo())
        {
            id_ptr_on<Projectile> p = Create<Projectile>(Bullet::T_ITEM_S(),tile.ret_id());
            p->MakeMovementPattern(target,GetOwner());
            std::string snd;
            snd = "Gunshot.ogg";
            PlaySoundIfVisible(snd, tile.ret_id());
            id_ptr_on<Item> bc =Create<Item>(BulletCasing::T_ITEM_S(),tile.ret_id());
            Dir dir = GetRand() % 4;
            bc->Rotate(dir);
        }
		//a counter that tells the number of bullets in magazine/clip
        else
        {
            PlaySoundIfVisible("empty.ogg", tile.ret_id());
		//sound and the *click* text from ss13
        }
     }
}
void Revolver::AttackBy(id_ptr_on<Item> item)
{
    if (id_ptr_on<AmmunitionBox> r = item)
    {
	if(r->CheckBullets())
        {
		if(AddAmmo())
                {
		    r->RemoveBullet();
	        }
	}
	else
	{
	// message which tells the player that the box is empty 
	}
    }
}
