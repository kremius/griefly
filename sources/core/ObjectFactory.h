#pragma once

#include <vector>

#include "Idptr.h"
#include "objects/OnMapObject.h"

class IOnMapBase;

class ObjectFactory : public IObjectFactory
{
public:
    ObjectFactory(IGame* game);

    virtual unsigned int GetLastHash() override;
    virtual std::vector<ObjectInfo>& GetIdTable() override;

    virtual void ForeachProcess() override;

    virtual unsigned int Hash() override;

    virtual void Save(std::stringstream& str) override;
    virtual void Load(std::stringstream& str, quint32 real_this_mob) override;
    
    virtual void LoadFromMapGen(const QString& name) override;

    virtual void BeginWorldCreation() override;
    virtual void FinishWorldCreation() override;

    virtual quint32 CreateImpl(const QString& type, quint32 owner = 0) override;

    virtual void DeleteLater(quint32 id) override;
    virtual void ProcessDeletion() override;

    virtual void AddProcessingItem(quint32 item) override;

    virtual void ClearProcessing() override;

    virtual void SetPlayerId(quint32 net_id, quint32 real_id) override;
    virtual quint32 GetPlayerId(quint32 net_id) override;
    virtual quint32 GetNetId(quint32 real_id) override;
private:
    void Clear();

    void SaveMapHeader(std::stringstream& str);
    void LoadMapHeader(std::stringstream& savefile);

    IMainObject* CreateVoid(const QString& hash, quint32 id_new);

    static IMainObject* NewVoidObject(const QString& type, quint32 id);
    static IMainObject* NewVoidObjectSaved(const QString& type);

    IGame* game_;

    QByteArray saved_map_;

    bool is_world_generating_;

    void UpdateProcessingItems();

    std::vector<IMainObject*> ids_to_delete_;

    std::vector<ObjectInfo> objects_table_;
    std::vector<IdPtr<IMainObject>> process_table_;

    std::vector<IdPtr<IMainObject>> add_to_process_;

    unsigned int hash_last_;
    unsigned int hash_last_tick_;

    quint32 id_;

    std::map<quint32, quint32> players_table_;

};
