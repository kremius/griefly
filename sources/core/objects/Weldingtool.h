#pragma once

#include "Item.h"

class Weldingtool: public Item
{
public:
    DECLARE_SAVED(Weldingtool, Item);
    DECLARE_GET_TYPE_ITEM(Weldingtool);
    Weldingtool(quint32 id);
    virtual void Process() override;
    virtual void AttackBy(IdPtr<Item> item) override;
    void TurnOn();
    void TurnOff();
    bool Working() const { return on_; }
    int AddFuel(int amount);
private:
    int KV_SAVEBLE(fuel_);
    bool KV_SAVEBLE(on_);
};
ADD_TO_TYPELIST(Weldingtool);
