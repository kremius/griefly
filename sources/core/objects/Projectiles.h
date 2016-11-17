#pragma once

#include "Movable.h"
#include "Human.h"

std::ostream& operator<<(std::stringstream& file, const std::vector<Dir> & content);
std::istream& operator>>(std::stringstream& file, std::vector<Dir> & content);
unsigned int hash(const std::vector<Dir>& content);

class Projectile : public IMovable
{
public:
    Projectile(quint32 id);
    DECLARE_SAVED(Projectile, IMovable)	
    DECLARE_GET_TYPE_ITEM(Projectile)
    void MakeMovementPattern(VDir target, Dir facing);
    virtual void Process() override;
    virtual void BumpByGas(Dir dir, bool inside = false) override {}
    virtual void AfterWorldCreation() override;
    virtual void ProcessForce() override {}
    void MakeMovementLoops(int d1_number, int d2_number, Dir d1, Dir d2);
    int GetDamage();
    virtual bool CheckPassable() override;
protected:
    bool CheckHumanTile();
    bool KV_SAVEBLE(harmless_);

    int KV_SAVEBLE(damage_);
private:
    bool ProcessMovement();

    int KV_SAVEBLE(current_step_);
    std::vector<Dir> KV_SAVEBLE(movement_);
};
ADD_TO_TYPELIST(Projectile);
