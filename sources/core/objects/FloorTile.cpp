#include "FloorTile.h"

FloorTile::FloorTile(quint32 id) : Item(id)
{
    SetState("tile");

    name = "Floor tile";
}
