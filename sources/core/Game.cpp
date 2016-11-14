#include "Game.h"

#include <iomanip>

#include "KVAbort.h"

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

int ping_send;

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

    map_ = nullptr;
    factory_ = nullptr;
    texts_ = nullptr;
    chat_ = nullptr;
    sync_random_ = nullptr;
    names_= nullptr;

    unsync_generator_ = 0;
    current_mob_ = 0;
}

Game::~Game()
{
    delete map_;
    delete factory_;
    delete texts_;
    delete chat_;
    delete sync_random_;
    delete names_;
}

void Game::InitGlobalObjects()
{
    qDebug() << "Begin init global objects";
    TextPainter* texts = new TextPainter;
    texts_ = texts;
    sync_random_ = new SyncRandom;
    qDebug() << "Begin master load";
    map_ = new MapMaster(sync_random_, texts);
    qDebug() << "End master load";
    factory_ = new ObjectFactory(this);
    id_ptr_id_table = &(factory_->GetIdTable());
    Chat* chat = new Chat(this);
    chat_ = chat;
    names_ = new Names(sync_random_);
    qDebug() << "End init global objects";

    qDebug() << "Some moving and connecting";
    chat->moveToThread(&thread_);
    texts->moveToThread(&thread_);

    connect(chat, &Chat::insertHtmlIntoChat, this, &Game::insertHtmlIntoChat);
    connect(texts, &TextPainter::addSystemText, this, &Game::addSystemText);
    qDebug() << "End some moving and connecting";
}

void Game::MakeTiles(int new_map_x, int new_map_y, int new_map_z)
{
    GetMap().ResizeMap(new_map_x, new_map_y, new_map_z);
    for(int x = 0; x < GetMap().GetWidth(); x++)
    {
        for(int y = 0; y < GetMap().GetHeight(); y++)
        {
            for (int z = 0; z < GetMap().GetDepth(); z++)
            {
                IdPtr<CubeTile> loc = GetFactory().CreateImpl(CubeTile::T_ITEM_S());
                loc->SetPos(x, y, z);
                GetMap().GetSquares()[x][y][z] = loc;
            }
        }
    }
}

void Game::UpdateVisible() 
{
    GetMap().GetVisiblePoints()->clear();
    GetMob()->CalculateVisible(GetMap().GetVisiblePoints());
}

void Game::Process()
{
    QElapsedTimer fps_timer;
    fps_timer.start();
    lps_ = 0;
    process_in_ = false;

    QElapsedTimer ping_timer;
    ping_timer.start();

    while (true)
    {
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
                GetMap().GetAtmosphere().Process();
            }
            if (ATMOS_MOVE_OFTEN == 1 || MAIN_TICK % ATMOS_MOVE_OFTEN == 1)
            {
                GetMap().GetAtmosphere().ProcessMove();
            }
            atmos_process_ns_ += timer.nsecsElapsed();
            atmos_process_ns_ /= 2;

            timer.start();
            GetFactory().ProcessDeletion();
            deletion_process_ns_ += timer.nsecsElapsed();
            deletion_process_ns_ /= 2;

            timer.start();
            UpdateVisible();
            update_visibility_ns_ += timer.nsecsElapsed();
            update_visibility_ns_ /= 2;

            timer.start();
            GenerateFrame();
            GetTexts().Process();
            frame_generation_ns_ += timer.nsecsElapsed();
            frame_generation_ns_ /= 2;
        }

        if (fps_timer.elapsed() >= 1000)
        {
            fps_timer.restart();
            cpu_load_ = ((1000.0f / MAX_WAIT_ON_QUEUE) - lps_) / (1000.0f / MAX_WAIT_ON_QUEUE) * 100;
            lps_ = 0;
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

        ++lps_;
        process_in_ = false;
    }
    thread_.exit();
}

const QString ON_LOGIN_MESSAGE =
        "Welcome to Griefly! It is yet another space station remake, so if you are here then you probably already know how to play."
        " Just in case: arrows for movement, left mouse click for hand actions (hit, attack, take), chat for speaking."
        " Use prefix ooc in the chat if you would like to use the ooc channel (it is a global channel).\n";

void Game::WaitForExit()
{
    thread_.wait();
}

std::vector<ObjectInfo>* id_ptr_id_table = nullptr;

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
            qDebug() << "No mapgen param!";
            KvAbort();
            return;
        }

        QString mapgen_name = GetParamsHolder().GetParam<QString>("mapgen_name");
        if (QFileInfo::exists(mapgen_name))
        {
            srand(QTime::currentTime().msecsSinceStartOfDay());

            GetFactory().LoadFromMapGen(GetParamsHolder().GetParam<QString>("mapgen_name"));
            qDebug() << "End load from mapgen atmpsphere";


            GetFactory().CreateImpl(Lobby::T_ITEM_S());

            if (GetParamsHolder().GetParamBool("-unsync_generation"))
            {
                quint32 unsync_generator
                    = GetFactory().CreateImpl(UnsyncGenerator::T_ITEM_S());
                SetUnsyncGenerator(unsync_generator);
            }

            for (auto it = GetFactory().GetIdTable().begin();
                      it != GetFactory().GetIdTable().end();
                      ++it)
            {
                if (it->object && (it->object->RT_ITEM() == SpawnPoint::REAL_TYPE_ITEM))
                {
                    GetLobby().AddSpawnPoint(it->object->GetId());
                }
            }

            quint32 newmob = GetFactory().CreateImpl(LoginMob::T_ITEM_S());

            ChangeMob(newmob);
            GetFactory().SetPlayerId(id, newmob);

            GetMap().GetAtmosphere().LoadGrid();
            GetMap().FillAtmosphere();
            qDebug() << "End fill atmpsphere";

        }
        else
        {
            qDebug() << "Mapgen file does not exist!" << mapgen_name;
            KvAbort();
            return;
        }
    }
    else
    {
        qDebug() << "Begin load map";

        QByteArray map_data = Network2::GetInstance().GetMapData();

        if (map_data.length() == 0)
        {
            qDebug() << "An empty map received";
            KvAbort();
        }
        std::stringstream ss;
        ss.write(map_data.data(), map_data.length());
        ss.seekg(0, std::ios::beg);

        GetFactory().Load(ss, id);
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

            quint32 game_id = GetFactory().GetPlayerId(new_id);

            if (game_id != 0)
            {
                qDebug() << "game_id " << game_id << " already exists";
                continue;
            }

            quint32 newmob = GetFactory().CreateImpl(LoginMob::T_ITEM_S());

            qDebug() << "New client " << newmob;

            GetFactory().SetPlayerId(new_id, newmob);
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
                std::stringstream ss;
                GetFactory().Save(ss);
                qDebug() << "Tellp: " << ss.tellp();
                std::string string = ss.str();
                data = QByteArray(string.c_str(), string.length());
                AddLastMessages(data);
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
                qDebug() << "Tick mismatch! " << tick << " " << MAIN_TICK;
                KvAbort();
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
            GetChat().PostOOCText(login, text);
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
    map_->Represent();
    GetMob()->GenerateInterfaceForFrame();


    // TODO: reset all shifts
    GetRepresentation().SetCameraForFrame(GetMob()->GetX(), GetMob()->GetY());
    GetRepresentation().Swap();
}

void Game::PlayMusic(const QString& name, float volume)
{
    qDebug() << name;
    emit playMusic(name, volume);
}

void Game::AddSound(const QString& name)
{
    GetRepresentation().AddToNewFrame(name);
}

IMapMaster& Game::GetMap()
{
    return *map_;
}

const IMapMaster& Game::GetMap() const
{
    return *map_;
}

IObjectFactory& Game::GetFactory()
{
    return *factory_;
}

IChat& Game::GetChat()
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

void Game::SetUnsyncGenerator(quint32 generator)
{
    unsync_generator_ = generator;
}

IdPtr<UnsyncGenerator> Game::GetUnsyncGenerator()
{
    return unsync_generator_;
}

void Game::ChangeMob(IdPtr<IMob> i)
{
    if (!GetParamsHolder().GetParamBool("-editor") && current_mob_.IsValid())
    {
        current_mob_->DeinitGUI();
    }

    current_mob_ = i;

    if (current_mob_.IsValid())
    {
        if (!GetParamsHolder().GetParamBool("-editor"))
        {
            current_mob_->InitGUI();
        }
    }

    qDebug() << "Current mob change: " << current_mob_.Id();
}

IdPtr<IMob> Game::GetMob()
{
    return current_mob_;
}

void Game::SetMob(quint32 new_mob)
{
    current_mob_ = new_mob;
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
    if (GetUnsyncGenerator().IsValid())
    {
        GetUnsyncGenerator()->PerformUnsync();
    }
}

void Game::AddLastMessages(QByteArray& data)
{
    data.append('\n');
    for (int i = (log_pos_ + 1) % messages_log_.size();
             i != log_pos_;
             i = (i + 1) % messages_log_.size())
    {
        data.append(QByteArray::number(messages_log_[i].type) + " ");
        data.append(messages_log_[i].json + '\n');
    }
}

void Game::AddMessageToMessageLog(Message2 message)
{
    messages_log_[log_pos_] = message;
    log_pos_ = (log_pos_ + 1) % messages_log_.size();
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
         quint32 game_id = GetFactory().GetPlayerId(net_id);
         if (game_id == 0)
         {
             qDebug() << "Game id is 0";
         }
         IdPtr<IMessageReceiver> game_object = game_id;

         if (game_object.IsValid())
         {
             game_object->ProcessMessage(*it);
         }
         else
         {
             qDebug() << "Game object is not valid: " << net_id;
             KvAbort();
         }
    }
    messages_to_process_.clear();
}

void Game::CheckMessagesOrderCorrectness()
{
    if (!messages_to_process_.empty())
    {
        qDebug() << "CheckMessagesOrderCorrectness fail";
        KvAbort();
    }
}
