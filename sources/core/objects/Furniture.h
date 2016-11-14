#pragma once

#include "Movable.h"

class Bed: public IMovable
{
public:
    DECLARE_SAVED(Bed, IMovable);
    DECLARE_GET_TYPE_ITEM(Bed);
    Bed(quint32 id);
};
ADD_TO_TYPELIST(Bed);

class Chair: public IMovable
{
public:
    DECLARE_SAVED(Chair, IMovable);
    DECLARE_GET_TYPE_ITEM(Chair);
    Chair(quint32 id);
};
ADD_TO_TYPELIST(Chair);

class Stool: public IMovable
{
public:
    DECLARE_SAVED(Stool, IMovable);
    DECLARE_GET_TYPE_ITEM(Stool);
    Stool(quint32 id);
};
ADD_TO_TYPELIST(Stool);
