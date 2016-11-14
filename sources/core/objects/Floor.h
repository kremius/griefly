#pragma once

#include "Turf.h"

class Floor: public ITurf
{
public:
    DECLARE_SAVED(Floor, ITurf);
    DECLARE_GET_TYPE_ITEM(Floor);
    Floor(quint32 id);
    virtual void AfterWorldCreation() override;

    virtual void AttackBy(IdPtr<Item> item) override;
    void SetOpen(bool o);

    bool KV_SAVEBLE(bloody);
protected:
    bool KV_SAVEBLE(open_);
};
ADD_TO_TYPELIST(Floor);

class Plating: public Floor
{
public:
    DECLARE_SAVED(Plating, Floor);
    DECLARE_GET_TYPE_ITEM(Plating);
    Plating(quint32 id);
};
ADD_TO_TYPELIST(Plating);

