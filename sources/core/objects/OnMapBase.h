#pragma once

#include <functional>

#include "DrawInt.h"

class ITurf;
class IMovable;
class Item;

typedef int PassableLevel;
namespace Passable
{
    const PassableLevel EMPTY = 0;
    const PassableLevel AIR = 10;
    const PassableLevel SMALL_ITEM = 20;
    const PassableLevel BIG_ITEM = 30;
    const PassableLevel SMALL_CREATURE = 40;
    const PassableLevel BIG_CREATURE = 50;
    const PassableLevel FULL = 100;
}

inline bool CanPass(PassableLevel block, PassableLevel object)
{
    return block >= object;
}

class IOnMapBase: public IDraw
{
public:
    DECLARE_SAVED(IOnMapBase, IDraw);
    DECLARE_GET_TYPE_ITEM(IOnMapBase);
    IOnMapBase(quint32 id) : IDraw(id) {owner = 0;}
    virtual bool IsVisibleByPlayer() const
    {
        return true;
    }
    virtual void ForEach(std::function<void(IdPtr<IOnMapBase>)> callback)
    {
        return;
    }
    // 0 - passable
    // 10 - air passable
    //
    //
    virtual PassableLevel GetPassable(Dir direct) const
    {
        return Passable::EMPTY;
    }
    virtual bool IsTransparent() const
    {
        return true;
    }
    
    virtual void Delete() override
    {
        if (owner.IsValid())
        {
            owner->RemoveItem(GetId());
        }
        IDraw::Delete();
    }

    virtual void AttackBy(IdPtr<Item> item);

    virtual void ApplyForce(VDir force) {}

    virtual bool CanTouch(IdPtr<IOnMapBase> item) const
    {
        if (owner.IsValid())
        {
            return owner->CanTouch(item);
        }
        return false;
    }

    // Add some item
    // True - added
    // False - failed
    // This must not remove item from old place
    virtual bool AddItem(IdPtr<IOnMapBase> item)
    {
        if (owner.IsValid())
        {
            return owner->AddItem(item);
        }
        return false;
    }
    // Remove some item
    // True - removed
    // False failed
    virtual bool RemoveItem(IdPtr<IOnMapBase> item)
    {
        return false;
    }
    // If id equal with object id, dir fail or something else (operation unsupported)
    virtual IdPtr<IOnMapBase> GetNeighbour(Dir direct) const
    {
        if (owner.IsValid())
        {
            return owner->GetNeighbour(direct);
        }
        return GetId();
    }

    virtual bool Contains(IdPtr<IOnMapBase> item) const
    {
        // if (owner.IsValid())
        //    return owner->Contains(item);
        return false;
    }

    virtual void Bump(IdPtr<IMovable> item);
    virtual void BumpByGas(Dir dir, bool inside = false);

    template<class T>
    IdPtr<T> GetItem()
    {
        return GetItemImpl(T::RT_ITEM_S());
    }
    void SetOwner(quint32 value)
    {
        owner = value;
    }
    IdPtr<IOnMapBase> GetOwner() const
    {
        return owner;
    }

    virtual int GetX() const
    {
        return owner->GetX();
    }
    virtual int GetY() const
    {
        return owner->GetY();
    }
    virtual int GetZ() const
    {
        return owner->GetZ();
    }
    virtual void UpdatePassable() {}
    virtual IdPtr<ITurf> GetTurf();
    virtual void SetTurf(IdPtr<ITurf> turf);
    IdPtr<IOnMapBase> GetRoot();
protected:
    IdPtr<IOnMapBase> KV_SAVEBLE(owner);
    virtual quint32 GetItemImpl(unsigned int hash) { return 0; }
};
ADD_TO_TYPELIST(IOnMapBase);
