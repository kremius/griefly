#pragma once

#include "Movable.h"
#include "net/Network2.h"

class IMessageReceiver : public IMovable
{
public:
    DECLARE_SAVED(IMessageReceiver, IMovable);
    DECLARE_GET_TYPE_ITEM(IMessageReceiver);
    virtual void ProcessMessage(const Message2& msg){}
    IMessageReceiver(quint32 id) : IMovable(id) {}
};

ADD_TO_TYPELIST(IMessageReceiver);
