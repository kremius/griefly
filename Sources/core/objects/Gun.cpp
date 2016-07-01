#include "Gun.h"
#include "Tile.h"

Gun::Gun(size_t id) : Item(id)
{
    SetSprite("icons/guns.dmi");
    ammunition_ = 1;
    max_ammunition_ = 1;
}

bool Gun::UseAmmo()
{
    if (ammunition_ <= 0)
    {
        ammunition_ = 0;
        return false;
    }
    --ammunition_;
    return true;
}

void Gun::Shoot(VDir target)
{
    id_ptr_on<CubeTile> tile = GetOwner()->GetOwner();
    if (tile.valid())
    {	
        if(UseAmmo())
        { 
            id_ptr_on<Projectile> p = Create<Projectile>(Bullet::T_ITEM_S(),GetOwner()->GetOwner());
            p->MakeMovementPattern(target,GetOwner());	
        }
        else
        {
            PlaySoundIfVisible("empty.ogg",owner.ret_id());
		// the *click* text from ss13
        }
    }	
}

bool Gun::AddAmmo()
{
    if (ammunition_ < max_ammunition_)
    {
        ++ammunition_;
        return true;
    }
    return false;
}
void Gun::AttackBy(id_ptr_on<Item> item)
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

bool Gun::Targetable(id_ptr_on<IOnMapBase> item)
{
    if (!item.valid())
    {
        return false;
    }

    if (!item->GetOwner())
    {
        return false;
    }

    id_ptr_on<CubeTile> cube_tile = item->GetOwner();
    if (!cube_tile.valid())
    {
        return false;
    }

    return true;
}

VDir Gun::TargetTileLoc(id_ptr_on<IOnMapBase> item) const
{
    id_ptr_on<CubeTile> cube_tile = item->GetOwner();
    VDir f;
    f.x =  (cube_tile->GetX() - GetOwner()->GetX());
    f.y =  (cube_tile->GetY() - GetOwner()->GetY());
    f.z = 0;
    return f;
}
