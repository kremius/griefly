#pragma once

#include "OnMapObject.h"
#include "Movable.h"

class Door: public IOnMapObject
{
public:
    DECLARE_SAVED(Door, IOnMapObject);
    DECLARE_GET_TYPE_ITEM(Door);
    Door(quint32 id);
    
    enum STATE
    {
        OPEN, CLOSED, OPENING, CLOSING, WELDED  
    };

    virtual void Open();
    virtual void Close();
    virtual void Process() override;
    virtual void Bump(IdPtr<IMovable> item) override;
    virtual void AttackBy(IdPtr<Item> item) override;
    bool IsState(STATE state) const { return state == door_state_ ; }
    void Weld();
private:
    int KV_SAVEBLE(door_state_);
    quint32 KV_SAVEBLE(last_tick_);
};
ADD_TO_TYPELIST(Door);

class SecurityDoor: public Door
{
public:
    DECLARE_SAVED(SecurityDoor, Door);
    DECLARE_GET_TYPE_ITEM(SecurityDoor);
    SecurityDoor(quint32 id);
};
ADD_TO_TYPELIST(SecurityDoor);

class NontransparentDoor: public Door
{
public:
    DECLARE_SAVED(NontransparentDoor, Door);
    DECLARE_GET_TYPE_ITEM(NontransparentDoor);
    NontransparentDoor(quint32 id);

    virtual void Open() override;
    virtual void Close() override;
};
ADD_TO_TYPELIST(NontransparentDoor);

class ExternalDoor: public NontransparentDoor
{
public:
    DECLARE_SAVED(ExternalDoor, NontransparentDoor);
    DECLARE_GET_TYPE_ITEM(ExternalDoor);
    ExternalDoor(quint32 id);
};
ADD_TO_TYPELIST(ExternalDoor);

class MaintenanceDoor: public NontransparentDoor
{
public:
    DECLARE_SAVED(MaintenanceDoor, NontransparentDoor);
    DECLARE_GET_TYPE_ITEM(MaintenanceDoor);
    MaintenanceDoor(quint32 id);
};
ADD_TO_TYPELIST(MaintenanceDoor);

class GlassDoor: public IMovable
{
public:
    DECLARE_SAVED(GlassDoor, IMovable);
    DECLARE_GET_TYPE_ITEM(GlassDoor);
    GlassDoor(quint32 id);

    virtual void AfterWorldCreation() override;

    enum STATE
    {
        OPEN, CLOSED, OPENING, CLOSING  
    };

    virtual void Open();
    virtual void Close();
    virtual void Process() override;
    virtual void Bump(IdPtr<IMovable> item) override;
    virtual void AttackBy(IdPtr<Item> item) override;
    bool IsState(STATE state) const { return state == door_state_; };
private:
    int KV_SAVEBLE(door_state_);
    quint32 KV_SAVEBLE(last_tick_);

    QString KV_SAVEBLE(door_prefix_);
};
ADD_TO_TYPELIST(GlassDoor);
