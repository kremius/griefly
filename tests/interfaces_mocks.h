#pragma once

#include <gmock/gmock.h>

#include "Interfaces.h"

#include "representation/Text.h"
#include "core/SyncRandom.h"
#include "core/Names.h"

class MockIAtmosphere : public IAtmosphere
{
public:
    MOCK_METHOD3(Resize, void(quint32 x, quint32 y, quint32 z));
    MOCK_METHOD0(Process, void());
    MOCK_METHOD0(ProcessMove, void());
};

class MockIMapMaster : public IMapMaster
{
public:
    MOCK_CONST_METHOD0(GetWidth, int());
    MOCK_CONST_METHOD0(GetHeight, int());
    MOCK_CONST_METHOD0(GetDepth, int());
    MOCK_METHOD0(Represent, void());
    MOCK_METHOD3(ResizeMap, void(int new_map_x, int new_map_y, int new_map_z));
    MOCK_METHOD0(GetSquares, std::vector<std::vector<std::vector<SqType>>>&());
    MOCK_CONST_METHOD0(GetSquares, const std::vector<std::vector<std::vector<SqType>>>&());
    MOCK_METHOD0(GetVisiblePoints, std::list<PosPoint>*());
    MOCK_METHOD0(GetAtmosphere, IAtmosphere&());
    MOCK_METHOD0(FillAtmosphere, void());
    MOCK_METHOD4(CalculateVisisble, void(std::list<PosPoint>*, int, int, int));
    MOCK_METHOD1(IsTileVisible, bool(quint32 tile_id));
    MOCK_METHOD3(IsTransparent, bool(int, int, int));
    MOCK_CONST_METHOD3(CheckBorders, bool(const int* x, const int* y, const int* z));
};

class MockIObjectFactory : public IObjectFactory
{
public:
    MOCK_METHOD0(GetLastHash, unsigned int());
    MOCK_METHOD0(GetIdTable, std::vector<ObjectInfo>&());
    MOCK_METHOD0(ForeachProcess, void());
    MOCK_METHOD0(Hash, unsigned int());
    MOCK_METHOD1(Save, void(std::stringstream& str));
    MOCK_METHOD2(Load, void(std::stringstream& str, quint32 real_this_mob));
    MOCK_METHOD1(LoadFromMapGen, void(const QString& name));
    MOCK_METHOD0(BeginWorldCreation, void());
    MOCK_METHOD0(FinishWorldCreation, void());
    MOCK_METHOD2(CreateImpl, quint32(const QString&, quint32));
    MOCK_METHOD1(DeleteLater, void(quint32 id));
    MOCK_METHOD0(ProcessDeletion, void());
    MOCK_METHOD1(AddProcessingItem, void(quint32 item));
    MOCK_METHOD0(ClearProcessing, void());
    MOCK_METHOD2(SetPlayerId, void(quint32 net_id, quint32 real_id));
    MOCK_METHOD1(GetPlayerId, quint32(quint32 net_id));
    MOCK_METHOD1(GetNetId, quint32(quint32 real_id));
};

class MockIChat : public IChat {
public:
    MOCK_METHOD2(PostTextFor, void(const QString& str, IdPtr<IOnMapBase> owner));
    MOCK_METHOD1(PostText, void(const QString& str));
    MOCK_METHOD2(PostOOCText, void(const QString& who, const QString& str));
    MOCK_METHOD2(PostSimpleText, void(const QString& str, quint32 tile_id));
    MOCK_METHOD4(PostDamage, void(const QString& by, const QString& who,
                                const QString& object, quint32 tile_id));
    MOCK_METHOD3(PostWords, void(const QString& who, const QString& text, quint32 tile_id));
};

class MockIGame : public IGame
{
public:
    MOCK_METHOD0(GetMap, IMapMaster&());
    MOCK_CONST_METHOD0(GetMap, const IMapMaster&());
    MOCK_METHOD0(GetFactory, IObjectFactory&());
    MOCK_METHOD0(GetChat, IChat&());
    MOCK_METHOD0(GetTexts, TextPainter&());
    MOCK_METHOD0(GetRandom, SyncRandom&());
    MOCK_METHOD0(GetNames, Names&());
    MOCK_METHOD3(MakeTiles, void(int size_x, int size_y, int size_z));
    MOCK_METHOD1(SetUnsyncGenerator, void(quint32 generator));
    MOCK_METHOD1(ChangeMob, void(IdPtr<IMob> new_mob));
    MOCK_METHOD0(GetMob, IdPtr<IMob>());
    MOCK_METHOD1(SetMob, void(quint32 new_mob));
    MOCK_METHOD2(PlayMusic, void(const QString& name, float volume));
    MOCK_METHOD1(AddSound, void(const QString& name));
};

