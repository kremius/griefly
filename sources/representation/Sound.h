#include <string>
#include <unordered_map>
#include <vector>

#include <SFML/Audio.hpp>

#include "core/Constheader.h"
#include "core/Hashes.h"

#ifdef PlaySound
#undef PlaySound
#endif // PlaySound

class SoundManager
{
public:
    void InitSound(sf::Sound* sound, const QString& name);
    void LoadSound(const QString& name);
private:
    const sf::SoundBuffer* GetBuffer(const QString& name);
    std::unordered_map<QString, sf::SoundBuffer*> holder_;
};

SoundManager& GetSoundManager();

class SoundPlayer
{
public:
    SoundPlayer();
    sf::Sound* PlaySound(const QString& name);
    void PlayMusic(const QString& name, float volume = 100.0f);
    void StopMusic();
private:
    sf::Music music_;
    std::vector<sf::Sound> sounds_;
};

SoundPlayer& GetSoundPlayer();
