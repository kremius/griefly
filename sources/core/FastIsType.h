#pragma once

#include <QVector>
#include <QBitArray>

#if defined(__linux__)
#define __forceinline __attribute__((always_inline)) inline
#endif

extern QVector<QBitArray>* cast_table;

__forceinline bool FastIsType(int typeto, int typefrom)
{
    return (*cast_table)[typefrom].testBit(typeto);
}

void InitCastTable();

template<typename Typeto>
inline bool FastIsType(int typefrom)
{
    return FastIsType(Typeto::RT_ITEM_S(), typefrom);
}

template<typename Typeto, typename TypeObjectFrom>
inline Typeto* CastTo(TypeObjectFrom* ptr)
{
    if (ptr == nullptr)
    {
        return nullptr;
    }
    if (FastIsType<Typeto>(ptr->RT_ITEM()))
    {
        return reinterpret_cast<Typeto*>(ptr);
    }
    return nullptr;
}
