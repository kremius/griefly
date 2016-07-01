#pragma once

#include "Movable.h"
#include "Human.h"

std::ostream& operator<<(std::stringstream& file, const std::vector<Dir> & content);
std::istream& operator>>(std::stringstream& file, std::vector<Dir> & content);
unsigned int hash(const std::vector<Dir>& content);

class Projectile : public IMovable
{
public:
    Projectile(size_t id);
    DECLARE_SAVED(Projectile,IMovable)	
    DECLARE_GET_TYPE_ITEM(Projectile)
    void MakeMovementPattern(VDir target,id_ptr_on<Human> x);
    virtual void Process() override;
    virtual void Bump(id_ptr_on<IMovable> item) override;
    virtual void BumpByGas(Dir dir, bool inside = false) override {};
    virtual bool CheckPassable() override;
    virtual void AfterWorldCreation() override;
    virtual void ProcessForce() override {};
    void MakeMovementLoops(int x,int y,Dir d1,Dir d2);
    int GetDamage();
private:
    bool KV_SAVEBLE(reached_target);
    int KV_SAVEBLE(set_target_);
    id_ptr_on<Human> KV_SAVEBLE(shooter_);
    int KV_SAVEBLE(current_step_);
    std::vector<Dir> KV_SAVEBLE(movement_);
protected:
    int KV_SAVEBLE(damage);
};
ADD_TO_TYPELIST(Projectile);
