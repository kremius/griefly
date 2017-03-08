#include "ObjectFactory.h"

#include "KVAbort.h"

#include "objects/MainObject.h"
#include "objects/OnMapObject.h"
#include "Game.h"
#include "Map.h"
#include "SyncRandom.h"
#include "net/MagicStrings.h"
#include "net/NetworkMessagesTypes.h"
#include "AutogenMetadata.h"

#include "MapEditor.h"

ObjectFactory::ObjectFactory(IGame* game)
{
    objects_table_.resize(100);
    id_ = 1;
    is_world_generating_ = true;
    game_ = game;
    id_ptr_id_table = &objects_table_;
}

ObjectFactory::~ObjectFactory()
{
    ProcessDeletion();
}

std::vector<ObjectInfo>& ObjectFactory::GetIdTable()
{
    return objects_table_;
}

void ObjectFactory::UpdateProcessingItems()
{   
    if (!add_to_process_.size())
    {
        return;
    }
    for (auto it = add_to_process_.begin(); it != add_to_process_.end(); ++it)
    {
        if (!(*it).IsValid())
        {
            continue;
        }
        if ((*it)->GetFreq() == 0)
        {
            continue;
        }
        auto to_add = std::find(process_table_.begin(), process_table_.end(), *it);
        if (to_add == process_table_.end())
        {
            process_table_.push_back(*it);
        }
    }
    std::sort(process_table_.begin(), process_table_.end(),
    [](IdPtr<IMainObject> item1, IdPtr<IMainObject> item2)
    {
        return item1.Id() < item2.Id();
    });
    add_to_process_.clear();
}

void ObjectFactory::ForeachProcess()
{
    UpdateProcessingItems();

    const int CLEAR_TICK = 10;
    if (MAIN_TICK % CLEAR_TICK == 1)
    {
        ClearProcessing();
    }

    quint32 table_size = process_table_.size();
    for (quint32 i = 0; i < table_size; ++i)
    {
        if (!(   process_table_[i].IsValid()
              && process_table_[i]->GetFreq()))
        {
            continue;
        }
        else if ((MAIN_TICK % process_table_[i]->GetFreq()) == 0)
        {
            process_table_[i]->Process();
        }
    }
}

void ObjectFactory::SaveMapHeader(FastSerializer& savefile)
{
	auto random = game_->GetRandom();
	auto map = game_->GetMap();
    savefile << MAIN_TICK;
    savefile << id_;

    // Random save
    savefile << random.GetSeed();
    savefile << random.GetCallsCounter();

    // Save Map Size

    savefile << map.GetWidth();
    savefile << map.GetHeight();
    savefile << map.GetDepth();

    // Save player table
    savefile << static_cast<quint32>(players_table_.size());
    for (auto it = players_table_.begin(); it != players_table_.end(); ++it)
    {
        savefile << it->first;
        savefile << it->second;
    }
}

void ObjectFactory::LoadMapHeader(FastDeserializer& savefile)
{
    savefile >> MAIN_TICK;
    qDebug() << "MAIN_TICK: " << MAIN_TICK;

    savefile >> id_;
    qDebug() << "id_: " << id_;
    
    unsigned int new_seed;
    unsigned int new_calls_counter;
    savefile >> new_seed;
    savefile >> new_calls_counter;

    game_->GetRandom().SetRand(new_seed, new_calls_counter);

    objects_table_.resize(id_ + 1);

    // Load map size
    int x;
    int y;
    int z;

    savefile >> x;
    savefile >> y;
    savefile >> z;

    game_->GetMap().ResizeMap(x, y, z);

    // Load player table
    quint32 s;
    savefile >> s;
    for (quint32 i = 0; i < s; ++i)
    {
        quint32 first;
        savefile >> first;
        quint32 second;
        savefile >> second;

        qDebug() << first;
        qDebug() << second;
        SetPlayerId(first, second);
    }
}

void ObjectFactory::Save(FastSerializer& savefile)
{
    SaveMapHeader(savefile);

    auto it = ++objects_table_.begin();
    while (it != objects_table_.end())
    {
        if (it->object)
        {
            it->object->Save(savefile);
        }
        ++it;
    }

    savefile.WriteType(END_TYPE);
}

void ObjectFactory::Load(FastDeserializer& savefile, quint32 real_this_mob)
{
    Clear();

    LoadMapHeader(savefile);
    while (!savefile.IsEnd())
    {
        QString type;
        savefile.ReadType(&type);

        if (type == END_TYPE)
        {
            qDebug() << "Zero id reached";
            break;
        }

        quint32 id_loc;
        savefile >> id_loc;

        IMainObject* object = CreateVoid(type, id_loc);
        object->Load(savefile);
    }

    quint32 player_id = GetPlayerId(real_this_mob);
    game_->SetMob(player_id);
    qDebug() << "Player id:" << player_id;
    game_->ChangeMob(player_id);
    is_world_generating_ = false;

    game_->GetMap().GetAtmosphere().LoadGrid();
}

void ObjectFactory::LoadFromMapGen(const QString& name)
{
    Clear();

    QFile file(name);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Error open: " << name;
        KvAbort();
    }

    QByteArray raw_data;
    while (file.bytesAvailable())
    {
        QByteArray local = file.readLine();
        if (local.size() < 1)
        {
            break;
        }
        local = local.left(local.size() - 1);
        raw_data.append(local);
    }
    raw_data = QByteArray::fromHex(raw_data);
    FastDeserializer ss(raw_data.data(), raw_data.size());

    BeginWorldCreation();

    int x;
    int y;
    int z;

    ss >> x;
    ss >> y;
    ss >> z;

    game_->MakeTiles(x, y, z);

    qDebug() << "Begin loading cycle";
    while (!ss.IsEnd())
    {
        QString item_type;
        qint32 x;
        qint32 y;
        qint32 z;

        ss.ReadType(&item_type);

        ss >> x;
        ss >> y;
        ss >> z;

        //qDebug() << "Create<IOnMapObject>" << &game_->GetFactory();
        //qDebug() << "Create<IOnMapObject> " << QString::fromStdString(t_item);
        IdPtr<IOnMapObject> i = CreateImpl(item_type);
        if (!i.IsValid())
        {
            qDebug() << "Unable to cast: " << item_type;
            KvAbort();
        }
        //qDebug() << "Success!";

        MapgenVariablesType variables;
        WrapReadMessage(ss, variables);

        for (auto it = variables.begin(); it != variables.end(); ++it)
        {
            if ((it->second.size() == 0) || (it->first.size() == 0))
            {
                continue;
            }

            QByteArray variable_data = it->second;

            FastDeserializer local(variable_data.data(), variable_data.size());

            auto& setters_for_type = GetSettersForTypes();
            setters_for_type[item_type][it->first](i.operator*(), local);
        }

        //qDebug() << "id_ptr_on<ITurf> t = i";
        if (IdPtr<ITurf> t = i)
        {
            if (game_->GetMap().GetSquares()[x][y][z]->GetTurf())
            {
                qDebug() << "DOUBLE TURF!";
            }
            game_->GetMap().GetSquares()[x][y][z]->SetTurf(t);
        }
        else
        {
            game_->GetMap().GetSquares()[x][y][z]->AddItem(i);
        }
    }
    FinishWorldCreation();
}

IMainObject* ObjectFactory::NewVoidObject(const QString& type, quint32 id)
{
    return (*GetItemsCreators())[type](id);
}

IMainObject* ObjectFactory::NewVoidObjectSaved(const QString& type)
{
    return (*GetVoidItemsCreators())[type]();
}

void ObjectFactory::Clear()
{
    quint32 table_size = objects_table_.size();
    if (table_size != objects_table_.size())
    {
        qDebug() << "WARNING: table_size != idTable_.size()!";
    }

    ids_to_delete_.clear();
    process_table_.clear();
    add_to_process_.clear();

    id_ = 1;
}

void ObjectFactory::BeginWorldCreation()
{
    is_world_generating_ = true;
}

void ObjectFactory::FinishWorldCreation()
{
    is_world_generating_ = false;
    quint32 table_size = objects_table_.size();
    for (quint32 i = 1; i < table_size; ++i)
    {
        if (objects_table_[i].object != nullptr)
        {
            objects_table_[i].object->AfterWorldCreation();
        }
    }
}

quint32 ObjectFactory::CreateImpl(const QString &type, quint32 owner_id)
{
    IMainObject* item = NewVoidObject(type, id_);
    item->SetGame(game_);

    if (id_ >= objects_table_.size())
    {
        objects_table_.resize(id_ * 2);
    }
    objects_table_[id_].object = item;
    quint32 retval = id_;
    ++id_;
    IdPtr<IOnMapBase> owner = owner_id;
    if (owner.IsValid())
    {
        if (CastTo<ITurf>(item) != nullptr)
        {
            owner->SetTurf(item->GetId());
        }
        else if (!owner->AddItem(item->GetId()))
        {
            qDebug() << "AddItem failed";
            KvAbort();
        }
    }

    if (!is_world_generating_)
    {
        item->AfterWorldCreation();
    }
    return retval;
}

IMainObject* ObjectFactory::CreateVoid(const QString &hash, quint32 id_new)
{
    IMainObject* item = NewVoidObjectSaved(hash);
    item->SetGame(game_);
    if (id_new >= objects_table_.size())
    {
        objects_table_.resize(id_new * 2);
    }

    if (id_new >= id_)
    {
        id_ = id_new + 1;
    }
    objects_table_[id_new].object = item;
    item->SetId(id_new);
    return item;
}

void ObjectFactory::DeleteLater(quint32 id)
{
    ids_to_delete_.push_back(objects_table_[id].object);
    objects_table_[id].object = nullptr;
}

void ObjectFactory::ProcessDeletion()
{
    for (auto it = ids_to_delete_.begin(); it != ids_to_delete_.end(); ++it)
    {
        delete *it;
    }
    ids_to_delete_.clear();
}

unsigned int ObjectFactory::Hash()
{
    unsigned int h = 0;
    quint32 table_size = objects_table_.size();
    for (quint32 i = 1; i < table_size; ++i)
    {
        if (objects_table_[i].object != nullptr)
        {
            h += objects_table_[i].object->Hash();
        }
    }

    ForceManager::Get().Clear();
    h += ForceManager::Get().Hash();

    ClearProcessing();

    int i = 1;
    for (auto p = process_table_.begin(); p != process_table_.end(); ++p)
    {
        h += p->Id() * i;
        i++;
    }

    return h;
}

void ObjectFactory::SetPlayerId(quint32 net_id, quint32 real_id)
{
    players_table_[net_id] = real_id;
}
quint32 ObjectFactory::GetPlayerId(quint32 net_id)
{
    auto it = players_table_.find(net_id);
    if (it != players_table_.end())
    {
        return it->second;
    }
    return 0;
}

quint32 ObjectFactory::GetNetId(quint32 real_id)
{
    for (auto it = players_table_.begin(); it != players_table_.end(); ++it)
    {
        if (it->second == real_id)
        {
            return it->first;
        }
    }
    return 0;
}

void ObjectFactory::AddProcessingItem(quint32 item)
{
    add_to_process_.push_back(item);
}

void ObjectFactory::ClearProcessing()
{
    std::vector<IdPtr<IMainObject>> remove_from_process;
    quint32 table_size = process_table_.size();
    for (quint32 i = 0; i < table_size; ++i)
    {
        if (!(   process_table_[i].IsValid()
              && process_table_[i]->GetFreq()))
        {
            remove_from_process.push_back(process_table_[i]);
        }
    }

    for (auto it = remove_from_process.begin(); it != remove_from_process.end(); ++it)
    {
        process_table_.erase(std::find(process_table_.begin(), process_table_.end(), *it));
    }
}
