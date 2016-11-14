#pragma once

#include <vector>

#include "ViewInfo.h"
#include "Metadata.h"
#include "GLSprite.h"

#include <QElapsedTimer>

class View2
{
public:
    class FramesetState
    {
    public:
        FramesetState();

        void LoadFramesetInfo(const ViewInfo::FramesetInfo& frameset_info);

        const GLSprite* GetSprite() const { return sprite_; }
        const ImageMetadata::SpriteMetadata* GetMetadata() const { return metadata_; }

        bool IsTransp(int x, int y, int shift, int angle);
        void Draw(quint32 shift, int x, int y, int angle = 0);
    private:
        void Reset();

        const GLSprite* sprite_;
        const ImageMetadata::SpriteMetadata* metadata_;
        int image_state_;
        QElapsedTimer last_frame_tick_;
    };
    View2();

    void SetX(int new_pixel_x)
    {
        pixel_x_ = new_pixel_x;
    }
    void SetY(int new_pixel_y)
    {
        pixel_y_ = new_pixel_y;
    }
    int GetX() const { return pixel_x_; }
    int GetY() const { return pixel_y_; }

    bool IsTransp(int x, int y, quint32 shift);
    void Draw(int x_shift, int y_shift, quint32 shift);

    void LoadViewInfo(const ViewInfo& view_info);

    const FramesetState& GetBaseFrameset() const { return base_frameset_; }
private:
    FramesetState base_frameset_;
    std::vector<FramesetState> underlays_;
    std::vector<FramesetState> overlays_;

    int pixel_x_;
    int pixel_y_;

    ViewInfo info_;
};
