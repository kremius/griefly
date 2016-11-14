#pragma once

#include "MainObject.h"
#include "SpawnPoints.h"
#include "Tile.h"

class Lobby: public IMainObject
{
public:
    DECLARE_SAVED(Lobby, IMainObject);
    DECLARE_GET_TYPE_ITEM(Lobby)
    Lobby(quint32 id);
    virtual void AfterWorldCreation() override;

    int GetSecondUntilStart();
    virtual void Process() override;

    void AddSpawnPoint(IdPtr<SpawnPoint> PosPoint);
    std::vector<IdPtr<CubeTile>> GetTilesFor(const QString& type);

    int KV_SAVEBLE(security_score_);
    int KV_SAVEBLE(janitors_score_);
private:
    int KV_SAVEBLE(seconds_);
    std::vector<IdPtr<SpawnPoint>> KV_SAVEBLE(spawn_points_);

    KV_ON_LOAD_CALL(SetThisAsLobby);
    void SetThisAsLobby();
};
ADD_TO_TYPELIST(Lobby);

void SetLobby(Lobby* lobby);
Lobby& GetLobby();
