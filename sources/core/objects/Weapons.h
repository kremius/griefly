#include "Gun.h"

class LaserGun: public Gun
{
public:
    DECLARE_SAVED(LaserGun, Gun);
    DECLARE_GET_TYPE_ITEM(LaserGun);
    LaserGun(quint32 id);
    virtual void Shoot(VDir target) override;
};
ADD_TO_TYPELIST(LaserGun);

class Revolver: public Gun
{
public:
    DECLARE_SAVED(Revolver, Gun);
    DECLARE_GET_TYPE_ITEM(Revolver);
    Revolver(quint32 id);
    virtual void Shoot(VDir target) override;
};
ADD_TO_TYPELIST(Revolver);
