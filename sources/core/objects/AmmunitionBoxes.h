#pragma once

#include "Item.h"

class AmmunitionBox: public Item
{
public:
    DECLARE_SAVED(AmmunitionBox, Item);
    DECLARE_GET_TYPE_ITEM(AmmunitionBox);
    AmmunitionBox(quint32 id);
    void RemoveBullet();
    bool CheckBullets();
private:
    int KV_SAVEBLE(bullets_);
};
ADD_TO_TYPELIST(AmmunitionBox);

