#pragma once

#include "Movable.h"

#include "../atmos/AtmosHolder.h"
#include "Item.h"

class GasTank: public IMovable
{
public:
    DECLARE_SAVED(GasTank, IMovable);
    DECLARE_GET_TYPE_ITEM(GasTank);
    GasTank(quint32 id);
    virtual void AfterWorldCreation() override;

    virtual void AttackBy(IdPtr<Item> item) override;
    virtual void Process() override;

    AtmosHolder* GetAtmosHolder() { return &atmos_holder_; }
private:
    void Open();
    void Close();

    bool KV_SAVEBLE(open_);
    AtmosHolder KV_SAVEBLE(atmos_holder_);
};
ADD_TO_TYPELIST(GasTank);

class MagicGasTank: public GasTank
{
public:
    DECLARE_SAVED(MagicGasTank, GasTank);
    DECLARE_GET_TYPE_ITEM(MagicGasTank);
    MagicGasTank(quint32 id);
    virtual void AfterWorldCreation() override;
    virtual void Process() override;
};
ADD_TO_TYPELIST(MagicGasTank);
