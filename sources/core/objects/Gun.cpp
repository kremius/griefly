#include "Gun.h"
#include "Tile.h"

Gun::Gun(quint32 id) : Item(id)
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

void Gun::ShootImpl(VDir target, const QString& sound,
                    const QString& projectile_type, const QString& casing_type)
{
    IdPtr<CubeTile> tile = GetOwner()->GetOwner();
    IdPtr<IMovable> shooter = GetOwner();
    if (tile.IsValid())
    {	
        if (UseAmmo())
        {
            int x = target.x;
            int y = target.y;
            Dir shooting_direction;
            Dir facing = shooter->GetDir();
            if(qAbs(y) != qAbs(x))
            {
                shooting_direction = VDirToDir(target);
            }
            else
            {
                if (y < 0)
                {
                    if (facing == D_LEFT || facing == D_RIGHT)
                    {
                        if (x > 0)
                        {
                            shooting_direction = D_RIGHT;
                        }
                        else
                        {
                            shooting_direction = D_LEFT;
                        }
                    }
                    else
                    {
                        shooting_direction = D_UP;
                    }
                }
                else
                {
                    if (facing == D_LEFT || facing == D_RIGHT)
                    {
                        if (x > 0)
                        {
                            shooting_direction = D_RIGHT;
                        }
                        else
                        {
                            shooting_direction = D_LEFT;
                        }
                    }
                    else
                    {
                        shooting_direction = D_DOWN;
                    }
                }
            }
            shooter->Rotate(shooting_direction);
            auto projectile = Create<Projectile>(projectile_type, tile);
            projectile->MakeMovementPattern(target, facing);
            PlaySoundIfVisible(sound, tile.Id());
            projectile->Process();
            if (!casing_type.isEmpty())
            {
                Dir dir = GetRand() % 4;
                IdPtr<Item> casing = Create<Item>(casing_type, tile.Id());
                casing->Rotate(dir);
            } 	
        }
        else
        {
            PlaySoundIfVisible("empty.ogg", tile.Id()); 
            // the *click* text from ss13
        }
    }	
}

void Gun::Shoot(VDir target)
{
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
void Gun::AttackBy(IdPtr<Item> item)
{
    if (IdPtr<AmmunitionBox> box = item)
    {
        if (box->CheckBullets())
        {
            if (AddAmmo())
            {
                box->RemoveBullet();
                return;
            }
        }
        // message which tells the player that the box is empty 
    }
}

bool Gun::Targetable(IdPtr<IOnMapBase> item)
{
    if (!item.IsValid())
    {
        return false;
    }

    if (!item->GetOwner())
    {
        return false;
    }

    IdPtr<CubeTile> cube_tile = item->GetOwner();
    if (!cube_tile.IsValid())
    {
        return false;
    }

    return true;
}

VDir Gun::TargetTileLoc(IdPtr<IOnMapBase> item) const
{
    IdPtr<CubeTile> cube_tile = item->GetOwner();
    VDir f;
    f.x = (cube_tile->GetX() - GetOwner()->GetX());
    f.y = (cube_tile->GetY() - GetOwner()->GetY());
    f.z = 0;
    return f;
}
