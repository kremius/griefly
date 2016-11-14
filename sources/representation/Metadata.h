#pragma once

#include <map>
#include <string>
#include <vector>

#include <png.h>

#include <qglobal.h>
#include <QString>

class ImageMetadata
{
public:
    ImageMetadata() { valid_ = false; }
    struct SpriteMetadata
    {
        SpriteMetadata()
        {
            dirs = 1;
            rewind = false;
            frames_data.resize(1);
            first_frame_pos = 0;
            loop = -1;
            hotspot[0] = -1;
            hotspot[1] = -1;
            hotspot[2] = -1;
        }

        struct FrameMetadata
        {
            FrameMetadata()
            {
                delay = 0;
           /*     state_w = 0;
                state_h = 0; TODO */
            }

            quint32 delay;
         /*   quint32 state_w;
            quint32 state_h;*/
        };
        std::vector<FrameMetadata> frames_data;

        std::vector<int> frames_sequence;
        
        quint32 dirs;
        bool rewind;
        int loop;
        int hotspot[3];

        quint32 first_frame_pos;
    };
    bool Valid() const
    {
        return valid_;
    }
    int GetW() const
    {
        return width_;
    }
    int GetH() const
    {
        return height_;
    }
    void Init(const QString& file_name, int width, int height);

    const SpriteMetadata& GetSpriteMetadata(const QString& name);
    bool IsValidState(const QString& name);
private:
    void InitWithoutMetadata();

    bool valid_;

    QString dmi_version_;

    png_uint_32 total_width_;
    png_uint_32 total_height_;

    quint32 width_;
    quint32 height_;

    bool ParseDescription(std::stringstream& desc);
    void MakeSequence();

    std::map<QString, SpriteMetadata> metadata_;
};
