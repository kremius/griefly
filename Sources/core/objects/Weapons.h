#include "Gun.h"



class LaserGun: public Gun
{
public:
    DECLARE_SAVED(LaserGun, Gun);
    DECLARE_GET_TYPE_ITEM(LaserGun);
    LaserGun(size_t id);
    virtual void Shoot(VDir target) override;
    virtual void AttackBy(id_ptr_on<Item> item) override;
};
ADD_TO_TYPELIST(LaserGun);

class Revolver: public Gun
{
public:
    DECLARE_SAVED(Revolver, Gun);
    DECLARE_GET_TYPE_ITEM(Revolver);
    Revolver(size_t id);
    virtual void Shoot(VDir target) override;
    virtual void AttackBy(id_ptr_on<Item> item) override;
};
ADD_TO_TYPELIST(Revolver);
