#include "Projectiles.h"
#include "../Helpers.h"

inline std::ostream& operator<<(std::stringstream& file,const std::vector<Dir>& s)
{
    file << " " << s.size() << " ";
    for (auto it : s)
        file << " " << it << " ";
    return file;
}

inline std::istream& operator>>(std::stringstream& file,std::vector<Dir>& s)
{
    size_t size;
    file >> size;
    s.reserve(size);

    unsigned int local_id;
    for (size_t i = 0; i < size; ++i)
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


Projectile::Projectile(size_t id) : IMovable(id)
{
    SetPassable(D_ALL, Passable::AIR);
    damage = 0;
    SetSprite("icons/projectiles.dmi");
    v_level = 5;
    current_step_ = 0;
    set_target_ = 0;
    tickSpeed = 1;
    pixSpeed = 4;
    reached_target = 0;
}
void Projectile::Process()
{
   if (set_target_ == 1)
   {
       std::cout << "here" << std::endl;
       Dir step =  movement_[current_step_];
       Rotate(step);
       if(!CheckPassable())
       {
           this->Delete();
           return;
       }
       else if (reached_target)
       {
           MainMove();
           return;
       }
       else if (current_step_ < movement_.size()-1 && !reached_target)
       {
           current_step_++;
       }
       else if(!reached_target && current_step_ == movement_.size()-1)
       {
           current_step_ =0;
           Dir tmp = movement_[1];
           movement_.clear();
           movement_.push_back(tmp);
           reached_target = 1;
       }    
       MainMove();
   }
}

bool Projectile::CheckPassable()
{
    id_ptr_on<Projectile> p = this->GetId();
    PassableLevel loc = GetPassable(GetDir());
    if (loc != Passable::FULL)
    {
        SetPassable(GetDir(), Passable::FULL);
    }
    if (!CanPass(owner->GetPassable(GetDir()), passable_level))
    {
        owner->Bump(p);
        if (loc != Passable::FULL)
        {
            SetPassable(GetDir(), loc);
        }
        return false;
    }
    if (loc != Passable::FULL)
    {
        SetPassable(GetDir(), loc);
    }

    auto neighbour = owner->GetNeighbour(GetDir());
    if (   !CanPass(neighbour->GetPassable(D_ALL), passable_level)
        || !CanPass(neighbour->GetPassable(helpers::revert_dir(GetDir())), passable_level))
    {
        neighbour->Bump(p);
        return false;
    }
    
    return true;
}
void Projectile::Bump(id_ptr_on<IMovable> item)
{
}
//TODO maybe it can apply froce depending on bullet speed but that should be considered        ApplyForce(DirToVDir[m->dMove]);
void Projectile::MakeMovementPattern(VDir target,id_ptr_on<Human> someone)
{
    set_target_=1;
    int x = target.x;
    int y = target.y;
    int abs_x = std::abs(x);
    int abs_y = std::abs(y);
    shooter_ = someone;
    Dir facing = someone->GetDir();
    std::cout << y << " x: " <<  x << std::endl;
    if (abs_x == abs_y)
    {
        if (y<0)
        {
            if (facing == 0|| facing == 1)
            {
                if (x>0)
                {
                    MakeMovementLoops(abs_y,abs_x,D_RIGHT,D_UP);
                }
                else
                {
                    MakeMovementLoops(abs_y,abs_x,D_LEFT,D_UP);
                }
            }
            else
            {
                if (x>0)
                {
                    MakeMovementLoops(abs_y,abs_x,D_UP,D_RIGHT);
                }
                else
                {
                    MakeMovementLoops(abs_y,abs_x,D_UP,D_LEFT);
                }
            }
        }
        else
        {
            if (facing == 0|| facing == 1)
            {
                if ( x > 0)
                {
                    MakeMovementLoops(abs_y,abs_x,D_RIGHT,D_DOWN);
                }
                else
                {
                    MakeMovementLoops(abs_y,abs_x,D_LEFT,D_DOWN);
                }
            }
            else
            {
                if (x>0)
                {
                    MakeMovementLoops(abs_y,abs_x,D_DOWN,D_RIGHT);
                }
                else
                {
                    MakeMovementLoops(abs_y,abs_x,D_DOWN,D_LEFT);
                }
            }
        }
    }
    else if (x >= 0 && y >= 0)
    {
       if (y > x)
       {
            MakeMovementLoops(abs_y,abs_x,D_DOWN,D_RIGHT);
       }
       else if (x > y)
       {
            MakeMovementLoops(abs_x,abs_y,D_RIGHT,D_DOWN);
       }
    }
    else if(x <= 0 && y <= 0)
    {   
       if ( abs_y > abs_x  )
       {
           MakeMovementLoops(abs_y,abs_x,D_UP,D_LEFT);
       }
       else if ( abs_x > abs_y)
       {
           MakeMovementLoops(abs_x,abs_y,D_LEFT,D_UP);
       }
    }
    else if (x < 0 && y > 0)
    {
        if (y > abs_x)
        {
             MakeMovementLoops(abs_y,abs_x,D_DOWN,D_LEFT);
        }
        else if (abs_x > abs_y)
        {
            MakeMovementLoops(abs_x,abs_y,D_LEFT,D_DOWN);
        }
    }
    else if (x > 0 && y < 0)
    {
        if (abs_y > x)
        {
            MakeMovementLoops(abs_y,abs_x,D_UP,D_RIGHT);
        }
        else if (x > abs_y )
        {
            MakeMovementLoops(abs_x,abs_y,D_RIGHT,D_UP);
        }
        
    }
    else
    {
        qDebug() << "Critical error, MakeMovementPattern has reached unreachable place";
        abort();
    }
    shooter_->Rotate(movement_[1]);
}
void Projectile::MakeMovementLoops(int x,int y,Dir d1,Dir d2)
{
int shuffle = 0;
int z = (2*y)-1;
for (int i = 0; i < y+x;i++)
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
    return damage;
}

