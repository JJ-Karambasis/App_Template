struct audio_toolbox_thread {
    app* App;
    audio_toolbox* Toolbox;
};

static OSStatus Audio_Toolbox_Thread(void* UserData,
                                     AudioUnitRenderActionFlags* ActionFlags,
                                     const AudioTimeStamp* TimeStamp,
                                     uint32_t BusNumber,
                                     uint32_t NumFrames,
                                     AudioBufferList* Data) {
    audio_toolbox_thread* Thread = (audio_toolbox_thread*)UserData;
    
    app* App = Thread->App;

    scratch Scratch = Get_Scratch();
    sound_samples_f32 Samples = Sound_Allocate_Samples(&Scratch, NumFrames);

    App->Mix_Samples(App, &Samples);
    
    f32* LeftChannelAt = Samples.LeftChannel;
    f32* RightChannelAt = Samples.RightChannel;
    s16* OutputAudio = (s16*)Data->mBuffers[0].mData;
    for(u32 FrameIndex = 0; FrameIndex < NumFrames; FrameIndex++) {
        *OutputAudio++ = SNorm_S16(*LeftChannelAt++);
        *OutputAudio++ = SNorm_S16(*RightChannelAt++);
    }

    return noErr;
}

void audio_toolbox::Start_Playing() {
    AudioOutputUnitStart(Instance);
}

void audio_toolbox::Stop_Playing() {
    AudioOutputUnitStop(Instance);
}

audio_toolbox* Audio_Toolbox_Create(allocator* Allocator, app* App) {
    audio_toolbox* Result = new(Allocator) audio_toolbox;

    AudioComponentDescription ComponentDescription = {
        .componentType = kAudioUnitType_Output,
#ifdef OS_IOS
        .componentSubType = kAudioUnitSubType_RemoteIO,
#else
        .componentSubType = kAudioUnitSubType_DefaultOutput,
#endif
        .componentManufacturer = kAudioUnitManufacturer_Apple
    };

    Result->Component = AudioComponentFindNext(nullptr, &ComponentDescription);
    OSStatus Status = AudioComponentInstanceNew(Result->Component, &Result->Instance);

    if(Status != noErr) {
        //TODO: Diagnostic and error logging
        return nullptr;
    }

    AudioStreamBasicDescription StreamDescription = {
        .mSampleRate = SOUND_SAMPLES_PER_SEC,
        .mFormatID = kAudioFormatLinearPCM,
        .mFormatFlags = kAudioFormatFlagIsSignedInteger|kAudioFormatFlagIsPacked,
        .mFramesPerPacket = 1,
        .mChannelsPerFrame = SOUND_NUM_CHANNELS,
        .mBitsPerChannel = AUDIO_TOOLBOX_SOUND_BYTES_PER_SAMPLE*8,
        .mBytesPerFrame = AUDIO_TOOLBOX_SOUND_BYTES_PER_SAMPLE*SOUND_NUM_CHANNELS,
        .mBytesPerPacket = AUDIO_TOOLBOX_SOUND_BYTES_PER_SAMPLE*SOUND_NUM_CHANNELS
    };

    Status = AudioUnitSetProperty(Result->Instance, kAudioUnitProperty_StreamFormat,
                                  kAudioUnitScope_Input, 0, &StreamDescription,
                                  sizeof(StreamDescription));
    
    if(Status != noErr) {
        //TODO: Diagnostic and error logging
        return nullptr;
    }

    audio_toolbox_thread* ThreadData = new(Allocator) audio_toolbox_thread;
    ThreadData->App = App;
    ThreadData->Toolbox = Result;

    AURenderCallbackStruct AudioCallback = {
        .inputProc = Audio_Toolbox_Thread,
        .inputProcRefCon = ThreadData
    };

    Status = AudioUnitSetProperty(Result->Instance,
                                  kAudioUnitProperty_SetRenderCallback,
                                  kAudioUnitScope_Global, 0,
                                  &AudioCallback, sizeof(AudioCallback));
    if(Status != noErr) {
        //TODO: Diagnostic and error logging
        return nullptr;
    }

    AudioUnitInitialize(Result->Instance);

    return Result;
}