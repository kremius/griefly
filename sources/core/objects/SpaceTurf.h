#pragma once

#include "Turf.h"

class Space: public ITurf
{
public:
    DECLARE_SAVED(Space, ITurf);
    DECLARE_GET_TYPE_ITEM(Space);
    Space(quint32 id);
    virtual void AfterWorldCreation() override;
    virtual void AttackBy(IdPtr<Item> item) override;
};
ADD_TO_TYPELIST(Space);
