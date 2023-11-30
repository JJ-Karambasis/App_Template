#ifndef SOUND_H
#define SOUND_H

#define SOUND_SAMPLES_PER_SEC 44100
#define SOUND_NUM_CHANNELS 2

struct sound_samples_f32 {
    u64  SampleCount;
    f32* LeftChannel;
    f32* RightChannel;
};

sound_samples_f32 Sound_Allocate_Samples(allocator* Allocator, u64 SampleCount);
void              Sound_Free_Samples(allocator* Allocator, sound_samples_f32* Samples);
bool              Sound_Load_Samples(sound_samples_f32* DstSamples, allocator* Allocator, const string& Path);

#endif