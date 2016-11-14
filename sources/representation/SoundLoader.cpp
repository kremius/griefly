#include "SoundLoader.h"
#include "Sound.h"

void LoadSounds()
{
    GetSoundManager().LoadSound("glass_step.ogg");
    GetSoundManager().LoadSound("airlock.ogg");
    GetSoundManager().LoadSound("Wirecutter.ogg");
    GetSoundManager().LoadSound("Screwdriver.ogg");
}
