#pragma once

#include "KvAbort.h"

#include "../FastIsType.h"
#include "../Constheader.h"
#include "../Idptr.h"
#include "../Hashes.h"
#include "../KvMacros.h"

#include "Interfaces.h"

#ifdef _BUILD_TESTS
#include <gtest/gtest.h>
#endif // _BUILD_TESTS

void InitSettersForTypes();

namespace kv
{

class MapObject;
class Hearer;

namespace internal
{
    GameInterface*& GetObjectGame(Object* object);
    quint32& GetObjectId(Object* object);
    quint32 CreateImpl(Object* object, const QString& type, quint32 owner = 0);
}

class Object
{
    friend GameInterface*& internal::GetObjectGame(Object* object);
    friend quint32& internal::GetObjectId(Object* object);
    friend quint32 internal::CreateImpl(Object* object, const QString& type, quint32 owner);
public:
    Object() { id_ = 0; how_often_ = 0; game_ = nullptr; }
    Object(kv::internal::NoInitialization) { id_ = 0; how_often_ = 0; game_ = nullptr; }

    virtual ~Object() { }

    void PlayMusic(const QString& name, int volume = 100);

    virtual void Save(FastSerializer& serializer);
    virtual void Load(FastDeserializer& deserializer);

    virtual unsigned int Hash()
    {
        unsigned int retval = 0;
        retval += hash(GetId());
        retval += hash(GetFreq());
        return retval;
    }

    virtual void Delete();
    virtual void AfterWorldCreation() { }
    virtual const QString& GetType() const
    {
        return GetTypeStatic();
    }
    static const QString& GetTypeStatic();
    static int TYPE_INDEX;
    virtual int GetTypeIndex() const
    {
        return TYPE_INDEX;
    }
    static int GetTypeIndexStatic()
    {
        return TYPE_INDEX;
    }

    static Object* _Z_KV_Creator() { return new Object(); }
    static Object* _Z_KV_VoidCreator() { return new Object(kv::internal::no_initialization);}

    virtual void Process() { }

    quint32 GetId() const { return id_; }

    void SetFreq(int freq);
    int GetFreq() const { return how_often_; }

    GameInterface& GetGame();
    const GameInterface& GetGame() const;
    ObjectFactoryInterface& GetFactory();
    unsigned int GetRand();

    virtual Hearer* ToHearer() { return nullptr; }
    void PostHtmlFor(const QString& html, IdPtr<Object> mob);
    void PostVisible(const QString& html, const Position& place);

    template<typename T>
    IdPtr<T> Create(const QString& type, IdPtr<MapObject> owner = 0)
    {
        IdPtr<T> retval = CreateImpl(type, owner.Id());
        if (!retval.IsValid())
        {
            KvAbort(QString("Unable to cast object: %1").arg(type));
        }
        return retval;
    }
private:
    quint32 CreateImpl(const QString& type, quint32 owner = 0);

    GameInterface* game_;
    quint32 id_;
    int how_often_;
};

namespace internal
{
    inline GameInterface*& GetObjectGame(Object* object)
    {
        return object->game_;
    }
    inline quint32& GetObjectId(Object* object)
    {
        return object->id_;
    }
    inline quint32 CreateImpl(Object* object, const QString& type, quint32 owner)
    {
        return object->CreateImpl(type, owner);
    }
}
}
