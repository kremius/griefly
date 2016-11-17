#include "Representation.h"

#include "core/Constheader.h"
#include "core/Helpers.h"

#include "Sound.h"
#include "net/Network2.h"
#include "net/MagicStrings.h"
#include "Params.h"

#include "Text.h"
#include "SpriteHolder.h"
#include "SoundLoader.h"
#include "ImageLoader.h"

#include "qt/qtopengl.h"

#include <QMutexLocker>
#include <QCoreApplication>

Representation::Representation()
{
    current_frame_ = &first_data_;
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
    SetScreen(new Screen(sizeW, sizeH));
    GetGLWidget()->resize(old_size_w, old_size_h);
    std::cout << "Screen set" << std::endl;
    SetSpriter(new SpriteHolder);

    std::cout << "Begin load resources" << std::endl;
    LoadImages();
    LoadSounds();
}

void Representation::ResetPerformance()
{
    performance_.mutex_ns = 0;
}

void Representation::AddToNewFrame(const Representation::InterfaceUnit &unit)
{
    new_frame_->units.push_back(unit);
}

void Representation::AddToNewFrame(const Representation::Entity& ent)
{
    new_frame_->entities.push_back(ent);
}

void Representation::AddToNewFrame(const Sound &sound)
{
    new_frame_->sounds.push_back(sound.name);
}

void Representation::SetCameraForFrame(int pos_x, int pos_y)
{
    new_frame_->camera_pos_x = pos_x;
    new_frame_->camera_pos_y = pos_y;
}

void Representation::Swap()
{
    QMutexLocker lock(&mutex_);

    std::swap(current_frame_, new_frame_);
    is_updated_ = true;
    new_frame_->entities.clear();
    new_frame_->sounds.clear();
    new_frame_->units.clear();
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

Representation::Entity::Entity()
{
    id = 0;
    pos_x = 0;
    pos_y = 0;
    vlevel = 0;
    dir = D_DOWN;
}

Representation::InterfaceUnit::InterfaceUnit()
{
    pixel_x = 0;
    pixel_y = 0;
    shift = 0;
}

void Representation::Process()
{
    performance_.timer.start();
    QMutexLocker lock(&mutex_);
    performance_.mutex_ns
        = qMax(performance_.mutex_ns, performance_.timer.nsecsElapsed());

    SynchronizeViews();

    HandleInput();
    const int AUTOPLAY_INTERVAL = 10;
    if (autoplay_ && (autoplay_timer_.elapsed() > AUTOPLAY_INTERVAL))
    {
        int loops = autoplay_timer_.elapsed() / AUTOPLAY_INTERVAL;
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

    if (!NODRAW)
    {
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
    else
    {
        const int SLEEP_MS = 50;
        mutex_.unlock();
        QThread::msleep(SLEEP_MS);
        mutex_.lock();
    }
}

void Representation::Click(int x, int y)
{
    GetScreen().NormalizePixels(&x, &y);
    int s_x = x - camera_.GetFullShiftX();
    int s_y = y - camera_.GetFullShiftY();
    //qDebug() << "S_X: " << s_x << ", S_Y: " <<  s_y;
    //qDebug() << "X: " << x << ", Y: " <<  y;

    auto& units = current_frame_->units;


    for (unsigned int i = 0; i < units.size(); ++i)
    {
        int bdir = units[i].shift;
        if (!interface_views_[i].IsTransp(x, y, bdir))
        {
            Network2::GetInstance().SendOrdinaryMessage(units[i].name);
            return;
        }
    }

    auto& ents = current_frame_->entities;

    int id_to_send = -1;

    for (auto it = ents.rbegin(); it != ents.rend(); ++it)
    {
        if (it->vlevel >= MAX_LEVEL)
        {
            int bdir = helpers::dir_to_byond(it->dir);
            if (!views_[it->id].view.IsTransp(s_x, s_y, bdir))
            {
                //qDebug() << "Clicked " << it->id;
                id_to_send = it->id;
                break;
            }
        }
    }

    for (int vlevel = MAX_LEVEL - 1; vlevel >= 0 && (id_to_send == -1); --vlevel)
    {
        for (auto it = ents.rbegin(); it != ents.rend(); ++it)
        {
            if (it->vlevel != vlevel)
            {
                continue;
            }
            int bdir = helpers::dir_to_byond(it->dir);
            if (!views_[it->id].view.IsTransp(s_x, s_y, bdir))
            {
                //qDebug() << "Clicked " << it->id;
                id_to_send = it->id;
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
            message = Click::LEFT_R;
        }

        Message2 msg = Network2::MakeClickMessage(id_to_send, message);
        Network2::GetInstance().SendMsg(msg);
    }
}

void Representation::SynchronizeViews()
{
    if (is_updated_)
    {
        camera_.SetPos(current_frame_->camera_pos_x, current_frame_->camera_pos_y);

        for (auto it = current_frame_->entities.begin(); it != current_frame_->entities.end(); ++it)
        {
            auto& view_with_frame_id = views_[it->id];
            view_with_frame_id.view.LoadViewInfo(it->view);
            if (view_with_frame_id.frame_id != current_frame_id_)
            {
                view_with_frame_id.view.SetX(it->pos_x * 32);
                view_with_frame_id.view.SetY(it->pos_y * 32);
            }
            view_with_frame_id.frame_id = current_frame_id_ + 1;
        } 

        interface_views_.resize(current_frame_->units.size());
        for (int i = 0; i < static_cast<int>(interface_views_.size()); ++i)
        {
            interface_views_[i].LoadViewInfo(current_frame_->units[i].view);
            interface_views_[i].SetX(current_frame_->units[i].pixel_x);
            interface_views_[i].SetY(current_frame_->units[i].pixel_y);
        }

        for (auto it = current_frame_->sounds.begin(); it != current_frame_->sounds.end(); ++it)
        {
            GetSoundPlayer().PlaySound(*it);
        }

        ++current_frame_id_;
        is_updated_ = false;
    }
}

int get_pixel_speed_for_distance(int distance)
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

    distance = std::abs(distance);

    return sign * (((distance - 1) / 8) + 1);
}

void Representation::PerformPixelMovement()
{
    for (auto it = current_frame_->entities.begin(); it != current_frame_->entities.end(); ++it)
    {
        int pixel_x = it->pos_x * 32;
        int pixel_y = it->pos_y * 32;

        auto& view_with_frame_id = views_[it->id];
        int old_x = view_with_frame_id.view.GetX();
        int old_y = view_with_frame_id.view.GetY();
        if (old_x != pixel_x)
        {
            int diff_x = pixel_x - old_x;
            diff_x = get_pixel_speed_for_distance(diff_x);
            view_with_frame_id.view.SetX(old_x + diff_x);
        }
        if (old_y != pixel_y)
        {
            int diff_y = pixel_y - old_y;
            diff_y = get_pixel_speed_for_distance(diff_y);
            view_with_frame_id.view.SetY(old_y + diff_y);
        }
    }
}

void Representation::Draw()
{
    for (int vlevel = 0; vlevel < MAX_LEVEL; ++vlevel)
    {
        for (auto it = current_frame_->entities.begin(); it != current_frame_->entities.end(); ++it)
        {
            if (it->vlevel == vlevel)
            {
                views_[it->id].view.Draw(
                    camera_.GetFullShiftX(),
                    camera_.GetFullShiftY(),
                    helpers::dir_to_byond(it->dir));
            }
        }
    }
    for (auto it = current_frame_->entities.begin(); it != current_frame_->entities.end(); ++it)
    {
        if (it->vlevel >= MAX_LEVEL)
        {
            views_[it->id].view.Draw(
                camera_.GetFullShiftX(),
                camera_.GetFullShiftY(),
                helpers::dir_to_byond(it->dir));
        }
    }
}

void Representation::DrawInterface()
{
    for (int i = 0; i < static_cast<int>(interface_views_.size()); ++i)
    {
        interface_views_[i].Draw(0, 0, current_frame_->units[i].shift);
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
        int diff_x = get_pixel_speed_for_distance(pixel_shift_x_);
        pixel_shift_x_ -= diff_x;
    }
    if (pixel_shift_y_ != 0)
    {
        int diff_y = get_pixel_speed_for_distance(pixel_shift_y_);
        pixel_shift_y_ -= diff_y;
    }
}

int Representation::Camera::GetFullShiftX()
{
    return -1 * (pos_x * 32 + pixel_shift_x_) + (sizeW / 2) - 16;
}

int Representation::Camera::GetFullShiftY()
{
    return -1 * (pos_y * 32 + pixel_shift_y_) + (sizeH / 2) - 16;
}

Representation* g_r = nullptr;
Representation& GetRepresentation()
{
    return *g_r;
}

void SetRepresentation(Representation* new_g_r)
{
    g_r = new_g_r;
}


bool IsRepresentationValid()
{
    return g_r != nullptr;
}
