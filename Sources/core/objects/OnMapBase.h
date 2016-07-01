#pragma once

#include <functional>

#include "DrawInt.h"
//#include "../ObjectFactory.h"

class ITurf;
class IMovable;
class Item;
class Projectile;


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
    IOnMapBase(size_t id) : IDraw(id) {owner = 0; is_strong_owner = false;}
    virtual bool IsVisibleByPlayer() const
    {
        return true;
    }
    virtual void ForEach(std::function<void(id_ptr_on<IOnMapBase>)> callback)
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
        if (owner.valid())
        {
            owner->RemoveItem(GetId());
        }
        IDraw::Delete();
    }

    virtual void AttackBy(id_ptr_on<Item> item);

    virtual void ApplyForce(VDir force) {}

    virtual bool CanTouch(id_ptr_on<IOnMapBase> item) const
    {
        if (owner.valid())
        {
            return owner->CanTouch(item);
        }
        return false;
    }
    // Add some item
    // True - added
    // False - failed
    // This must not remove item from old place
    virtual bool AddItem(id_ptr_on<IOnMapBase> item)
    {
        if (owner.valid())
        {
            return owner->AddItem(item);
        }
        return false;
    }
    // Remove some item
    // True - removed
    // False failed
    virtual bool RemoveItem(id_ptr_on<IOnMapBase> item)
    {
        return false;
    }
    // If id equal with object id, dir fail or something else (operation unsupported)
    virtual id_ptr_on<IOnMapBase> GetNeighbour(Dir direct) const
    {
        if (owner.valid())
        {
            return owner->GetNeighbour(direct);
        }
        return GetId();
    }

    virtual bool Contains(id_ptr_on<IOnMapBase> item) const
    {
        // if (owner.valid())
        //    return owner->Contains(item);
        return false;
    }

    virtual void Bump(id_ptr_on<IMovable> item);
    virtual void BumpByGas(Dir dir, bool inside = false);

    template<class T>
    id_ptr_on<T> GetItem()
    {
        return GetItemImpl(T::RT_ITEM_S());
    }
    void SetOwner(size_t value)
    {
        owner = value;
    }
    id_ptr_on<IOnMapBase> GetOwner() const
    {
        return owner;
    }
    bool IsStrongOwner() const
    {
        return is_strong_owner;
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
    virtual id_ptr_on<ITurf> GetTurf();
    virtual void SetTurf(id_ptr_on<ITurf> turf);
protected:
    bool KV_SAVEBLE(is_strong_owner);
    id_ptr_on<IOnMapBase> KV_SAVEBLE(owner);
    virtual size_t GetItemImpl(unsigned int hash) { return 0; }
};
ADD_TO_TYPELIST(IOnMapBase);
