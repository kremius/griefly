#pragma once

#include <list>

#include "objects/Mob.h"
#include "objects/test/UnsyncGenerator.h"
#include "objects/GlobalObjectsHolder.h"

#include "SyncRandom.h"
#include "Names.h"
#include "WorldLoaderSaver.h"
#include "ChatFrameInfo.h"

#include <QApplication>
#include <QKeyEvent>
#include <QString>
#include <QObject>
#include <QThread>

#include "Interfaces.h"

class TextPainter;

class Game : public QObject, public GameInterface
{
    Q_OBJECT
public:
    void WaitForExit();
    void InitWorld(int id, QString map_name);

    Game();
    ~Game();

    virtual void MakeTiles(int size_x, int size_y, int size_z) override;

    virtual void PlayMusic(const QString& name, int volume, quint32 mob) override;
    virtual void AddSound(const QString& name, kv::Position position) override;

    virtual AtmosInterface& GetAtmosphere() override;
    virtual MapInterface& GetMap() override;
    virtual const MapInterface& GetMap() const override;
    virtual ObjectFactoryInterface& GetFactory() override;
    virtual ChatInterface& GetChat() override;
    virtual TextPainter& GetTexts() override;
    virtual SyncRandom& GetRandom() override;
    virtual Names& GetNames() override;
    virtual kv::ChatFrameInfo& GetChatFrameInfo() override;

    virtual IdPtr<kv::Mob> GetMob() override;
    virtual void SetMob(quint32 new_mob) override;

    virtual IdPtr<kv::GlobalObjectsHolder> GetGlobals();
    virtual void SetGlobals(quint32 globals);

    virtual void SetPlayerId(quint32 net_id, quint32 real_id) override;
    virtual quint32 GetPlayerId(quint32 net_id) const override;
    virtual quint32 GetNetId(quint32 real_id) const override;
public slots:
    void process();
    void endProcess();
    void generateUnsync();
signals:    
    void sendMap(QString url, QByteArray data);
    void addSystemText(QString key, QString text);
    void insertHtmlIntoChat(QString html);
private:
    void GenerateFrame();
    void AppendSoundsToFrame(const VisiblePoints& points);
    void AppendChatMessages();

    void ProcessInputMessages();
    void InitGlobalObjects();
    void Process();
    void ProcessHearers();

    void AddLastMessages(QByteArray* data);
    void AddMessageToMessageLog(Message2 message);

    void AddBuildInfo(QByteArray* data);

    void PostOoc(const QString& who, const QString& text);

    FastSerializer serializer_;

    std::vector<Message2> messages_log_;
    int log_pos_;

    bool is_end_process_;

    float cpu_load_;

    std::vector<float> cpu_loads_;
    int cpu_loads_id_;

    QString last_touch_;

    std::vector<Message2> messages_to_process_;
    void ProcessBroadcastedMessages();
    void CheckMessagesOrderCorrectness();
    //bool hash_

    QString ping_id_;
    QElapsedTimer ping_send_time_;
    int current_ping_;
    bool ping_send_is_requested_;
    bool process_in_;
    QString adrs_;
    quint32 creator_;
    bool auto_player_;

    int current_connections_;

    // Perfomance
    qint64 process_messages_ns_;
    qint64 foreach_process_ns_;
    qint64 force_process_ns_;
    qint64 atmos_process_ns_;
    qint64 deletion_process_ns_;
    qint64 update_visibility_ns_;
    qint64 frame_generation_ns_;

    QThread thread_;

    AtmosInterface* atmos_;
    ObjectFactoryInterface* factory_;
    ChatInterface* chat_;
    TextPainter* texts_;

    SyncRandom* sync_random_;
    Names* names_;
    WorldLoaderSaver* world_loader_saver_;

    kv::ChatFrameInfo chat_frame_info_;

    IdPtr<kv::GlobalObjectsHolder> global_objects_;

    IdPtr<kv::Mob> current_mob_;
    QVector<QPair<kv::Position, QString>> sounds_for_frame_;
    VisiblePoints points_;
};
