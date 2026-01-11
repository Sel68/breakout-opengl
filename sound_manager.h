#include <AL/al.h>
#include <AL/alc.h>
#include <map>
#include <string>
#include <vector>
#include <iostream>

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"

struct SoundManager {
    ALCdevice* device;
    ALCcontext* context;
    std::map<std::string, ALuint> buffers;
    std::vector<ALuint> sources; // Pool for fire-and-forget sounds

    SoundManager() {
        device = alcOpenDevice(nullptr);
        context = alcCreateContext(device, nullptr);
        alcMakeContextCurrent(context);
    }

    // Load any supported format into a buffer
    void load(const std::string& name, const std::string& path) {
        ALuint buffer;
        alGenBuffers(1, &buffer);

        ALenum format;
        ALsizei size;
        ALsizei freq;
        void* data = nullptr;

        if (path.substr(path.find_last_of(".") + 1) == "wav") {
            unsigned int channels;
            unsigned int sampleRate;
            drwav_uint64 totalPCMFrameCount;
            data = drwav_open_file_and_read_pcm_frames_s16(path.c_str(), &channels, &sampleRate, &totalPCMFrameCount, nullptr);
            format = (channels > 1) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
            freq = sampleRate;
            size = (ALsizei)(totalPCMFrameCount * channels * sizeof(short));
        } else {
            drmp3_config config;
            drmp3_uint64 totalPCMFrameCount;
            data = drmp3_open_file_and_read_pcm_frames_s16(path.c_str(), &config, &totalPCMFrameCount, nullptr);
            format = (config.channels > 1) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
            freq = config.sampleRate;
            size = (ALsizei)(totalPCMFrameCount * config.channels * sizeof(short));
        }

        alBufferData(buffer, format, data, size, freq);
        buffers[name] = buffer;
        free(data);
    }

    // High-level Play function
    void play(const std::string& name, bool loop = false) {
        ALuint source;
        alGenSources(1, &source);
        alSourcei(source, AL_BUFFER, buffers[name]);
        alSourcei(source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
        alSourcePlay(source);
        sources.push_back(source); 
        
        // Optional: Clean up finished sources here
    }

    ~SoundManager() {
        for (auto s : sources) alDeleteSources(1, &s);
        for (auto const& [name, b] : buffers) alDeleteBuffers(1, &b);
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(context);
        alcCloseDevice(device);
    }
};