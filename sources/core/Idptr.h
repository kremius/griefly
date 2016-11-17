#pragma once

#include <cstdlib>

#include <iostream>

#include <QDebug>

#include "FastIsType.h"
#include "KVAbort.h"

class IMainObject;

struct ObjectInfo
{
    ObjectInfo()
        : object(nullptr)
    {
        // Nothing
    }
    IMainObject* object;
};

extern std::vector<ObjectInfo>* id_ptr_id_table;

struct IdPtrBase
{
protected:
    mutable IMainObject* casted_;
    quint32 id_;
};

template<class T>
class IdPtr : public IdPtrBase
{
    template<class U>
    friend std::ostream& operator<<(std::ostream& stream, const IdPtr<U>& ptr);
    template<class U>
    friend std::istream& operator>>(std::istream& stream, IdPtr<U>& ptr);
    template<class U>
    friend class IdPtr;
public:
    IdPtr()
    {
        id_ = 0;
        casted_ = nullptr;
    }
    IdPtr(quint32 id)
    {
        *this = id;
    }
    IdPtr(const IdPtr& other)
    {
        *this = other;
    }

    template<class U>
    IdPtr(const IdPtr<U>& other)
    {
        *this = other;
    }
    template<class U>
    bool operator==(const IdPtr<U>& other)
    {
        return id_ == other.id_;
    }
    template<class U>
    bool operator!=(const IdPtr<U>& other)
    {
        return !operator==(other);
    }

    IdPtr& operator=(quint32 id)
    {
        id_ = id;
        casted_ = nullptr;
        return *this;
    }
    IdPtr& operator=(const IdPtr& other)
    {
        id_ = other.id_;
        casted_ = other.casted_;
        return *this;
    }

    template<class U>
    IdPtr& operator=(const IdPtr<U>& other)
    {
#if defined(KV_ID_PTR_CASTING_CACHE)
        if (other.id_ == 0)
        {
            id_ = 0;
            casted_ = nullptr;
            return *this;
        }
        if (other.casted_ != nullptr)
        {
            if (std::is_base_of<T, U>::value)
            {
                casted_ = other.casted_;
                id_ = other.id_;
                return *this;
            }
            T* retval = castTo<T>(other.casted_);
            if (retval)
            {
                casted_ = retval;
                id_ = other.id_;
                return *this;
            }
            id_ = 0;
            casted_ = nullptr;
            return *this;
        }
#endif // KV_ID_PTR_CASTING_CACHE
        *this = other.id_;
        return *this;
    }

    T* operator*() const
    {
        if (id_ == 0)
        {
            return nullptr;
        }
        if (casted_ == nullptr)
        {
            Update();
        }
        return static_cast<T*>(casted_);
    }

    T* operator->() const
    {
        return operator*();
    }

    bool IsValid() const
    {
        Update();
        return operator*() != nullptr;
    }
    operator void*() const
    {
#if defined(KV_ID_PTR_VALID_CACHE)
        if (casted_)
        {
            return static_cast<void*>(casted_);
        }
#endif // KV_ID_PTR_VALID_CACHE
        if (IsValid())
        {
            return static_cast<void*>(casted_);
        }
        return nullptr;
    }
    quint32 Id() const
    {
        return id_;
    }
private:
    void Update() const
    {
        if (id_ == 0)
        {
            casted_ = nullptr;
            return;
        }

        IMainObject* local = GetFromIdTable(id_);
        if (local == nullptr)
        {
            casted_ = nullptr;
            return;
        }
        if (casted_)
        {
            return;
        }
        T* retval = CastTo<T>(local);
        if (retval)
        {
            casted_ = retval;
        }
    }

    static IMainObject* GetFromIdTable(quint32 id)
    {
        if (id >= id_ptr_id_table->size())
        {
            qDebug() << "Id table lookup fail, id: "
                     << id << ", size: " << id_ptr_id_table->size();
            KvAbort();
        }
        return (*id_ptr_id_table)[id].object;
    }
    // Dynamic memory allocation is disabled
    static void* operator new(size_t);
};

template<typename T>
std::ostream& operator<<(std::ostream& stream, const IdPtr<T>& ptr)
{
    stream << ptr.id_ << " ";
    return stream;
}

template<typename T>
std::istream& operator>>(std::istream& stream, IdPtr<T>& ptr)
{
    stream >> ptr.id_;
    return stream;
}
