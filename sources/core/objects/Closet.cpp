#include "Closet.h"

#include "Human.h"
#include "representation/Sound.h"

Closet::Closet(quint32 id)
    : IMovable(id)
{
    v_level = 4;

    name = "Closet";

    open_ = false;

    SetPassable(D_ALL, Passable::AIR);
    SetPassable(D_UP, Passable::FULL);
    SetPassable(D_DOWN, Passable::FULL);
    SetPassable(D_LEFT, Passable::FULL);
    SetPassable(D_RIGHT, Passable::FULL);

    SetSprite("icons/closet.dmi");
    SetState("closed");
}

bool Closet::Contains(IdPtr<IOnMapBase> item) const
{
    for (auto it = content_.begin(); it != content_.end(); ++it)
    {
        if (it->Id() == item.Id())
        {
            return true;
        }
    }
    return false;
}

bool Closet::CanTouch(IdPtr<IOnMapBase> item) const
{
    if (Contains(item))
    {
        return true;
    }
    return false;
}

void Closet::AttackBy(IdPtr<Item> item)
{
    if (item)
    {
        return;
    }

    if (open_)
    {
        Close();
    }
    else
    {
        Open();
    }
}

void Closet::Bump(IdPtr<IMovable> item)
{
    if (Contains(item))
    {
        if (!open_)
        {
            Open();
        }
        return;
    }
    IMovable::Bump(item);
}

IdPtr<IOnMapBase> Closet::GetNeighbour(Dir direct) const
{
    return GetId();
}

bool Closet::AddItem(IdPtr<IOnMapBase> item)
{
    if (IdPtr<Item> i = item)
    {
        content_.push_back(item);
        item->SetOwner(GetId());
        return true;
    }
    if (IdPtr<Human> h = item)
    {
        content_.push_back(item);
        item->SetOwner(GetId());
        return true;
    }
    return false;
}

bool Closet::RemoveItem(IdPtr<IOnMapBase> item)
{
    for (auto it = content_.begin(); it != content_.end(); ++it)
    {
        if (*it == item)
        {
            content_.erase(it);
            return true;
        }
    }
    return false;
}

void Closet::AfterWorldCreation()
{
    IMovable::AfterWorldCreation();

    if (!owner)
    {
        return;
    }

    owner->ForEach([this](IdPtr<IOnMapBase> object)
    {
       if (AddItem(object))
       {
           owner->RemoveItem(object);
       }
    });
}

void Closet::Delete()
{
    std::vector<IdPtr<IMovable>> copy = content_;
    for (auto it = copy.begin(); it != copy.end(); ++it)
    {
        if (!it->IsValid())
        {
            qDebug() << "Closet contains invalid id_ptr_on";
            KvAbort();
        }
        (*it)->Delete();
    }
    IMovable::Delete();
}

void Closet::Close()
{
    owner->ForEach([this](IdPtr<IOnMapBase> item)
    {
        if (AddItem(item))
        {
            owner->RemoveItem(item);
        }
    });

    open_ = false;
    SetPassable(D_ALL, Passable::AIR);
    SetState("closed");

    PlaySoundIfVisible("click.ogg", owner.Id());
}

void Closet::Open()
{
    for (auto it = content_.begin(); it != content_.end(); ++it)
    {
        owner->AddItem(*it);
    }
    content_.clear();

    open_ = true;
    SetPassable(D_ALL, Passable::FULL);
    SetState("open");

    PlaySoundIfVisible("click.ogg", owner.Id());
}


SecurityLocker::SecurityLocker(quint32 id) : Closet(id)
{
    locked_ = true;
    SetState("secure1");

    name = "Security locker";
}

void SecurityLocker::AttackBy(IdPtr<Item> item)
{
    if (item.IsValid())
    {
        if (!open_)
        {
            if (locked_)
            {
                Unlock();
            }
            else
            {
                Lock();
            }
            return;
        }
    }
    Closet::AttackBy(item);
}

void SecurityLocker::Lock()
{
    if (open_)
    {
        return;
    }
    locked_ = true;
    SetState("secure1");
}

void SecurityLocker::Unlock()
{
    if (open_)
    {
        return;
    }
    locked_ = false;
    SetState("secure");
}

void SecurityLocker::Open()
{
    if (locked_)
    {
        return;
    }
    Closet::Open();
    SetState("secureopen");
}

void SecurityLocker::Close()
{
    Closet::Close();
    SetState("secure");
}
