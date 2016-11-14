#pragma once

#include "Movable.h"

class Tank: public IMovable
{
public:
    DECLARE_SAVED(Tank, IMovable);
    DECLARE_GET_TYPE_ITEM(Tank);
    Tank(quint32 id) : IMovable(id) { }
};
ADD_TO_TYPELIST(Tank);

class FuelTank: public Tank
{
public:
    DECLARE_SAVED(FuelTank, Tank);
    DECLARE_GET_TYPE_ITEM(FuelTank);
    FuelTank(quint32 id);
    virtual void AttackBy(IdPtr<Item> item) override;
};
ADD_TO_TYPELIST(FuelTank);

class WaterTank: public Tank
{
public:
    DECLARE_SAVED(WaterTank, Tank);
    DECLARE_GET_TYPE_ITEM(WaterTank);
    WaterTank(quint32 id);
    virtual void AttackBy(IdPtr<Item> item) override;
};
ADD_TO_TYPELIST(WaterTank);
