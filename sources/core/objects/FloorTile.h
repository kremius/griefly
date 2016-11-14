#pragma once

#include "Item.h"

class FloorTile: public Item
{
public:
    DECLARE_SAVED(FloorTile, Item);
    DECLARE_GET_TYPE_ITEM(FloorTile);
    FloorTile(quint32 id);
};
ADD_TO_TYPELIST(FloorTile);
