#pragma once

#include "Movable.h"

namespace kv
{

class Structure : public IMovable
{
public:
    DECLARE_SAVED(Structure, IMovable);
    DECLARE_GET_TYPE_ITEM(Structure);
    Structure() { v_level = 3; };
    virtual void AttackBy(IdPtr<Item> item) override;
};
ADD_TO_TYPELIST(Structure);

}
