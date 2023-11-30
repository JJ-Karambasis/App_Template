sound_samples_f32 Sound_Allocate_Samples(allocator* Allocator, u64 SampleCount) {
    sound_samples_f32 Result = {
        .SampleCount = SampleCount,
    };

    f32* Samples = new(Allocator) f32[SampleCount*2];
    Result.LeftChannel  = Samples;
    Result.RightChannel = Samples+SampleCount;
    return Result;
}

void Sound_Free_Samples(allocator* Allocator, sound_samples_f32* Samples) {
    operator delete(Samples->LeftChannel, Allocator);
}

#define Riff_Code(c0, c1, c2, c3) (((u8)(c0) << 0) | ((u8)(c1) << 8) | ((u8)(c2) << 16) | ((u8)(c3) << 24))
#pragma pack(push, 1)
struct wav_format {
    u16 Type;
    u16 ChannelCount;
    u32 SampleRate;
    u32 AvgBytesPerSec;
    u16 ByteAlignment;
    u16 BitsPerSample;
};
#define WAV_FORMAT_PCM 1
#pragma pack(pop)

bool Sound_Load_Samples(sound_samples_f32* DstSamples, allocator* Allocator, const string& Path) {
    //See https://docs.fileformat.com/audio/wav/ for wav file specification

    scratch Scratch = Get_Scratch();

    const_buffer WAVFile = Read_Entire_File(&Scratch, Path);
    if(WAVFile.Is_Empty()) {
        return false;
    }

    memory_stream Stream(WAVFile);

    if(Stream.Consume_U32() != Riff_Code('R', 'I', 'F', 'F')) {
        //TODO: Diangostic and error logging
        return false;
    }

    Stream.Skip(sizeof(u32));

    if(Stream.Consume_U32() != Riff_Code('W', 'A', 'V', 'E')) {
        //TODO: Diagnostic and error logging
        return false;
    }

    bool  ReadFormat = false;
    u32   ChannelCount = 2;
    u32   SampleByteCount = 0;
    const void* Samples = nullptr;

    while(Stream.Is_Valid()) {
        u32 ChunkHeader = Stream.Consume_U32();
        u32 ChunkSize = Stream.Consume_U32();

        switch(ChunkHeader) {
            case Riff_Code('f', 'm', 't', ' '): {
                wav_format* WavFormat = (wav_format*)Stream.Peek();

                if(WavFormat->Type != WAV_FORMAT_PCM) {
                    //TODO: Diagnostic and error logging
                    return false;
                }

                if((WavFormat->ChannelCount != 1) && (WavFormat->ChannelCount != 2)) {
                    //TODO: Diagnostic and error logging
                    return false;
                }

                if(WavFormat->SampleRate != 44100) {
                    //TODO: Diagnostic and error logging
                    return false;
                }

                if(WavFormat->BitsPerSample != 16) {
                    //TODO: Diagnostic and error logging
                    return false;
                }

                ChannelCount = WavFormat->ChannelCount;

                ReadFormat = true;
            } break;

            case Riff_Code('d', 'a', 't', 'a'): {
                //TODO: Confirm if this is true or not!
                Assert(!Samples, "Cannot have more than one data section!");

                if(!ReadFormat) {
                    //TODO: Diagnostic and error logging
                    return false;
                }

                SampleByteCount = ChunkSize;
                Samples = Stream.Peek();
            } break;
        }

        Stream.Skip(ChunkSize);
    }

    if(!Samples) {
        //TODO: Diagnostic and error logging
        return false;
    }
    
    u32 SampleCount = SampleByteCount / (ChannelCount*2);
    *DstSamples = Sound_Allocate_Samples(Allocator, SampleCount);

    s16* SrcSamples = (s16*)Samples;
    if(ChannelCount == 2) {
        for(u32 SampleIndex = 0; SampleIndex < SampleCount; SampleIndex++) {
            DstSamples->LeftChannel[SampleIndex] = SNorm(*SrcSamples++);
            DstSamples->RightChannel[SampleIndex] = SNorm(*SrcSamples++);
        }
    } else {
        Assert(ChannelCount == 1, "Invalid channel count!");
        for(u32 SampleIndex = 0; SampleIndex < SampleCount; SampleIndex++) {
            f32 Sample = SNorm(*SrcSamples++);
            DstSamples->LeftChannel[SampleIndex] = Sample;
            DstSamples->RightChannel[SampleIndex] = Sample;
        }
    }

    return true;
}

#undef Riff_Code