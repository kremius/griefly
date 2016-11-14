#pragma once

#include "Item.h"
#include "Projectiles.h"
#include "AmmunitionBoxes.h"
#include "ProjectileTypes.h"
#include "../Game.h"

class Gun: public Item
{
public:
    DECLARE_SAVED(Gun, Item);
    DECLARE_GET_TYPE_ITEM(Gun);
    Gun(quint32 id);
    virtual void Shoot(VDir target);
    bool AddAmmo();
    bool UseAmmo();   
    virtual void AttackBy(IdPtr<Item> item);
    static bool Targetable(IdPtr<IOnMapBase> item);
    VDir TargetTileLoc(IdPtr<IOnMapBase> item) const;
protected:
    int KV_SAVEBLE(max_ammunition_);
    int KV_SAVEBLE(ammunition_);
    void ShootImpl(VDir target, const QString& sound,
                   const QString& projectile_type, const QString& casing_type);
};
ADD_TO_TYPELIST(Gun);
