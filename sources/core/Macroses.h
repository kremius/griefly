#pragma once

#if defined(__linux__)
#define __forceinline __attribute__((always_inline)) inline
#endif

#include "StreamWrapper.h"

#define ADD_TO_TYPELIST(classname)                                              \

class NotLoadItem {} const nouse;

template<int>
struct FakeParamClass {};

#define DECLARE_SAVED(thisclass, masterclass)  \
    friend void InitSettersForTypes();\
         typedef thisclass THIS_CLASS_TYPE;      \
    static IMainObject* _Z_creator(quint32 id) { return new thisclass(id); };\
    static IMainObject* _Z_creatorSaved() { return new thisclass(nouse);}; \
        static const int THIS_COUNTER = __COUNTER__;           \
template<int num> __forceinline void KV_SAVE_FUNC(std::stringstream& file, FakeParamClass<num>){};     \
template<int num> __forceinline void KV_LOAD_FUNC(std::stringstream& file, FakeParamClass<num>){};     \
template<int num> __forceinline unsigned int KV_HASH_FUNC(unsigned int hash, FakeParamClass<num>){return hash;};     \
        virtual bool Save(std::stringstream& file) override                \
        {                                                           \
        static_assert(std::is_same<decltype(this), thisclass*>::value && std::is_base_of<masterclass, thisclass>::value, #thisclass " error DECLARE_SAVED");\
            masterclass ::Save(file);                           \
            KV_SAVE_FUNC(file, FakeParamClass<THIS_COUNTER + 1>());    \
            return true;                                            \
        };                                                          \
        virtual bool Load(std::stringstream& file) override                 \
        {                                                           \
            masterclass ::Load(file);                           \
            KV_LOAD_FUNC(file, FakeParamClass<THIS_COUNTER + 1>()); \
            return true;                                            \
        };                                                          \
        virtual unsigned int Hash() override                 \
        {                                                           \
            return                                                  \
            KV_HASH_FUNC(masterclass ::Hash(), FakeParamClass<THIS_COUNTER + 1>());\
        }; \
        thisclass (NotLoadItem) : masterclass(nouse) {}

#define KV_SAVEBLE(name) name; struct Z_Impl##name { static const int name##counter = __COUNTER__;}; __forceinline void KV_SAVE_FUNC(std::stringstream& file, FakeParamClass<Z_Impl##name :: name##counter>){file << " "; WrapWriteMessage(file, name); file << " "; KV_SAVE_FUNC(file, FakeParamClass<Z_Impl##name :: name##counter + 1>());} __forceinline void KV_LOAD_FUNC(std::stringstream& file, FakeParamClass<Z_Impl##name :: name##counter>){WrapReadMessage(file, name); KV_LOAD_FUNC(file, FakeParamClass<Z_Impl##name :: name##counter + 1>());} __forceinline unsigned int KV_HASH_FUNC(unsigned int h, FakeParamClass<Z_Impl##name :: name##counter>){return hash(name) * (Z_Impl##name :: name##counter + 1) + KV_HASH_FUNC(h, FakeParamClass<Z_Impl##name :: name##counter + 1>());};__forceinline static void _Z_KV_SETTERS##name (IMainObject* ptr, std::stringstream& str){ THIS_CLASS_TYPE *ptr_loc = reinterpret_cast<THIS_CLASS_TYPE *>(ptr); WrapReadMessage(str, ptr_loc-> name);}

#define KV_ON_LOAD(name, value) name; struct Z_Impl##name { static const int name##counter = __COUNTER__;}; __forceinline void KV_SAVE_FUNC(std::stringstream& file, FakeParamClass<Z_Impl##name :: name##counter>){KV_SAVE_FUNC(file, FakeParamClass<Z_Impl##name ::name##counter + 1>());} __forceinline void KV_LOAD_FUNC(std::stringstream& file, FakeParamClass<Z_Impl##name ::name##counter>){name = value; KV_LOAD_FUNC(file, FakeParamClass<Z_Impl##name :: name##counter + 1>());} __forceinline unsigned int KV_HASH_FUNC(unsigned int h, FakeParamClass<Z_Impl##name :: name##counter>){return KV_HASH_FUNC(h, FakeParamClass<Z_Impl##name :: name##counter + 1>());};

#define KV_ON_LOAD_CALL(function) struct Z_Impl##function { static const int function##counter = __COUNTER__;}; __forceinline void KV_SAVE_FUNC(std::stringstream& file, FakeParamClass<Z_Impl##function :: function##counter>){KV_SAVE_FUNC(file, FakeParamClass<Z_Impl##function :: function##counter + 1>());} __forceinline void KV_LOAD_FUNC(std::stringstream& file, FakeParamClass<Z_Impl##function ::function##counter>){ function(); KV_LOAD_FUNC(file, FakeParamClass<Z_Impl##function :: function##counter + 1>());} __forceinline unsigned int KV_HASH_FUNC(unsigned int h, FakeParamClass<Z_Impl##function :: function##counter>){return KV_HASH_FUNC(h, FakeParamClass<Z_Impl##function :: function##counter + 1>());};


#define DECLARE_GET_TYPE_ITEM(str)            \
    virtual const QString& T_ITEM() const \
    {                                         \
        return T_ITEM_S();                    \
    }                                         \
    static const QString& T_ITEM_S()      \
    {                                         \
        static QString result = #str;     \
        return result;                        \
    }                                         \
    static int REAL_TYPE_ITEM;                \
    virtual int RT_ITEM() const               \
    {                                         \
        return REAL_TYPE_ITEM;                \
    }                                         \
    static int RT_ITEM_S()                    \
    {                                         \
        return REAL_TYPE_ITEM;                \
    }                                         
