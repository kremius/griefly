#include "Metadata.h"

#include <fstream>
#include <sstream>
#include <string>

#include <QDebug>

#include <png.h>

#include "KVAbort.h"

const ImageMetadata::SpriteMetadata& 
    ImageMetadata::GetSpriteMetadata(const QString& name)
{
    return metadata_[name];
}

bool ImageMetadata::IsValidState(const QString& name)
{
    if (!Valid())
    {
        return false;
    }
    return metadata_.find(name) != metadata_.end();
}

const int PNGSIGSIZE = 8;

void UserReadData(png_structp png_ptr, png_bytep data, png_size_t length) {
    png_voidp io_ptr = png_get_io_ptr(png_ptr);
    std::istream* stream = static_cast<std::istream*>(io_ptr);
    stream->read(reinterpret_cast<char*>(data), length);
}

void ImageMetadata::Init(const QString& name, int width, int height)
{   
    qDebug() << "Begin to init metadata for " << name;

    width_ = width;
    height_ = height;

    std::ifstream source;
    source.open(name.toStdString(), std::fstream::binary);
    if (source.fail()) 
    {
        qDebug() << "Metadata error: Fail to open file";
        return;
    }

    png_byte pngsig[PNGSIGSIZE];
    source.read(reinterpret_cast<char*>(pngsig), PNGSIGSIZE);

    if (source.fail()) 
    {
        qDebug() << "Metadata error: Fail to read png signature";
        KvAbort();
    }

    int is_png = png_sig_cmp(pngsig, 0, PNGSIGSIZE);

    if (is_png)
    {
        qDebug() << "Metadata error: Data is not valid PNG-data: " << is_png;
        InitWithoutMetadata();
        return;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
        qDebug() << "Metadata error: Couldn't initialize png read struct";
        return;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        qDebug() << "Metadata error: Couldn't initialize png info struct";
        png_destroy_read_struct(&png_ptr, static_cast<png_infopp>(0), static_cast<png_infopp>(0));
        return;
    }

    png_set_read_fn(png_ptr, static_cast<png_voidp>(&source), &UserReadData);

    png_set_sig_bytes(png_ptr, PNGSIGSIZE);
    png_read_info(png_ptr, info_ptr);

    png_get_IHDR(png_ptr, info_ptr, &total_width_, &total_height_, 0, 0, 0, 0, 0);


    png_textp text_ptr;
    int num_text;

    if (png_get_text(png_ptr, info_ptr, &text_ptr, &num_text) > 0)
    {
        for (int i = 0; i < num_text; ++i)
        {
            QString string_key = QString(text_ptr[i].key);
            if (string_key == "Description")
            {
                std::stringstream string_text;
                string_text << std::string(text_ptr[i].text);
                ParseDescription(string_text);
                break;
            }
        }
    }

    png_destroy_read_struct(&png_ptr, &info_ptr, static_cast<png_infopp>(0));
    source.close();
    qDebug() << "End load metadata for " << name;

    if (!Valid())
    {
        InitWithoutMetadata();
    }
}

void ImageMetadata::InitWithoutMetadata()
{
    qDebug() << "Fail metadata load, try without it";
    metadata_[""].first_frame_pos = 0;

    metadata_[""].frames_sequence.push_back(0);
    valid_ = true;
}

bool ImageMetadata::ParseDescription(std::stringstream& desc)
{
    std::string loc;
    desc >> loc;
    if (loc != "#")
    {
        qDebug() << "Fail to read '#' from .dmi file";
        return false;
    }
    loc.clear();
    desc >> loc;
    if (loc != "BEGIN")
    {
        qDebug() << "Fail to read 'BEGIN' from .dmi file";
        return false;
    }
    loc.clear();
    desc >> loc;
    if (loc != "DMI")
    {
        qDebug() << "Fail to read 'DMI' from .dmi file";
        return false;
    }
    loc.clear();
    desc >> loc;
    if (loc != "version")
    {
        qDebug() << "Fail to read 'version' from .dmi file";
        return false;
    }
    loc.clear();
    desc >> loc;
    if (loc != "=")
    {
        qDebug() << "Fail to read '=' from .dmi file";
        return false;
    }
    loc.clear();
    ////////////
    {
        std::string local;
        desc >> local;
        dmi_version_ = QString::fromStdString(local);
    }
    qDebug() << "Read version: " << dmi_version_;
    ////////////
    desc >> loc;
    if (loc != "width")
    {
        qDebug() << "Fail to read 'width' from .dmi file";
        return false;
    }
    loc.clear();
    desc >> loc;
    if (loc != "=")
    {
        qDebug() << "Fail to read '=' from .dmi file";
        return false;
    }
    loc.clear();
    
    //////////////
    desc >> width_;
    qDebug() << "Read width: " << width_;
    /////////////

    desc >> loc;
    if (loc != "height")
    {
        qDebug() << "Fail to read 'height' from .dmi file";
        return false;
    }
    loc.clear();
    desc >> loc;
    if (loc != "=")
    {
        qDebug() << "Fail to read '=' from .dmi file";
        return false;
    }
    loc.clear();
    
    /////////////
    desc >> height_;
    qDebug() << "Read height: " << height_;
    ////////////
    
    desc >> loc;
    QString current_state = "###";
    quint32 first_frame_pos = 0;
    while (loc != "#")
    {
        if (loc == "state")
        {
            loc.clear();
            desc >> loc;
            if (loc != "=")
            {
                qDebug() << "Fail to read '=' from .dmi file";
                return false;
            }
            std::string whole_str = "";
               
            loc.clear();
            desc >> loc;
            whole_str += loc;
            while (loc[loc.length() - 1] != '"')
            {
                loc.clear();
                desc >> loc;
                whole_str += " ";
                whole_str += loc;
            }
            loc = whole_str;
            qDebug() << "New state: " << QString::fromStdString(loc);
            current_state = QString::fromStdString(loc.substr(1, loc.length() - 2));
            metadata_[current_state].first_frame_pos = first_frame_pos;
        }
        else if (loc == "dirs")
        {
            loc.clear();
            desc >> loc;
            if (loc != "=")
            {
                qDebug() << "Fail to read '=' from .dmi file";
                return false;
            }
            loc.clear();

            quint32 dirs;
            desc >> dirs;

            if (current_state == "###")
            {
                qDebug() << "Dirs without state";
                return false;
            }
            metadata_[current_state].dirs = dirs;
        }
        else if (loc == "frames")
        {
            loc.clear();
            desc >> loc;
            if (loc != "=")
            {
                qDebug() << "Fail to read '=' from .dmi file";
                return false;
            }
            loc.clear();

            quint32 frames;
            desc >> frames;

            if (current_state == "###")
            {
                qDebug() << "Frames without state";
                return false;
            }
            metadata_[current_state].frames_data.resize(frames);
            first_frame_pos += frames * metadata_[current_state].dirs;
        }
        else if (loc == "delay")
        {
            loc.clear();
            desc >> loc;
            if (loc != "=")
            {
                qDebug() << "Fail to read '=' from .dmi file";
                return false;
            }
            loc.clear();

            if (current_state == "###")
            {
                qDebug() << "Delay without state";
                return false;
            }

            for (quint32 i = 0; i < metadata_[current_state].frames_data.size() - 1; ++i)
            {
                quint32 value;
                desc >> value;
                metadata_[current_state].frames_data[i].delay = value;

                char comma;
                desc >> comma;
                if (comma != ',')
                {
                    qDebug() << "Fail to read ',' from .dmi file";
                    return false;
                }
            }
            quint32 value;
            desc >> value;
            metadata_[current_state].frames_data
                [metadata_[current_state].frames_data.size() - 1]
                    .delay = value;
        }
        else if (loc == "rewind")
        {
            loc.clear();
            desc >> loc;
            if (loc != "=")
            {
                qDebug() << "Fail to read '=' from .dmi file";
                return false;
            }
            loc.clear();

            if (current_state == "###")
            {
                qDebug() << "Rewind without state";
                return false;
            }

            quint32 rewind;
            desc >> rewind;
            metadata_[current_state].rewind = rewind ? true : false;
        }
        else if (loc == "loop")
        {
            loc.clear();
            desc >> loc;
            if (loc != "=")
            {
                qDebug() << "Fail to read '=' from .dmi file";
                return false;
            }
            loc.clear();

            if (current_state == "###")
            {
                qDebug() << "Loop without state";
                return false;
            }

            int loop;
            desc >> loop;
            metadata_[current_state].loop = loop;
        }
        else if (loc == "hotspot")
        {
            loc.clear();
            desc >> loc;
            if (loc != "=")
            {
                qDebug() << "Fail to read '=' from .dmi file";
                return false;
            }
            loc.clear();

            if (current_state == "###")
            {
                qDebug() << "Hotspot without state";
                return false;
            }
            
            int num;
            desc >> num;
            metadata_[current_state].hotspot[0] = num;

            char comma;
            desc >> comma;
            if (comma != ',')
            {
                qDebug() << "Fail to read ',' from .dmi file";
                return false;
            }

            desc >> num;
            metadata_[current_state].hotspot[1] = num;

            desc >> comma;
            if (comma != ',')
            {
                qDebug() << "Fail to read ',' from .dmi file";
                return false;
            }

            desc >> num;
            metadata_[current_state].hotspot[2] = num;

        }
        else
        {
            // That happens quite oftern so it is needed
            // to detect unknon params in DMI format
            qDebug() << "Unknown param: " << QString::fromStdString(loc);
            KvAbort();
        }
        loc.clear();
        desc >> loc;
    }

    qDebug() << "End of states";

    loc.clear();
    desc >> loc;
    if (loc != "END")
    {
        qDebug() << "Fail to read 'END' from .dmi file";
        return false;
    }
    loc.clear();
    desc >> loc;
    if (loc != "DMI")
    {
        qDebug() << "Fail to read 'DMI' from .dmi file";
        return false;
    }

    qDebug() << "Begin make sequence";

    MakeSequence();

    qDebug() << "End make sequence";

    valid_ = true;
    return true;
}

void ImageMetadata::MakeSequence()
{
    for (auto it = metadata_.begin(); it != metadata_.end(); ++it)
    {
        auto& metadata = it->second.frames_data;
        auto& sequence = it->second.frames_sequence;
        int local_loop = it->second.loop;
        if (it->second.loop == -1 || it->second.loop == 0)
        {
            local_loop = 1;
        }

        for (int loop_i = 0; loop_i < local_loop; ++loop_i)
        {
            for (quint32 i = 0; i < metadata.size(); ++i)
            {
                sequence.push_back(i);
            }
            if (it->second.rewind)
            {
                int from = metadata.size() - 2;
                if (from < 0)
                {
                    from = 0;
                }
                for (quint32 i = from; i > 0; --i)
                {
                    sequence.push_back(i);
                }
            }
        }
        if (!(it->second.loop == -1 || it->second.loop == 0))
        {
            sequence.push_back(-1);
        }
    }
}
