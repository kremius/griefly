#pragma once

#include <unordered_map>
#include <vector>

#include "View2.h"
#include "ViewInfo.h"

#include <QMap>
#include <QMutex>
#include <QKeyEvent>
#include <QElapsedTimer>

const char* const STOP_MUSIC = "^";

class Representation
{
public:
    Representation();

    struct Entity
    {
        Entity();

        ViewInfo view;
        quint32 id;
        int pos_x;
        int pos_y;
        int vlevel;
        int dir;
    };

    struct InterfaceUnit
    {
        InterfaceUnit();
        ViewInfo view;
        QString name;

        int pixel_x;
        int pixel_y;

        int shift;
    };

    struct Sound
    {
        Sound(const QString& name) : name(name) {}
        QString name;
    };

    struct Performance
    {
        QElapsedTimer timer;
        qint64 mutex_ns;
    };

    const Performance& GetPerformance() { return performance_; }
    void ResetPerformance();

    void AddToNewFrame(const InterfaceUnit& unit);
    void AddToNewFrame(const Entity& ent);
    void AddToNewFrame(const Sound& sound);

    void SetCameraForFrame(int pos_x, int pos_y);

    void Swap();
    void Process();
    void Click(int x, int y);

    void HandleKeyboardDown(QKeyEvent* event);
    void HandleKeyboardUp(QKeyEvent* event);
    void ResetKeysState();
private:
    QMap<Qt::Key, bool> keys_state_;

    Performance performance_;

    QElapsedTimer autoplay_timer_;
    bool autoplay_;

    QElapsedTimer message_sending_interval_;

    QMutex mutex_;

    void HandleInput();

    void SynchronizeViews();
    void PerformPixelMovement();
    void Draw();
    void DrawInterface();

    QElapsedTimer pixel_movement_tick_;

    quint32 current_frame_id_;

    bool is_updated_;

    struct FrameData
    {
        std::vector<Entity> entities;
        std::vector<QString> sounds;
        std::vector<InterfaceUnit> units;
        int camera_pos_x;
        int camera_pos_y;
    };

    typedef FrameData DataType;

    DataType* current_frame_;
    DataType* new_frame_;

    DataType first_data_;
    DataType second_data_;

    struct ViewWithFrameId
    {
        View2 view;
        quint32 frame_id;
    };

    std::unordered_map<quint32, ViewWithFrameId> views_;
    std::vector<View2> interface_views_;

    class Camera
    {
    public:
        Camera();

        void SetPos(int new_pos_x, int new_pos_y);

        void PerformPixelMovement();

        int GetFullShiftX();
        int GetFullShiftY();
    private:
        int pos_x;
        int pos_y;

        int pixel_shift_x_;
        int pixel_shift_y_;
    } camera_;
};

Representation &GetRepresentation();
bool IsRepresentationValid();
void SetRepresentation(Representation* g_r);
