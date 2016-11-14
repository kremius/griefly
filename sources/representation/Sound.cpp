#include "Sound.h"

#include "Representation.h"

#include <QDebug>

#ifdef PlaySound
#undef PlaySound
#endif // PlaySound

SoundManager& GetSoundManager()
{
    static SoundManager snd_mngr;
    return snd_mngr;
}

void SoundManager::InitSound(sf::Sound* sound, const QString& name)
{
    sound->setBuffer(*GetBuffer(name));
}

void SoundManager::LoadSound(const QString& name)
{
    qDebug() << "Load sound " << name;
    holder_[name] = new sf::SoundBuffer;
    if (!holder_[name]->loadFromFile(("sounds/" + name).toStdString()))
    {
        qDebug() << "Error during load sound " << name;
    }
}

const sf::SoundBuffer* SoundManager::GetBuffer(const QString& name)
{
    if (!holder_[name])
    {
        LoadSound(name);
    }
    return holder_[name];
}

void InitSound(sf::Sound* sound, const QString& name)
{
    GetSoundManager().InitSound(sound, name);
}
void PlaySound(sf::Sound* sound, const QString& name)
{
    if (!sound->getBuffer())
    {
        InitSound(sound, name);
    }
    if (sound->getStatus() != sf::SoundSource::Playing)
    {
        sound->play();
    }
}

SoundPlayer& GetSoundPlayer()
{
    static SoundPlayer snd_player;
    return snd_player;
}

SoundPlayer::SoundPlayer()
{
    sounds_.resize(100);
}

sf::Sound* SoundPlayer::PlaySound(const QString& name)
{
    quint32 i;
    for (i = 0; i < sounds_.size(); ++i)
    {
        if (sounds_[i].getStatus() == sf::SoundSource::Stopped)
        {
            break;
        }
    }
    if (i == sounds_.size())
    {
        qDebug() << "Unable to play sound '" + name + "', the sound limit is reached "
                      << i;
    }
    GetSoundManager().InitSound(&sounds_[i], name);
    // TODO: volume
    sounds_[i].setVolume(15.0f);
    sounds_[i].play();
    return &sounds_[i];
}

void SoundPlayer::PlayMusic(const QString &name, float volume)
{
    StopMusic();

    if (!music_.openFromFile(("music/" + name).toStdString()))
    {
        qDebug() << "Cannot open music file " << "music/" << name;
        return;
    }
    music_.setLoop(true);
    music_.setVolume(volume);
    music_.play();
}

void SoundPlayer::StopMusic()
{
    if (music_.getStatus() != sf::Music::Status::Stopped)
    {
        music_.stop();
    }
}

sf::Sound* PlaySound(const QString& name)
{
    sf::Sound* s = GetSoundPlayer().PlaySound(name);
    //s->setPosition(mob_position::x, y, 0);
    return s;
}
