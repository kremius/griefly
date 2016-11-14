#pragma once

#include "OnMapObject.h"

class SpawnPoint: public IOnMapObject
{
public:
    DECLARE_SAVED(SpawnPoint, IOnMapObject);
    DECLARE_GET_TYPE_ITEM(SpawnPoint);
    SpawnPoint(quint32 id);

    virtual void AfterWorldCreation() override;
    QString KV_SAVEBLE(spawn_type);
};
ADD_TO_TYPELIST(SpawnPoint);
