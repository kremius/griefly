#include "Projectiles.h"
#include "../Helpers.h"

std::ostream& operator<<(std::stringstream& file, const std::vector<Dir>& s)
{
    file << " " << s.size() << " ";
    for (auto it : s)
        file << " " << it << " ";
    return file;
}

std::istream& operator>>(std::stringstream& file, std::vector<Dir>& s)
{
    quint32 size;
    file >> size;
    s.reserve(size);

    unsigned int local_id;
    for (quint32 i = 0; i < size; ++i)
    {
        file >> local_id;
        s.push_back(local_id);
    }
    return file;
}

unsigned int hash(const std::vector<Dir>& content)
{
    unsigned int retval = 0;
    int i = 1;
    for (auto it = content.begin(); it != content.end(); ++it, ++i)
    {
        retval += *it * i;
    }
    return retval;
}


Projectile::Projectile(quint32 id) : IMovable(id)
{
    SetPassable(D_ALL, Passable::FULL);
    damage_ = 0;
    SetSprite("icons/projectiles.dmi");
    v_level = 6;
    current_step_ = 0;
    tickSpeed = 1;
    pixSpeed = 4;
    passable_level = Passable::SMALL_ITEM;
    harmless_ = true;
}
void Projectile::Process()
{
    Dir old_dir = GetDir();
    if (!ProcessMovement())
    {
        return;
    }
    harmless_ = false;
    if (GetDir() != old_dir)
    {
        ProcessMovement();
    }
}

bool Projectile::CheckPassable()
{   
    if (!harmless_)
    {
        if (CheckHumanTile())
        {
            return false;
        }
    }
    if (IMovable::CheckPassable())
    {
        return true;
    }
    Delete();
    return false;
}

bool Projectile::CheckHumanTile()
{
    auto tile = GetOwner();
    if (IdPtr<Human> human = tile->GetItem<Human>())
    {
        human->Bump(GetId());
        Delete();
        return true;
    }
    return false;
}

bool Projectile::ProcessMovement()
{
    if (movement_.size() == 0)
    {
        qDebug() << "Trajectory for the projectile is empty, critical error!";
        KvAbort();
    }
    Dir step = movement_[current_step_];
    Rotate(step);
    if(!CheckPassable())
    {
        return false;
    }
    current_step_ = (current_step_ + 1) % movement_.size();
    MainMove();
    return true;
}

void Projectile::MakeMovementPattern(VDir target, Dir facing)
{
    int x = target.x;
    int y = target.y;
    int abs_x = std::abs(x);
    int abs_y = std::abs(y);
    if (abs_x == abs_y)
    {
        if (y < 0)
        {
            if (facing == D_LEFT || facing == D_RIGHT)
            {
                if (x > 0)
                {
                    MakeMovementLoops(abs_y, abs_x, D_RIGHT, D_UP);
                }
                else
                {
                    MakeMovementLoops(abs_y, abs_x, D_LEFT, D_UP);
                }
            }
            else
            {
                if (x > 0)
                {
                    MakeMovementLoops(abs_y, abs_x, D_UP, D_RIGHT);
                }
                else
                {
                    MakeMovementLoops(abs_y, abs_x, D_UP, D_LEFT);
                }
            }
        }
        else
        {
            if (facing == D_LEFT || facing == D_RIGHT)
            {
                if (x > 0)
                {
                    MakeMovementLoops(abs_y, abs_x, D_RIGHT, D_DOWN);
                }
                else
                {
                    MakeMovementLoops(abs_y, abs_x, D_LEFT, D_DOWN);
                }
            }
            else
            {
                if (x > 0)
                {
                    MakeMovementLoops(abs_y, abs_x, D_DOWN, D_RIGHT);
                }
                else
                {
                    MakeMovementLoops(abs_y, abs_x, D_DOWN, D_LEFT);
                }
            }
        }
    }
    else if (x >= 0 && y >= 0)
    {
       if (y > x)
       {
            MakeMovementLoops(abs_y, abs_x, D_DOWN, D_RIGHT);
       }
       else if (x > y)
       {
            MakeMovementLoops(abs_x, abs_y, D_RIGHT, D_DOWN);
       }
    }
    else if(x <= 0 && y <= 0)
    {   
       if (abs_y > abs_x  )
       {
           MakeMovementLoops(abs_y, abs_x, D_UP, D_LEFT);
       }
       else if (abs_x > abs_y)
       {
           MakeMovementLoops(abs_x, abs_y, D_LEFT, D_UP);
       }
    }
    else if (x < 0 && y > 0)
    {
        if (y > abs_x)
        {
             MakeMovementLoops(abs_y, abs_x, D_DOWN, D_LEFT);
        }
        else if (abs_x > abs_y)
        {
            MakeMovementLoops(abs_x, abs_y, D_LEFT, D_DOWN);
        }
    }
    else if (x > 0 && y < 0)
    {
        if (abs_y > x)
        {
            MakeMovementLoops(abs_y, abs_x, D_UP, D_RIGHT);
        }
        else if (x > abs_y )
        {
            MakeMovementLoops(abs_x, abs_y, D_RIGHT, D_UP);
        }
        
    }
    else
    {
        qDebug() << "Critical error, MakeMovementPattern has reached unreachable place";
        KvAbort();
    }
}
void Projectile::MakeMovementLoops(int d1_number, int d2_number, Dir d1, Dir d2)
{
    int shuffle = 1;
    int z = (2 * d2_number) - 1;
    for (int i = 0; i < d2_number + d1_number; i++)
    {
        if (shuffle == 1 || z < i)
        {
            movement_.push_back(d1);
            shuffle = 0;
        }
        else if (shuffle == 0)
        {
            movement_.push_back(d2);
            shuffle = 1;
        }
    }
}
void Projectile::AfterWorldCreation()
{
    IMovable::AfterWorldCreation();
    SetFreq(1);
}
int Projectile::GetDamage()
{
    return damage_;
}

