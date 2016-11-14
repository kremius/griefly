#pragma once

#include "Breakable.h"

class Grille: public Breakable
{
public:
    DECLARE_SAVED(Grille, Breakable);
    DECLARE_GET_TYPE_ITEM(Grille);
    Grille(quint32 id);
    virtual void AttackBy(IdPtr<Item> item) override;
    virtual void Break() override;
    virtual void PlayOnHitSound() override;
private:
    bool KV_SAVEBLE(cutted_);
};
ADD_TO_TYPELIST(Grille);
