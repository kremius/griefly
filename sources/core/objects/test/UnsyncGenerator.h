#pragma once

#include "../MainObject.h"

class UnsyncGenerator: public IMainObject
{
public:
    DECLARE_SAVED(UnsyncGenerator, IMainObject);
    DECLARE_GET_TYPE_ITEM(UnsyncGenerator)
    UnsyncGenerator(quint32 id);
    void PerformUnsync();
private:
    QString KV_SAVEBLE(unsync_string_);

    void SetThisAsUnsyncGenerator();
    KV_ON_LOAD_CALL(SetThisAsUnsyncGenerator);
};
ADD_TO_TYPELIST(UnsyncGenerator);

