#include "Furniture.h"

Bed::Bed(quint32 id) : IMovable(id)
{
    anchored = true;
    v_level = 3;

    SetSprite("icons/objects.dmi");
    SetState("bed");

    name = "Bed";
}

Chair::Chair(quint32 id) : IMovable(id)
{
    anchored = true;

    v_level = 3;

    SetSprite("icons/objects.dmi");
    SetState("chair");

    name = "Chair";
}


Stool::Stool(quint32 id) : IMovable(id)
{
    anchored = true;

    v_level = 3;

    SetSprite("icons/objects.dmi");
    SetState("stool");

    name = "Stool";
}
