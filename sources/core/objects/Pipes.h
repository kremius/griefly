#pragma once

#include "Movable.h"

#include "../atmos/AtmosHolder.h"

#include "Item.h"
#include "GasTank.h"\

class PipeBase: public IMovable
{
public:
    DECLARE_SAVED(PipeBase, IMovable);
    DECLARE_GET_TYPE_ITEM(PipeBase);
    PipeBase(quint32 id);
    virtual void AttackBy(IdPtr<Item> item) override;
    virtual bool Connect(Dir dir, IdPtr<PipeBase> pipe) { return false; }
    virtual bool CanTransferGas(Dir dir) const { return false; }
    AtmosHolder* GetAtmosHolder() { return &atmos_holder_; }
protected:
    void ConnectHelper(IdPtr<PipeBase>& connection, Dir dir);
    void ProcessHelper(IdPtr<PipeBase>& connection, Dir dir);
    AtmosHolder KV_SAVEBLE(atmos_holder_);
};
ADD_TO_TYPELIST(PipeBase);

class Pipe: public PipeBase
{
public:
    DECLARE_SAVED(Pipe, PipeBase);
    DECLARE_GET_TYPE_ITEM(Pipe);
    Pipe(quint32 id);
    virtual bool Connect(Dir dir, IdPtr<PipeBase> pipe) override;
    virtual void AfterWorldCreation() override;
    virtual bool CanTransferGas(Dir dir) const override { return true; }
    virtual void Process() override;
protected:
    static void GetTailAndHead(Dir dir, Dir* head, Dir* tail);
    IdPtr<PipeBase> KV_SAVEBLE(head_);
    IdPtr<PipeBase> KV_SAVEBLE(tail_);
};
ADD_TO_TYPELIST(Pipe);

class Manifold: public PipeBase
{
public:
    DECLARE_SAVED(Manifold, PipeBase);
    DECLARE_GET_TYPE_ITEM(Manifold);
    Manifold(quint32 id);
    virtual bool Connect(Dir dir, IdPtr<PipeBase> pipe) override;
    virtual void AfterWorldCreation() override;
    virtual bool CanTransferGas(Dir dir) const override;
    virtual void Process() override;
private:
    static void GetConnectionsDirs(Dir dir, Dir* tail, Dir* left, Dir* right);

    IdPtr<PipeBase> KV_SAVEBLE(tail_);
    IdPtr<PipeBase> KV_SAVEBLE(left_);
    IdPtr<PipeBase> KV_SAVEBLE(right_);
};
ADD_TO_TYPELIST(Manifold);


class Valve: public Pipe
{
public:
    DECLARE_SAVED(Valve, Pipe);
    DECLARE_GET_TYPE_ITEM(Valve);
    Valve(quint32 id);
    virtual bool CanTransferGas(Dir dir) const override;
    virtual void Process() override;
    virtual void AttackBy(IdPtr<Item> item) override;
private:
    bool KV_SAVEBLE(closed_);
};
ADD_TO_TYPELIST(Valve);

class Vent: public PipeBase
{
public:
    DECLARE_SAVED(Vent, PipeBase);
    DECLARE_GET_TYPE_ITEM(Vent);
    Vent(quint32 id);
    virtual bool Connect(Dir dir, IdPtr<PipeBase> pipe) override;
    virtual void AfterWorldCreation() override;
    virtual bool CanTransferGas(Dir dir) const override { return true; }
    virtual void Process() override;

    void SetHidden(bool hidden);
private:
    IdPtr<PipeBase> KV_SAVEBLE(tail_);
};
ADD_TO_TYPELIST(Vent);

class Connector: public PipeBase
{
public:
    DECLARE_SAVED(Connector, PipeBase);
    DECLARE_GET_TYPE_ITEM(Connector);
    Connector(quint32 id);
    void ConnectToGasTank(IdPtr<GasTank> tank);
    void DisconnectFromGasTank();
    virtual bool Connect(Dir dir, IdPtr<PipeBase> pipe) override;
    virtual void AfterWorldCreation() override;
    virtual bool CanTransferGas(Dir dir) const override { return true; }
    virtual void Process() override;
private:
    IdPtr<GasTank> KV_SAVEBLE(tank_);
    IdPtr<PipeBase> KV_SAVEBLE(tail_);
};
ADD_TO_TYPELIST(Connector);

class PipePump: public Pipe
{
public:
    DECLARE_SAVED(PipePump, Pipe);
    DECLARE_GET_TYPE_ITEM(PipePump);
    PipePump(quint32 id);
    virtual bool CanTransferGas(Dir dir) const override;
    virtual void Process() override;
public:
    int KV_SAVEBLE(pump_pressure_);
};
ADD_TO_TYPELIST(PipePump);


