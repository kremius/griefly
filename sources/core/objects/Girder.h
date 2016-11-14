#pragma once

#include "Structure.h"

class Girder: public Structure
{
public:
    DECLARE_SAVED(Girder, Structure);
    DECLARE_GET_TYPE_ITEM(Girder);
    Girder(quint32 id);
    virtual void AttackBy(IdPtr<Item> item) override;
};
ADD_TO_TYPELIST(Girder);
