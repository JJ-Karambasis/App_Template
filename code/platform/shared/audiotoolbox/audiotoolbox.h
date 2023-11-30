#ifndef AUDIOTOOLBOX_H
#define AUDIOTOOLBOX_H

#include <AudioToolbox/AudioToolbox.h>

#define AUDIO_TOOLBOX_SOUND_BYTES_PER_SAMPLE 2

struct audio_toolbox {
    AudioComponent Component;
    AudioComponentInstance Instance;
    atom32 IsPlaying;

    void Start_Playing(); 
    void Stop_Playing();
};

audio_toolbox* Audio_Toolbox_Create(allocator* Allocator, app* App);

#endif