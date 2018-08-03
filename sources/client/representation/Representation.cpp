#include "Representation.h"

#include <limits>

#include "Sound.h"
#include "net/Network2.h"
#include "net/MagicStrings.h"

#include <NetworkMessages.h>

#include "Params.h"

#include "SpriteHolder.h"
#include "ImageLoader.h"

#include "qt/qtopengl.h"

#include <QMutexLocker>
#include <QCoreApplication>

namespace
{

const int MAX_LEVEL = 20;

}

using Music = kv::FrameData::Music;
using ChatMessage = kv::FrameData::ChatMessage;
using TextEntry = kv::FrameData::TextEntry;

Representation::Representation(QObject* parent)
    : QObject(parent)
{
    old_frame_ = &first_data_;
    new_frame_ = &second_data_;
    is_updated_ = false;
    current_frame_id_ = 1;
    pixel_movement_tick_.start();

    ResetPerformance();

    autoplay_ = false;
    if (GetParamsHolder().GetParamBool("-auto"))
    {
        autoplay_ = true;
        autoplay_timer_.start();
    }

    message_sending_interval_.start();

    int old_size_w = GetGLWidget()->width();
    int old_size_h = GetGLWidget()->height();
    SetScreen(new Screen(AREA_SIZE_W, AREA_SIZE_H));
    GetGLWidget()->resize(old_size_w, old_size_h);
    qDebug() << "Screen set";
    SetSpriter(new SpriteHolder);

    qDebug() << "Begin load resources";
    LoadImages();
}

void Representation::ResetPerformance()
{
    performance_.mutex_ns = 0;
}

void Representation::Swap()
{
    QMutexLocker lock(&mutex_);

    std::swap(old_frame_, new_frame_);
    is_updated_ = true;
    new_frame_->entities.clear();
    new_frame_->sounds.clear();
    new_frame_->units.clear();
    new_frame_->messages.clear();
    new_frame_->texts.clear();
}

const int SUPPORTED_KEYS_SIZE = 8;
const Qt::Key SUPPORTED_KEYS[SUPPORTED_KEYS_SIZE]
    = { Qt::Key_Up, Qt::Key_Down, Qt::Key_Left, Qt::Key_Right,
        Qt::Key_Control, Qt::Key_Alt, Qt::Key_Shift, Qt::Key_R };

const int NUMPAD_KEYS_SIZE = 4;
const Qt::Key NUMPAD_KEYS[NUMPAD_KEYS_SIZE]
    = { Qt::Key_8, Qt::Key_2, Qt::Key_4, Qt::Key_6 };

const int TEXT_NUMPAD_SIZE = 4;
const char* const TEXT_NUMPAD[TEXT_NUMPAD_SIZE]
    = { Input::MOVE_UP, Input::MOVE_DOWN, Input::MOVE_LEFT, Input::MOVE_RIGHT };

const int TEXT_KEYS_SIZE = 4;
const char* const TEXT_KEYS[TEXT_KEYS_SIZE]
    = { Input::MOVE_UP, Input::MOVE_DOWN, Input::MOVE_LEFT, Input::MOVE_RIGHT };

void Representation::HandleKeyboardDown(QKeyEvent* event)
{
    // TODO: language layouts (for example, russian) does not
    // work properly

    if (autoplay_ == false)
    {
        const int MESSAGE_INTERVAL = 33;
        if (message_sending_interval_.elapsed() < MESSAGE_INTERVAL)
        {
            return;
        }
        message_sending_interval_.restart();
    }

    if (!event)
    {
        int val = rand();
        const char* const text = TEXT_KEYS[val % TEXT_KEYS_SIZE];
        Network2::GetInstance().SendOrdinaryMessage(text);
        return;
    }

    QString text;

    if (event->modifiers() == Qt::KeypadModifier)
    {
        for (int i = 0; i < NUMPAD_KEYS_SIZE; ++i)
        {
            if (event->key() != NUMPAD_KEYS[i])
            {
                continue;
            }
            if (i < TEXT_NUMPAD_SIZE)
            {
                text = TEXT_NUMPAD[i];
            }
        }
    }

    for (int i = 0; i < SUPPORTED_KEYS_SIZE; ++i)
    {
        if (event->key() != SUPPORTED_KEYS[i])
        {
            continue;
        }

        keys_state_[SUPPORTED_KEYS[i]] = true;

        if (i < TEXT_KEYS_SIZE)
        {
            text = TEXT_KEYS[i];
        }
    }

    if (text.isEmpty())
    {
        return;
    }

    Network2::GetInstance().SendOrdinaryMessage(text);
}

void Representation::HandleKeyboardUp(QKeyEvent* event)
{
    for (int i = 0; i < SUPPORTED_KEYS_SIZE; ++i)
    {
        if (event->key() == SUPPORTED_KEYS[i])
        {
            keys_state_[SUPPORTED_KEYS[i]] = false;
        }
    }
}

void Representation::ResetKeysState()
{
    keys_state_.clear();
}

void Representation::HandleInput()
{
    QCoreApplication::processEvents(QEventLoop::AllEvents, 40);
}

void Representation::Process()
{
    SynchronizeViews();

    const int AUTOPLAY_INTERVAL = 10;
    if (autoplay_ && (autoplay_timer_.elapsed() > AUTOPLAY_INTERVAL))
    {
        int loops = static_cast<int>(autoplay_timer_.elapsed()) / AUTOPLAY_INTERVAL;
        for (int i = 0; i < loops; ++i)
        {
            int w = GetGLWidget()->width();
            int h = GetGLWidget()->height();

            if (rand() % 1 == 0)
            {
                Click(rand() % w, rand() % h);
            }
            if (rand() % 1 == 0)
            {
                HandleKeyboardDown(nullptr);
            }
        }
        autoplay_timer_.restart();
    }

    MakeCurrentGLContext();
    GetScreen().Clear();

    Draw();
    DrawInterface();

    const int PIXEL_MOVEMENT_SPEED = 16;
    if (pixel_movement_tick_.elapsed() > PIXEL_MOVEMENT_SPEED)
    {
        PerformPixelMovement();
        camera_.PerformPixelMovement();
        pixel_movement_tick_.restart();
    }
}

void Representation::Click(int x, int y)
{
    GetScreen().NormalizePixels(&x, &y);
    const int shift_x = x - camera_.GetFullShiftX();
    const int shift_y = y - camera_.GetFullShiftY();

    auto& units = current_frame_.units;


    for (int i = 0; i < units.size(); ++i)
    {
        const int bdir = units[i].shift;
        if (!interface_views_[i].IsTransp(x, y, bdir))
        {
            Network2::GetInstance().SendOrdinaryMessage(units[i].name);
            return;
        }
    }

    auto& ents = current_frame_.entities;

    int id_to_send = -1;

    for (auto it = ents.rbegin(); it != ents.rend(); ++it)
    {
        if (it->click_id == 0)
        {
            continue;
        }
        if (it->vlevel >= MAX_LEVEL)
        {
            int bdir = kv::helpers::DirToByond(it->dir);
            if (!views_[it->id].view.IsTransp(shift_x, shift_y, bdir))
            {
                id_to_send = static_cast<qint32>(it->click_id);
                break;
            }
        }
    }

    for (int vlevel = MAX_LEVEL - 1; vlevel >= 0 && (id_to_send == -1); --vlevel)
    {
        for (auto it = ents.rbegin(); it != ents.rend(); ++it)
        {
            if (it->click_id == 0)
            {
                continue;
            }
            if (it->vlevel != vlevel)
            {
                continue;
            }
            int bdir = kv::helpers::DirToByond(it->dir);
            if (!views_[it->id].view.IsTransp(shift_x, shift_y, bdir))
            {
                id_to_send = static_cast<qint32>(it->click_id);
                break;
            }
        }
    }

    if (id_to_send != -1)
    {
        // Alt key is disabled on purpose
        // Alt key + click is some default action on Ubuntu,
        // so it is bad to use it
        QString message = Click::LEFT;
        if (keys_state_[Qt::Key_Control])
        {
            message = Click::LEFT_CONTROL;
        }
        else if (keys_state_[Qt::Key_Shift])
        {
            message = Click::LEFT_SHIFT;
        }
        else if (keys_state_[Qt::Key_R])
        {
            qDebug() << "R key";
            message = Click::LEFT_R;
        }

        kv::Message msg = Network2::MakeClickMessage(id_to_send, message);
        Network2::GetInstance().Send(msg);
    }
}

void Representation::SynchronizeViews()
{
    Music old_music = current_frame_.music;
    {
        performance_.timer.start();
        QMutexLocker lock(&mutex_);
        if (is_updated_)
        {
            current_frame_ = *old_frame_;
        }
        performance_.mutex_ns
            = qMax(performance_.mutex_ns, performance_.timer.nsecsElapsed());
        if (!is_updated_)
        {
            return;
        }
    }

    camera_.SetPos(current_frame_.camera_pos_x, current_frame_.camera_pos_y);

    for (auto it = current_frame_.entities.begin(); it != current_frame_.entities.end(); ++it)
    {
        auto& view_with_frame_id = views_[it->id];
        view_with_frame_id.view.LoadViewInfo(it->view);
        if (view_with_frame_id.frame_id != current_frame_id_)
        {
            view_with_frame_id.view.SetX(it->pos_x * 32);
            view_with_frame_id.view.SetY(it->pos_y * 32);
            view_with_frame_id.view.RandomizeImageStateIfLooped();
        }
        view_with_frame_id.frame_id = current_frame_id_ + 1;
    }

    interface_views_.resize(current_frame_.units.size());
    for (int i = 0; i < static_cast<int>(interface_views_.size()); ++i)
    {
        interface_views_[i].LoadViewInfo(current_frame_.units[i].view);
        interface_views_[i].SetX(current_frame_.units[i].pixel_x);
        interface_views_[i].SetY(current_frame_.units[i].pixel_y);
    }

    for (auto it = current_frame_.sounds.begin(); it != current_frame_.sounds.end(); ++it)
    {
        GetSoundPlayer().PlaySound(it->name);
    }

    const Music music = current_frame_.music;
    if (old_music.name != music.name)
    {
        if (music.name != "")
        {
            GetSoundPlayer().PlayMusic(music.name, music.volume);
        }
        else
        {
            GetSoundPlayer().StopMusic();
        }
    }

    for (const ChatMessage& message : qAsConst(current_frame_.messages))
    {
        emit chatMessage(message.html);
    }
    emit clearSystemTexts();
    for (const TextEntry& text : qAsConst(current_frame_.texts))
    {
        emit systemText(text.tab, text.text);
    }
    emit removeEmptyTabs();

    ++current_frame_id_;
    is_updated_ = false;
}

namespace
{

int GetPixelSpeedForDistance(int distance)
{
    int sign = 0;
    if (distance > 0)
    {
        sign = 1;
    }
    if (distance < 0)
    {
        sign = -1;
    }
    if (sign == 0)
    {
        return 0;
    }

    distance = abs(distance);

    return sign * (((distance - 1) / 8) + 1);
}

}

void Representation::PerformPixelMovement()
{
    for (auto it = current_frame_.entities.begin(); it != current_frame_.entities.end(); ++it)
    {
        int pixel_x = it->pos_x * 32;
        int pixel_y = it->pos_y * 32;

        auto& view_with_frame_id = views_[it->id];
        int old_x = view_with_frame_id.view.GetX();
        int old_y = view_with_frame_id.view.GetY();
        if (old_x != pixel_x)
        {
            int diff_x = pixel_x - old_x;
            diff_x = GetPixelSpeedForDistance(diff_x);
            view_with_frame_id.view.SetX(old_x + diff_x);
        }
        if (old_y != pixel_y)
        {
            int diff_y = pixel_y - old_y;
            diff_y = GetPixelSpeedForDistance(diff_y);
            view_with_frame_id.view.SetY(old_y + diff_y);
        }
    }
}

void Representation::Draw()
{
    for (int vlevel = 0; vlevel < MAX_LEVEL; ++vlevel)
    {
        for (auto it = current_frame_.entities.begin(); it != current_frame_.entities.end(); ++it)
        {
            if (it->vlevel == vlevel)
            {
                const int bdir = kv::helpers::DirToByond(it->dir);
                views_[it->id].view.Draw(
                    camera_.GetFullShiftX(),
                    camera_.GetFullShiftY(),
                    static_cast<quint32>(bdir));
            }
        }
    }
    for (auto it = current_frame_.entities.begin(); it != current_frame_.entities.end(); ++it)
    {
        if (it->vlevel >= MAX_LEVEL)
        {
            const int bdir = kv::helpers::DirToByond(it->dir);
            views_[it->id].view.Draw(
                camera_.GetFullShiftX(),
                camera_.GetFullShiftY(),
                static_cast<quint32>(bdir));
        }
    }
}

void Representation::DrawInterface()
{
    for (int i = 0; i < interface_views_.size(); ++i)
    {
        interface_views_[i].Draw(
            0, 0, static_cast<quint32>(current_frame_.units[i].shift));
    }
}

Representation::Camera::Camera()
{
    pos_x = 0;
    pos_y = 0;
    pixel_shift_x_ = 0;
    pixel_shift_y_ = 0;
}

void Representation::Camera::SetPos(int new_pos_x, int new_pos_y)
{
    pixel_shift_x_ += (pos_x - new_pos_x) * 32;
    pixel_shift_y_ += (pos_y - new_pos_y) * 32;

    pos_x = new_pos_x;
    pos_y = new_pos_y;
}

void Representation::Camera::PerformPixelMovement()
{
    if (pixel_shift_x_ != 0)
    {
        const int diff_x = GetPixelSpeedForDistance(pixel_shift_x_);
        pixel_shift_x_ -= diff_x;
    }
    if (pixel_shift_y_ != 0)
    {
        const int diff_y = GetPixelSpeedForDistance(pixel_shift_y_);
        pixel_shift_y_ -= diff_y;
    }
}

int Representation::Camera::GetFullShiftX()
{
    return -1 * (pos_x * 32 + pixel_shift_x_) + (AREA_SIZE_W / 2) - 16;
}

int Representation::Camera::GetFullShiftY()
{
    return -1 * (pos_y * 32 + pixel_shift_y_) + (AREA_SIZE_H / 2) - 16;
}
