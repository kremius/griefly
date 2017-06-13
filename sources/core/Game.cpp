#include "Game.h"

#include <iomanip>

#include "KvAbort.h"

#include "Map.h"
#include "representation/Text.h"

#include "SyncRandom.h"
#include "ObjectFactory.h"
#include "net/MagicStrings.h"
#include "objects/Tile.h"
#include "Params.h"

#include "objects/Mob.h"
#include "representation/Chat.h"
#include "Names.h"
#include "objects/Movable.h"
#include "objects/Human.h"
#include "objects/LoginMob.h"
#include "objects/Lobby.h"
#include "objects/SpawnPoints.h"
#include "objects/test/UnsyncGenerator.h"

#include "AutogenMetadata.h"

#include "Version.h"

#include "net/Network2.h"
#include "net/NetworkMessagesTypes.h"
#include "representation/Representation.h"

#include "representation/Screen.h"

#include <QCoreApplication>
#include <QJsonObject>
#include <QByteArray>
#include <QUuid>
#include <QElapsedTimer>
#include <QFileInfo>

using namespace kv;

Game::Game()
{
    process_messages_ns_ = 0;
    foreach_process_ns_ = 0;
    force_process_ns_ = 0;
    atmos_process_ns_ = 0;
    deletion_process_ns_ = 0;
    update_visibility_ns_ = 0;
    frame_generation_ns_ = 0;

    auto_player_ = false;
    cpu_load_ = 0.0f;

    const int LOADS_AMOUNT = 60;
    cpu_loads_.resize(LOADS_AMOUNT);
    cpu_loads_id_ = 0;

    ping_id_ = "";

    current_ping_ = 0;

    ping_send_is_requested_ = true;

    is_end_process_ = false;

    current_connections_ = 0;

    const int MESSAGES_LOG_SIZE = 2 * 1024;
    messages_log_.resize(MESSAGES_LOG_SIZE);
    for (quint32 i = 0; i < messages_log_.size(); ++i)
    {
        messages_log_[i].type = 0;
        messages_log_[i].json.append("(empty)");
    }
    log_pos_ = 0;

    this->moveToThread(&thread_);
    connect(&thread_, &QThread::started, this, &Game::process);

    factory_ = nullptr;
    texts_ = nullptr;
    chat_ = nullptr;
    sync_random_ = nullptr;
    names_= nullptr;
}

Game::~Game()
{
    delete factory_;
    delete texts_;
    delete chat_;
    delete sync_random_;
    delete names_;
}

void Game::InitGlobalObjects()
{
    qDebug() << "Game global object initialization";

    texts_ = new TextPainter;
    atmos_ = new Atmosphere(texts_);
    sync_random_ = new SyncRandom;
    factory_ = new ObjectFactory(this);
    Chat* chat = new Chat(this);
    chat_ = chat;
    names_ = new Names(sync_random_);
    world_loader_saver_ = new WorldLoaderSaver(this);

    qDebug() << "Successfull initialization!";

    qDebug() << "Some QObject moving and connecting";

    texts_->moveToThread(&thread_);
    connect(texts_, &TextPainter::addSystemText, this, &Game::addSystemText);

    qDebug() << "End some moving and connecting";
}

void Game::MakeTiles(int new_map_x, int new_map_y, int new_map_z)
{
    GetMap().Resize(new_map_x, new_map_y, new_map_z);
    for (int x = 0; x < GetMap().GetWidth(); x++)
    {
        for (int y = 0; y < GetMap().GetHeight(); y++)
        {
            for (int z = 0; z < GetMap().GetDepth(); z++)
            {
                IdPtr<CubeTile> tile = GetFactory().CreateImpl(CubeTile::GetTypeStatic());
                tile->SetPos({x, y, z});
                GetMap().At(x, y, z) = tile;
            }
        }
    }
}

void Game::Process()
{
    QElapsedTimer fps_timer;
    fps_timer.start();
    process_in_ = false;

    QElapsedTimer ping_timer;
    ping_timer.start();

    QElapsedTimer cpu_timer;

    int cpu_consumed_ms = 0;

    while (true)
    {
        Network2::GetInstance().WaitForMessageAvailable();

        cpu_timer.start();

        QCoreApplication::processEvents(QEventLoop::AllEvents, 40);
        if (is_end_process_)
        {
            break;
        }
        ProcessInputMessages();

        const int ATMOS_OFTEN = 1;
        const int ATMOS_MOVE_OFTEN = 1;

        if (process_in_)
        {
            QElapsedTimer timer;

            timer.start();
            ProcessBroadcastedMessages();
            process_messages_ns_ += timer.nsecsElapsed();
            process_messages_ns_ /= 2;

            timer.start();
            GetFactory().ForeachProcess();
            foreach_process_ns_ += timer.nsecsElapsed();
            foreach_process_ns_ /= 2;

            timer.start();
            ForceManager::Get().Process();
            force_process_ns_ += timer.nsecsElapsed();
            force_process_ns_ /= 2;

            timer.start();
            if (ATMOS_OFTEN == 1 || MAIN_TICK % ATMOS_OFTEN == 1)
            {
                GetAtmosphere().Process();
            }
            if (ATMOS_MOVE_OFTEN == 1 || MAIN_TICK % ATMOS_MOVE_OFTEN == 1)
            {
                GetAtmosphere().ProcessMove();
            }
            atmos_process_ns_ += timer.nsecsElapsed();
            atmos_process_ns_ /= 2;

            ProcessHearers();

            timer.start();
            GetFactory().ProcessDeletion();
            deletion_process_ns_ += timer.nsecsElapsed();
            deletion_process_ns_ /= 2;

            timer.start();
            GenerateFrame();
            GetTexts().Process();
            frame_generation_ns_ += timer.nsecsElapsed();
            frame_generation_ns_ /= 2;
        }

        cpu_consumed_ms += cpu_timer.elapsed();

        qint64 fps_elapsed = fps_timer.elapsed();
        if (fps_elapsed >= 1000)
        {
            fps_timer.restart();
            cpu_load_ = ((cpu_consumed_ms * 1.0) / fps_elapsed) * 100;
            cpu_consumed_ms = 0;
            cpu_loads_[cpu_loads_id_] = cpu_load_;
            cpu_loads_id_ = (cpu_loads_id_ + 1) % cpu_loads_.size();
        }

        if (   ping_timer.elapsed() > 1000
            && ping_send_is_requested_)
        {
            ping_id_ = QUuid::createUuid().toString();
            Network2::GetInstance().SendPing(ping_id_);
            ping_timer.restart();
            ping_send_is_requested_ = false;
            ping_send_time_.restart();
        }

        process_in_ = false;
    }
    thread_.exit();
}

void Game::ProcessHearers()
{
    QVector<IdPtr<Object>>& hearers = global_objects_->hearers;
    QVector<IdPtr<Object>> deleted_hearers;

    for (const IdPtr<Object>& hearer : qAsConst(hearers))
    {
        if (!hearer.IsValid())
        {
            deleted_hearers.append(hearer);
            continue;
        }
        chat_frame_info_.ApplyHear(hearer->ToHearer());
    }

    for (const IdPtr<Object>& deleted : qAsConst(deleted_hearers))
    {
        hearers.erase(std::find(hearers.begin(), hearers.end(), deleted));
    }
}

const QString ON_LOGIN_MESSAGE =
        "Welcome to Griefly! It is yet another space station remake, so if you are here then you probably"
        " already know how to play."
        " Just in case: arrows for movement, left mouse click for hand actions (hit, attack, take), chat for speaking."
        " Pull objects - ctrl + click, rotate objects - R + click, look at objects - shift + click."
        " Use prefix ooc in the chat if you would like to use the ooc channel (it is a global channel).\n";

void Game::WaitForExit()
{
    thread_.wait();
}

QVector<ObjectInfo>* id_ptr_id_table = nullptr;

void Game::InitWorld(int id, QString map_name)
{   
    InitSettersForTypes();

    qDebug() << "Begin init world";

    InitGlobalObjects();

    qDebug() << "Begin choose map";
    if (map_name == "no_map")
    {
        if (!GetParamsHolder().GetParamBool("mapgen_name"))
        {
            KvAbort("No mapgen param!");
        }

        QString mapgen_name = GetParamsHolder().GetParam<QString>("mapgen_name");
        if (QFileInfo::exists(mapgen_name))
        {
            qsrand(QDateTime::currentDateTime().toMSecsSinceEpoch());
            unsigned int seed = static_cast<unsigned int>(qrand());
            GetRandom().SetRand(seed, 0);

            global_objects_ = GetFactory().CreateImpl(kv::GlobalObjectsHolder::GetTypeStatic());
            global_objects_->map = GetFactory().CreateImpl(kv::Map::GetTypeStatic());

            world_loader_saver_->LoadFromMapGen(GetParamsHolder().GetParam<QString>("mapgen_name"));

            GetFactory().CreateImpl(kv::Lobby::GetTypeStatic());

            if (GetParamsHolder().GetParamBool("-unsync_generation"))
            {
                global_objects_->unsync_generator
                    = GetFactory().CreateImpl(UnsyncGenerator::GetTypeStatic());
            }

            for (auto it = GetFactory().GetIdTable().begin();
                      it != GetFactory().GetIdTable().end();
                    ++it)
            {
                if (it->object && (it->object->GetTypeIndex() == SpawnPoint::GetTypeIndexStatic()))
                {
                    kv::GetLobby().AddSpawnPoint(it->object->GetId());
                }
            }

            IdPtr<LoginMob> newmob = GetFactory().CreateImpl(LoginMob::GetTypeStatic());

            SetPlayerId(id, newmob.Id());
            SetMob(newmob.Id());
            newmob->MindEnter();
        }
        else
        {
            KvAbort(QString("Mapgen file does not exist: %1").arg(mapgen_name));
        }
    }
    else
    {
        qDebug() << "Begin load map";
        QElapsedTimer load_timer;
        load_timer.start();

        QByteArray map_data = Network2::GetInstance().GetMapData();

        if (map_data.length() == 0)
        {
            KvAbort("An empty map received");
        }

        FastDeserializer deserializer(map_data.data(), map_data.size());

        world_loader_saver_->Load(deserializer, id);

        qDebug() << "Map is loaded, " << load_timer.elapsed() << " ms";
    }

    GetChat().PostText(ON_LOGIN_MESSAGE);

    GetTexts()["CpuLoad"].SetUpdater
    ([this](QString* str)
    {
        *str = QString("CPU load: %1%").arg(cpu_load_);
    }).SetFreq(1000);

    GetTexts()["CpuLoadAverage"].SetUpdater
    ([this](QString* str)
    {
        float sum = 0.0f;
        for (float load : cpu_loads_)
        {
            sum += load;
        }
        *str = QString("Average CPU load: %1%").arg(sum / cpu_loads_.size());
    }).SetFreq(1000);

    GetTexts()["Tick"].SetUpdater
    ([&](QString* str)
    {
        *str = QString("Main tick: %1").arg(MAIN_TICK);
    });

    GetTexts()["AmountConnections"].SetUpdater
    ([&](QString* str)
    {
        *str = QString("Players: %1").arg(current_connections_);
    });

    GetTexts()["PingTimer"].SetUpdater
    ([&](QString* str)
    {
        *str = QString("Ping: %1 ms").arg(current_ping_);
    });

    GetTexts()["{Perf}ProcessMessages"].SetUpdater
    ([&](QString* str)
    {
        *str = QString("Process messages: %1 ms")
            .arg((process_messages_ns_ * 1.0) / 1000000.0);
    }).SetFreq(1000);

    GetTexts()["{Perf}ProcessForeach"].SetUpdater
    ([&](QString* str)
    {
        *str = QString("Process objects: %1 ms")
            .arg((foreach_process_ns_ * 1.0) / 1000000.0);
    }).SetFreq(1000);

    GetTexts()["{Perf}ProcessForce"].SetUpdater
    ([&](QString* str)
    {
        *str = QString("Process force movement: %1 ms")
            .arg((force_process_ns_ * 1.0) / 1000000.0);
    }).SetFreq(1000);

    GetTexts()["{Perf}ProcessAtmos"].SetUpdater
    ([&](QString* str)
    {
        *str = QString("Process atmos: %1 ms")
            .arg((atmos_process_ns_ * 1.0) / 1000000.0);
    }).SetFreq(1000);

    GetTexts()["{Perf}ProcessDelete"].SetUpdater
    ([&](QString* str)
    {
        *str = QString("Process deletion: %1 ms")
            .arg((deletion_process_ns_ * 1.0) / 1000000.0);
    }).SetFreq(1000);

    GetTexts()["{Perf}UpdateVisibility"].SetUpdater
    ([&](QString* str)
    {
        *str = QString("Update visibility: %1 ms")
            .arg((update_visibility_ns_ * 1.0) / 1000000.0);
    }).SetFreq(1000);

    GetTexts()["{Perf}FrameGeneration"].SetUpdater
    ([&](QString* str)
    {
        *str = QString("Frame generation: %1 ms")
            .arg((frame_generation_ns_ * 1.0) / 1000000.0);
    }).SetFreq(1000);

    thread_.start();
}

void Game::ProcessInputMessages()
{
    while (Network2::GetInstance().IsMessageAvailable())
    {
        Message2 msg = Network2::GetInstance().PopMessage();

        AddMessageToMessageLog(msg);

        if (msg.type == MessageType::NEW_TICK)
        {
            process_in_ = true;
            ++MAIN_TICK;
            break;
        }
        if (msg.type == MessageType::NEW_CLIENT)
        {
            QJsonObject obj = Network2::ParseJson(msg);

            QJsonValue new_id_v = obj["id"];
            int new_id = new_id_v.toVariant().toInt();

            quint32 game_id = GetPlayerId(new_id);

            if (game_id != 0)
            {
                qDebug() << "game_id " << game_id << " already exists";
                continue;
            }

            IdPtr<LoginMob> newmob = GetFactory().CreateImpl(LoginMob::GetTypeStatic());
            SetPlayerId(new_id, newmob.Id());
            newmob->MindEnter();

            qDebug() << "New client " << newmob.Id();
            continue;
        }
        if (msg.type == MessageType::MAP_UPLOAD)
        {
            CheckMessagesOrderCorrectness();
            QElapsedTimer timer;
            timer.start();
            QJsonObject obj = Network2::ParseJson(msg);
            QString map_url = obj["url_to_upload_map"].toString();

            QJsonValue tick_v = obj["tick"];
            int tick = tick_v.toVariant().toInt();

            qDebug() << "Map upload to " << map_url << ", tick " << tick;
            qDebug() << "Current game tick: " << MAIN_TICK;

            QByteArray data;

            if (tick == MAIN_TICK)
            {
                qDebug() << "Map will be generated";

                serializer_.ResetIndex();
                world_loader_saver_->Save(serializer_);
                data = QByteArray(serializer_.GetData(), serializer_.GetIndex());

                AddLastMessages(&data);
                AddBuildInfo(&data);

                qDebug() << " " << data.length();
            }

            emit sendMap(map_url, data);
            qDebug() << "It took " << timer.elapsed() << "ms to send the map";
            continue;
        }

        if (msg.type == MessageType::REQUEST_HASH)
        {
            CheckMessagesOrderCorrectness();
            //qDebug() << "Hash: " << msg.json;
            QJsonObject obj = Network2::ParseJson(msg);

            QJsonValue tick_v = obj["tick"];
            int tick = tick_v.toVariant().toInt();

            if (tick != MAIN_TICK)
            {
                KvAbort(QString("Tick mismatch! %1 %2").arg(tick).arg(MAIN_TICK));
            }
            unsigned int hash = GetFactory().Hash();

            Message2 msg;

            msg.type = MessageType::HASH_MESSAGE;
            msg.json.append(
                      "{\"hash\":"
                    + QString::number(hash)
                    + ",\"tick\":"
                    + QString::number(MAIN_TICK)
                    + "}");

            Network2::GetInstance().SendMsg(msg);

            continue;
        }

        if (msg.type == MessageType::PING)
        {
            QJsonObject obj = Network2::ParseJson(msg);

            QString ping_id = obj["ping_id"].toString();

            if (ping_id != ping_id_)
            {
                continue;
            }

            current_ping_ = ping_send_time_.elapsed();
            ping_send_is_requested_ = true;
            continue;
        }
        if (msg.type == MessageType::CURRENT_CONNECTIONS)
        {
            QJsonObject obj = Network2::ParseJson(msg);
            QJsonValue amount_v = obj["amount"];
            current_connections_ = amount_v.toVariant().toInt();
            continue;
        }
        if (msg.type == MessageType::OOC_MESSAGE)
        {
            QJsonObject obj = Network2::ParseJson(msg);
            QString login = obj["login"].toString();
            QString text = obj["text"].toString();
            PostOoc(login, text);
            continue;
        }

        if (msg.type == MessageType::CLIENT_IS_OUT_OF_SYNC)
        {
            GetChat().PostText("The client is out of sync, so the server will drop the connection. Try to reconnect.");
            continue;
        }
        if (msg.type == MessageType::CLIENT_TOO_SLOW)
        {
            GetChat().PostText("The client is too slow, so the server will drop the connection. Try to reconnect.");
            continue;
        }
        if (msg.type == MessageType::SERVER_IS_RESTARTING)
        {
            GetChat().PostText("The server is restarting, so the connection will be dropped. Try to reconnect.");
            continue;
        }
        if (msg.type == MessageType::EXIT_SERVER)
        {
            GetChat().PostText("The server is near to exit, so it will drop the connection. Try to reconnect.");
            continue;
        }  
        if (   msg.type == MessageType::ORDINARY
            || msg.type == MessageType::MOUSE_CLICK
            || msg.type == MessageType::MESSAGE)
        {
            messages_to_process_.push_back(msg);
            continue;
        }

        qDebug() << "Unhandled message, type: " << msg.type << ", json: " << msg.json;
        // TODO: other stuff
    }
}

void Game::GenerateFrame()
{
    points_.clear();
    GetMob()->CalculateVisible(&points_);
    GetMap().Represent(points_);
    GetMob()->GenerateInterfaceForFrame();

    AppendSoundsToFrame(points_);
    GetChatFrameInfo().AddFromVisibleToPersonal(points_, GetNetId(GetMob().Id()));
    AppendChatMessages();

    // TODO: reset all shifts
    GetRepresentation().SetCameraForFrame(GetMob()->GetPosition().x, GetMob()->GetPosition().y);
    GetRepresentation().Swap();
}

void Game::AppendSoundsToFrame(const VisiblePoints& points)
{
    // TODO: Sounds for frame may be hash table or some sorted vector
    // now scalability is quite bad
    for (auto it : qAsConst(sounds_for_frame_))
    {
        if (std::find(points.begin(), points.end(), it.first) != points.end())
        {
            GetRepresentation().AddToNewFrame(Representation::Sound{it.second});
        }
    }
    sounds_for_frame_.clear();

    quint32 net_id = GetNetId(GetMob().Id());

    auto& musics_for_mobs = global_objects_->musics_for_mobs;

    auto music = musics_for_mobs.find(net_id);
    if (music != musics_for_mobs.end())
    {
        GetRepresentation().SetMusic({music->first, music->second});
    }
}

void Game::AppendChatMessages()
{
    quint32 net_id = GetNetId(GetMob().Id());

    for (const auto& personal : chat_frame_info_.GetPersonalTexts(net_id))
    {
        GetRepresentation().AddToNewFrame(Representation::ChatMessage{personal});
    }
    chat_frame_info_.Reset();
}

void Game::PlayMusic(const QString& name, int volume, quint32 mob)
{
    qDebug() << "Music playing:" << mob << name << volume;
    auto& musics_for_mobs = global_objects_->musics_for_mobs;
    musics_for_mobs[mob] = {name, volume};
}

void Game::AddSound(const QString& name, Position position)
{
    sounds_for_frame_.append({position, name});
}

AtmosInterface& Game::GetAtmosphere()
{
    return *atmos_;
}

MapInterface& Game::GetMap()
{
    return *(global_objects_->map);
}

const MapInterface& Game::GetMap() const
{
    return *(global_objects_->map);
}

ObjectFactoryInterface& Game::GetFactory()
{
    return *factory_;
}

ChatInterface& Game::GetChat()
{
    return *chat_;
}

TextPainter& Game::GetTexts()
{
    return *texts_;
}

SyncRandom& Game::GetRandom()
{
    return *sync_random_;
}

Names& Game::GetNames()
{
    return *names_;
}

ChatFrameInfo& Game::GetChatFrameInfo()
{
    return chat_frame_info_;
}

IdPtr<Mob> Game::GetMob()
{
    return current_mob_;
}

void Game::SetMob(quint32 new_mob)
{
    current_mob_ = new_mob;
}

IdPtr<GlobalObjectsHolder> Game::GetGlobals()
{
    return global_objects_;
}

void Game::SetGlobals(quint32 globals)
{
    global_objects_ = globals;
}

void Game::process()
{
    Process();
}

void Game::endProcess()
{
    is_end_process_ = true;
    qDebug() << "void Game::endProcess()";
}

void Game::generateUnsync()
{
    if (global_objects_->unsync_generator.IsValid())
    {
        global_objects_->unsync_generator->PerformUnsync();
    }
}

void Game::AddLastMessages(QByteArray* data)
{
    data->append('\n');
    for (int i = (log_pos_ + 1) % messages_log_.size();
             i != log_pos_;
             i = (i + 1) % messages_log_.size())
    {
        data->append(QByteArray::number(messages_log_[i].type) + " ");
        data->append(messages_log_[i].json + '\n');
    }
}

void Game::AddMessageToMessageLog(Message2 message)
{
    messages_log_[log_pos_] = message;
    log_pos_ = (log_pos_ + 1) % messages_log_.size();
}

void Game::AddBuildInfo(QByteArray* data)
{
    QString system_info("Build info: %1, Qt: %2");
    system_info = system_info.arg(GetBuildInfo()).arg(GetQtVersion());
    data->append(system_info);
}

void Game::PostOoc(const QString& who, const QString& text)
{
    QString escaped_who = who.toHtmlEscaped();
    QString escaped_text = text.toHtmlEscaped();

    const QString post_text
        = QString("<font color=\"blue\"><b>%1</b>: <span>%2</span></font>")
                .arg(escaped_who).arg(escaped_text);

    const auto& players = GetGlobals()->players_table;
    for (auto it = players.begin(); it != players.end(); ++it)
    {
        GetChatFrameInfo().PostPersonal(post_text, it.key());
    }
}

void Game::ProcessBroadcastedMessages()
{
    for (auto it = messages_to_process_.begin();
              it != messages_to_process_.end();
            ++it)
    {
         QJsonObject obj = Network2::ParseJson(*it);
         QJsonValue v = obj["id"];
         int net_id = v.toVariant().toInt();
         quint32 game_id = GetPlayerId(net_id);
         if (game_id == 0)
         {
             qDebug() << "Game id is 0";
         }
         IdPtr<Mob> game_object = game_id;

         if (game_object.IsValid())
         {
             game_object->ProcessMessage(*it);
         }
         else
         {
             KvAbort(QString("Game object is not valid: %1").arg(net_id));
         }
    }
    messages_to_process_.clear();
}

void Game::CheckMessagesOrderCorrectness()
{
    if (!messages_to_process_.empty())
    {
        KvAbort("CheckMessagesOrderCorrectness fail");
    }
}

void Game::SetPlayerId(quint32 net_id, quint32 real_id)
{
    global_objects_->players_table[net_id] = real_id;
}
quint32 Game::GetPlayerId(quint32 net_id) const
{
    auto& players_table = global_objects_->players_table;
    auto it = players_table.find(net_id);
    if (it != players_table.end())
    {
        return it.value();
    }
    return 0;
}

quint32 Game::GetNetId(quint32 real_id) const
{
    auto& players_table = global_objects_->players_table;
    for (auto it = players_table.begin(); it != players_table.end(); ++it)
    {
        if (it.value() == real_id)
        {
            return it.key();
        }
    }
    return 0;
}
