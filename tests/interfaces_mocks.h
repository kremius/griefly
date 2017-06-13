#pragma once

#include <gmock/gmock.h>

#include <qglobal.h>

#include "Interfaces.h"

#include "representation/Text.h"
#include "core/SyncRandom.h"
#include "core/Names.h"
#include "core/ChatFrameInfo.h"

class MockIAtmosphere : public AtmosInterface
{
public:
    MOCK_METHOD0(Process, void());
    MOCK_METHOD0(ProcessMove, void());

    MOCK_METHOD4(SetFlags, void(quint32 x, quint32 y, quint32 z, Flags flags));
    MOCK_METHOD1(LoadGrid, void(MapInterface*));
};

class MockIMap : public MapInterface
{
public:
    MOCK_CONST_METHOD0(GetWidth, int());
    MOCK_CONST_METHOD0(GetHeight, int());
    MOCK_CONST_METHOD0(GetDepth, int());
    MOCK_CONST_METHOD1(Represent, void(const VisiblePoints& points));
    MOCK_METHOD3(Resize, void(int new_map_x, int new_map_y, int new_map_z));
    MOCK_CONST_METHOD3(At, const SqType&(int x, int y, int z));
    MOCK_METHOD3(At, SqType&(int x, int y, int z));
    MOCK_METHOD0(FillTilesAtmosHolders, void());
    MOCK_CONST_METHOD4(CalculateLos, void(VisiblePoints*, int, int, int));
    MOCK_METHOD1(IsTileVisible, bool(quint32 tile_id));
    MOCK_CONST_METHOD3(IsTransparent, bool(int, int, int));
};

class MockIObjectFactory : public ObjectFactoryInterface
{
public:
    MOCK_METHOD0(GetIdTable, QVector<ObjectInfo>&());
    MOCK_METHOD0(ForeachProcess, void());
    MOCK_METHOD0(Hash, unsigned int());
    MOCK_METHOD0(BeginWorldCreation, void());
    MOCK_METHOD0(FinishWorldCreation, void());
    MOCK_METHOD0(MarkWorldAsCreated, void());
    MOCK_METHOD2(CreateImpl, quint32(const QString&, quint32));
    MOCK_METHOD2(CreateVoid, kv::Object*(const QString& hash, quint32 id_new));
    MOCK_METHOD1(DeleteLater, void(quint32 id));
    MOCK_METHOD0(ProcessDeletion, void());
    MOCK_METHOD1(AddProcessingItem, void(quint32 item));
    MOCK_METHOD0(Clear, void());
    MOCK_METHOD0(GetId, int());
    MOCK_METHOD1(SetId, void(int id));
};

class MockIChat : public ChatInterface {
public:
    MOCK_METHOD1(PostText, void(const QString& str));
};

class MockIGame : public GameInterface
{
public:
    MOCK_METHOD0(GetAtmosphere, AtmosInterface&());
    MOCK_METHOD0(GetMap, MapInterface&());
    MOCK_CONST_METHOD0(GetMap, const MapInterface&());
    MOCK_METHOD0(GetFactory, ObjectFactoryInterface&());
    MOCK_METHOD0(GetChat, ChatInterface&());
    MOCK_METHOD0(GetTexts, TextPainter&());
    MOCK_METHOD0(GetRandom, SyncRandom&());
    MOCK_METHOD0(GetNames, Names&());
    MOCK_METHOD0(GetChatFrameInfo, kv::ChatFrameInfo&());
    MOCK_METHOD3(MakeTiles, void(int size_x, int size_y, int size_z));
    MOCK_METHOD1(SetUnsyncGenerator, void(quint32 generator));
    MOCK_METHOD0(GetMob, IdPtr<kv::Mob>());
    MOCK_METHOD1(SetMob, void(quint32 new_mob));
    MOCK_METHOD0(GetGlobals, IdPtr<kv::GlobalObjectsHolder>());
    MOCK_METHOD1(SetGlobals, void(quint32 globals));
    MOCK_METHOD3(PlayMusic, void(const QString& name, int volume, quint32 mob));
    MOCK_METHOD2(AddSound, void(const QString& name, kv::Position position));
    MOCK_METHOD2(SetPlayerId, void(quint32, quint32));
    MOCK_CONST_METHOD1(GetPlayerId, quint32(quint32 net_id));
    MOCK_CONST_METHOD1(GetNetId, quint32(quint32 real_id));
};

