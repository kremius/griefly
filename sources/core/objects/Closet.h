#pragma once

#include "Movable.h"

#include "Tile.h"
#include "Item.h"

class Closet: public IMovable
{
public:
    DECLARE_SAVED(Closet, IMovable);
    DECLARE_GET_TYPE_ITEM(Closet);
    Closet(quint32 id);

    virtual void AttackBy(IdPtr<Item> item) override;
    virtual void Bump(IdPtr<IMovable> item) override;

    virtual bool Contains(IdPtr<IOnMapBase> item) const override;
    virtual bool CanTouch(IdPtr<IOnMapBase> item) const override;

    virtual bool AddItem(IdPtr<IOnMapBase> item) override;
    virtual bool RemoveItem(IdPtr<IOnMapBase> item) override;

    virtual void AfterWorldCreation() override;

    virtual void Delete() override;
    virtual IdPtr<IOnMapBase> GetNeighbour(Dir direct) const override;
    // TODO: ForEach (?)
protected:
    virtual void Open();
    virtual void Close();

    bool KV_SAVEBLE(open_);

    std::vector<IdPtr<IMovable>> KV_SAVEBLE(content_);
};
ADD_TO_TYPELIST(Closet);

class SecurityLocker: public Closet
{
public:
    DECLARE_SAVED(SecurityLocker, Closet);
    DECLARE_GET_TYPE_ITEM(SecurityLocker);
    SecurityLocker(quint32 id);
    virtual void AttackBy(IdPtr<Item> item) override;
protected:
    virtual void Lock();
    virtual void Unlock();

    virtual void Open() override;
    virtual void Close() override;
    bool KV_SAVEBLE(locked_);
};
ADD_TO_TYPELIST(SecurityLocker);
