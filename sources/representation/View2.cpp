#include "View2.h"

#include "SpriteHolder.h"
#include <QColor>
#include <cmath>

View2::FramesetState::FramesetState()
{
    Reset();
    last_frame_tick_.start();
}

void View2::FramesetState::LoadFramesetInfo(const ViewInfo::FramesetInfo& frameset_info)
{
    Reset();

    if (!IsSpriterValid())
    {
        return;
    }
    sprite_ = GetSpriter().GetSprite(frameset_info.GetSprite());
    if (sprite_ == nullptr)
    {
        return;
    }
    if (sprite_->Fail())
    {
        return;
    }
    if (!sprite_->GetSDLSprite()->metadata.IsValidState(frameset_info.GetState()))
    {
        return;
    }
    metadata_ = &sprite_->GetSDLSprite()->metadata.GetSpriteMetadata(frameset_info.GetState());
}

bool View2::FramesetState::IsTransp(int x, int y, int shift, int angle)
{
    if (!GetMetadata())
    {
        return true;
    }

    if (shift >= (int)GetMetadata()->dirs)
    {
        shift = 0;
    }

    float true_angle = (angle * 3.14f) / 180;

    float sin_a = sin(static_cast<float>(1 * true_angle));
    float cos_a = cos(static_cast<float>(1 * true_angle));

    x -= GetSprite()->W() / 2;
    y -= GetSprite()->H() / 2;

    int new_x = static_cast<int>(     cos_a * x + sin_a * y);
    int new_y = static_cast<int>(-1 * sin_a * x + cos_a * y);

    x = new_x + GetSprite()->W() / 2;
    y = new_y + GetSprite()->H() / 2;

    const Sprite* loc = GetSprite()->GetSDLSprite();
    if (y >= loc->h || x >= loc->w || x < 0 || y < 0)
    {
        return true;
    }

    //qDebug() << "Frameset: " << x << ", " << y;

    int current_frame = GetMetadata()->frames_sequence[image_state_];
    int current_frame_pos = GetMetadata()->first_frame_pos + current_frame * GetMetadata()->dirs + shift;

    int image_state_h_ = current_frame_pos / GetSprite()->FrameW();
    int image_state_w_ = current_frame_pos % GetSprite()->FrameW();

    QImage image = loc->frames[image_state_w_ * loc->frames_h_ + image_state_h_];

    QRgb pixel = image.pixel(x, y);
    return qAlpha(pixel) < 1;
}

const int ANIMATION_MUL = 100;

void View2::FramesetState::Draw(quint32 shift, int x, int y, int angle)
{
    if (!GetSprite() || GetSprite()->Fail() || !GetMetadata())
    {
        return;
    }

    if (shift >= GetMetadata()->dirs)
    {
        shift = 0;
    }

    int current_frame = GetMetadata()->frames_sequence[image_state_];
    int current_frame_pos = GetMetadata()->first_frame_pos + current_frame * GetMetadata()->dirs + shift;

    int image_state_h_ = current_frame_pos / GetSprite()->FrameW();
    int image_state_w_ = current_frame_pos % GetSprite()->FrameW();

    GetScreen().Draw(GetSprite(),
                      x, y,
                      image_state_w_, image_state_h_,
                      static_cast<float>(angle));

    if (GetMetadata()->frames_sequence.size() == 1)
    {
        return;
    }
    if (GetMetadata()->frames_sequence
            [(image_state_ + 1) % GetMetadata()->frames_sequence.size()]
                == -1)
    {
        return;
    }

    int time_diff = last_frame_tick_.elapsed();

    int next_state = image_state_;
    while (true)
    {
        // TODO: lags when time_diff very big
        int frame = GetMetadata()->frames_sequence[next_state];
        time_diff -= GetMetadata()->frames_data[frame].delay * ANIMATION_MUL;
        if (time_diff <= 0)
        {
            if (image_state_ == next_state)
            {
                break;
            }
            image_state_ = next_state;
            last_frame_tick_.restart();
            break;
        }
        next_state = (next_state + 1) % GetMetadata()->frames_sequence.size();
    }
}

void View2::FramesetState::Reset()
{
    sprite_ = nullptr;
    metadata_ = nullptr;
    image_state_ = 0;
    last_frame_tick_.restart();
}

View2::View2()
{
    pixel_x_ = 0;
    pixel_y_ = 0;
}

bool View2::IsTransp(int x, int y, quint32 shift)
{
    //qDebug() << GetX() << "," << GetY();
    for (int i = overlays_.size() - 1; i >= 0; --i)
    {
        int sum_angle = info_.GetAngle() + info_.GetOverlays()[i].GetAngle();
        if (!overlays_[i].IsTransp(x - GetX(), y - GetY(), shift, sum_angle))
        {
            return false;
        }
    }
    {
        int sum_angle = info_.GetAngle() + info_.GetBaseFrameset().GetAngle();
        if (!base_frameset_.IsTransp(x - GetX(), y - GetY(), shift, sum_angle))
        {
            return false;
        }
    }
    for (int i = 0; i < static_cast<int>(underlays_.size()); ++i)
    {
        int sum_angle = info_.GetAngle() + info_.GetUnderlays()[i].GetAngle();
        if (!underlays_[i].IsTransp(x - GetX(), y - GetY(), shift, sum_angle))
        {
            return false;
        }
    }
    return true;
}

void View2::Draw(int x_shift, int y_shift, quint32 shift)
{
    for (int i = underlays_.size() - 1; i >= 0; --i)
    {
        const auto& underlay = info_.GetUnderlays()[i];
        int sum_angle = info_.GetAngle() + underlay.GetAngle();
        int sum_x = GetX() + x_shift + underlay.GetShiftX();
        int sum_y = GetY() + y_shift + underlay.GetShiftY();
        underlays_[i].Draw(shift, sum_x, sum_y, sum_angle);
    }
    {
        int sum_angle = info_.GetAngle() + info_.GetBaseFrameset().GetAngle();
        base_frameset_.Draw(shift, GetX() + x_shift, GetY() + y_shift, sum_angle);
    }
    for (int i = 0; i < static_cast<int>(overlays_.size()); ++i)
    {
        const auto& overlay = info_.GetOverlays()[i];
        int sum_angle = info_.GetAngle() + overlay.GetAngle();
        int sum_x = GetX() + x_shift + overlay.GetShiftX();
        int sum_y = GetY() + y_shift + overlay.GetShiftY();
        overlays_[i].Draw(shift, sum_x, sum_y, sum_angle);
    }
}

void View2::LoadViewInfo(const ViewInfo& view_info)
{
    if (ViewInfo::IsSameFramesets(view_info, info_))
    {
        return;
    }

    //qDebug() << "Not same framesets "
    //         << QString::fromStdString(view_info.GetBaseFrameset().GetSprite());

    if (!ViewInfo::FramesetInfo::IsSameSprites(
            view_info.GetBaseFrameset(),
            info_.GetBaseFrameset()))
    {
        base_frameset_.LoadFramesetInfo(view_info.GetBaseFrameset());
    }

    {
        const auto& new_overlays = view_info.GetOverlays();
        overlays_.resize(new_overlays.size());
        int counter = 0;
        int intermediate_size = std::min(info_.GetOverlays().size(), new_overlays.size());
        for (; counter < intermediate_size; ++counter)
        {
            if (!ViewInfo::FramesetInfo::IsSameSprites(
                    new_overlays[counter],
                    info_.GetOverlays()[counter]))
            {
                overlays_[counter].LoadFramesetInfo(new_overlays[counter]);
            }
        }
        for (; counter < (int)new_overlays.size(); ++counter)
        {
            overlays_[counter].LoadFramesetInfo(new_overlays[counter]);
        }
    }

    {
        const auto& new_underlays = view_info.GetUnderlays();
        underlays_.resize(new_underlays.size());
        int counter = 0;
        int intermediate_size = std::min(info_.GetUnderlays().size(), new_underlays.size());
        for (; counter < intermediate_size; ++counter)
        {
            if (!ViewInfo::FramesetInfo::IsSameSprites(
                    new_underlays[counter],
                    info_.GetUnderlays()[counter]))
            {
                underlays_[counter].LoadFramesetInfo(new_underlays[counter]);
            }
        }
        for (; counter < (int)new_underlays.size(); ++counter)
        {
            underlays_[counter].LoadFramesetInfo(new_underlays[counter]);
        }
    }

    info_ = view_info;
}
