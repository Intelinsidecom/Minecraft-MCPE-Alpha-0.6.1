#include "SoundSystemFMOD.h"
#include "../../util/Mth.h"
#include "../../world/level/tile/Tile.h"
#include "../../world/phys/Vec3.h"
#include "../../client/sound/Sound.h"

#include "../log.h"
#include <cstring>

#pragma pack(push, 1)
struct WAVHeader {
    char riff[4];           // "RIFF"
    uint32_t fileSize;      // file size - 8
    char wave[4];         // "WAVE"
    char fmt[4];          // "fmt "
    uint32_t fmtSize;     // 16 for PCM
    uint16_t format;      // 1 = PCM
    uint16_t channels;
    uint32_t sampleRate;
    uint32_t byteRate;    // sampleRate * channels * bitsPerSample/8
    uint16_t blockAlign;  // channels * bitsPerSample/8
    uint16_t bitsPerSample;
    char data[4];         // "data"
    uint32_t dataSize;    // size of PCM data

    void init(int numChannels, int sampleRateHz, int bitsPerSample, int dataSizeBytes) {
        memcpy(riff, "RIFF", 4);
        fileSize = 36 + dataSizeBytes;
        memcpy(wave, "WAVE", 4);
        memcpy(fmt, "fmt ", 4);
        fmtSize = 16;
        format = 1; // PCM
        channels = numChannels;
        this->sampleRate = sampleRateHz;
        this->bitsPerSample = bitsPerSample;
        blockAlign = channels * (bitsPerSample / 8);
        byteRate = sampleRateHz * blockAlign;
        memcpy(data, "data", 4);
        dataSize = dataSizeBytes;
    }
};
#pragma pack(pop)

static const char* errIdString = 0;

bool SoundSystemFMOD::checkError(FMOD_RESULT result, const char* operation)
{
    if (result != FMOD_OK) {
        LOGE("### SoundSystemFMOD error @ %s: %d ####: %s\n", operation ? operation : "(none)", result, errIdString ? errIdString : "(none)");
        return false;
    }
    return true;
}

SoundSystemFMOD::SoundSystemFMOD()
:   available(true),
    system(nullptr),
    _rotation(-9999.9f)
{
    _buffers.reserve(64);
    for (int i = 0; i < MaxNumSources; ++i) {
        _channels[i].channel = nullptr;
        _channels[i].active = false;
    }
    init();
}

SoundSystemFMOD::~SoundSystemFMOD()
{
    if (system) {
        for (int i = 0; i < (int)_buffers.size(); ++i) {
            if (_buffers[i].inited && _buffers[i].sound) {
                FMOD_Sound_Release(_buffers[i].sound);
            }
        }
        _buffers.clear();

        for (int i = 0; i < MaxNumSources; ++i) {
            if (_channels[i].channel) {
                FMOD_Channel_Stop(_channels[i].channel);
            }
        }

        FMOD_System_Close(system);
        FMOD_System_Release(system);
        system = nullptr;
    }
}

void SoundSystemFMOD::init()
{
    FMOD_RESULT result;

    result = FMOD_System_Create(&system);
    if (!checkError(result, "System_Create")) {
        available = false;
        return;
    }

    unsigned int version;
    result = FMOD_System_GetVersion(system, &version);
    checkError(result, "GetVersion");

    result = FMOD_System_SetSoftwareChannels(system, MaxNumSources);
    checkError(result, "SetSoftwareChannels");

    result = FMOD_System_Set3DSettings(system, 1.0f, 1.0f, 1.0f);
    checkError(result, "Set3DSettings");

    result = FMOD_System_Init(system, MaxNumSources, FMOD_INIT_NORMAL, nullptr);
    if (!checkError(result, "System_Init")) {
        available = false;
        FMOD_System_Release(system);
        system = nullptr;
        return;
    }

    float listenerPos[] = {0, 0, 0};
    float listenerVel[] = {0, 0, 0};
    float listenerForward[] = {0.0f, 0.0f, 1.0f};
    float listenerUp[] = {0.0f, 1.0f, 0.0f};
    FMOD_System_Set3DListenerAttributes(system, 0, (FMOD_VECTOR*)listenerPos, (FMOD_VECTOR*)listenerVel, (FMOD_VECTOR*)listenerForward, (FMOD_VECTOR*)listenerUp);

    errIdString = "Init audio";
    checkError(result, "Set3DListenerAttributes");
}

void SoundSystemFMOD::destroy()
{
}

void SoundSystemFMOD::enable(bool status)
{
    LOGI("Enabling? audio: %d (system %p)\n", status, system);
    if (!system) return;

    if (status) {
        FMOD_System_MixerResume(system);
        errIdString = "Enable audio";
    } else {
        FMOD_System_MixerSuspend(system);
        errIdString = "Disable audio";
    }

    checkError(FMOD_OK, status ? "MixerResume" : "MixerSuspend");
}

void SoundSystemFMOD::setListenerPos(float x, float y, float z)
{
    if (_listenerPos.x != x || _listenerPos.y != y || _listenerPos.z != z) {
        _listenerPos.set(x, y, z);

        FMOD_VECTOR pos = {0.0f, 0.0f, 0.0f};
        FMOD_VECTOR vel = {0.0f, 0.0f, 0.0f};
        FMOD_VECTOR forward = {0.0f, 0.0f, 1.0f};
        FMOD_VECTOR up = {0.0f, 1.0f, 0.0f};

        float rad = _rotation * Mth::DEGRAD;
        forward.x = -Mth::sin(rad);
        forward.z = Mth::cos(rad);

        FMOD_System_Set3DListenerAttributes(system, 0, &pos, &vel, &forward, &up);
    }
}

void SoundSystemFMOD::setListenerAngle(float deg)
{
    if (_rotation != deg) {
        _rotation = deg;

        FMOD_VECTOR pos = {0.0f, 0.0f, 0.0f};
        FMOD_VECTOR vel = {0.0f, 0.0f, 0.0f};
        FMOD_VECTOR forward;
        FMOD_VECTOR up = {0.0f, 1.0f, 0.0f};

        float rad = deg * Mth::DEGRAD;
        forward.x = -Mth::sin(rad);
        forward.y = 0.0f;
        forward.z = Mth::cos(rad);

        FMOD_System_Set3DListenerAttributes(system, 0, &pos, &vel, &forward, &up);
    }
}

void SoundSystemFMOD::playAt(const SoundDesc& sound, float x, float y, float z, float volume, float pitch)
{
    if (pitch < 0.01f) pitch = 1.0f;

    if (!system) {
        LOGE("SoundSystemFMOD: System not initialized!\n");
        return;
    }

    FMOD_SOUND* fmodSound;
    if (!getBufferId(sound, &fmodSound)) {
        errIdString = "Get buffer (failed)";
        LOGE("getBufferId returned false!\n");
        return;
    }

    errIdString = "Get buffer";

    removeStoppedSounds();

    int channelIndex;
    if (!getFreeChannelIndex(&channelIndex)) {
        LOGI("No free sound sources left @ SoundSystemFMOD::playAt\n");
        return;
    }

    FMOD_CHANNEL* channel = nullptr;
    FMOD_RESULT result = FMOD_System_PlaySound(system, fmodSound, nullptr, true, &channel);
    errIdString = "PlaySound";
    if (!checkError(result, errIdString)) {
        return;
    }

    _channels[channelIndex].channel = channel;
    _channels[channelIndex].active = true;

    FMOD_VECTOR pos = {x, y, z};
    FMOD_VECTOR vel = {0.0f, 0.0f, 0.0f};
    result = FMOD_Channel_Set3DAttributes(channel, &pos, &vel, nullptr);
    errIdString = "Set3DAttributes";
    checkError(result, errIdString);

    result = FMOD_Channel_SetVolume(channel, volume);
    errIdString = "SetVolume";
    checkError(result, errIdString);

    result = FMOD_Channel_SetPitch(channel, pitch);
    errIdString = "SetPitch";
    checkError(result, errIdString);

    FMOD_Channel_SetMode(channel, FMOD_LOOP_OFF);
    errIdString = "SetMode";
    checkError(result, errIdString);

    FMOD_Channel_Set3DMinMaxDistance(channel, 5.0f, 16.0f);
    errIdString = "Set3DMinMaxDistance";
    checkError(result, errIdString);

    result = FMOD_Channel_SetPaused(channel, false);
    errIdString = "SetPaused(false)";
    checkError(result, errIdString);
}

void SoundSystemFMOD::removeStoppedSounds()
{
    int activeCount = 0;
    for (int i = 0; i < MaxNumSources; ++i) {
        if (_channels[i].active && _channels[i].channel) {
            FMOD_BOOL isPlaying = false;
            FMOD_RESULT result = FMOD_Channel_IsPlaying(_channels[i].channel, &isPlaying);
            if (result == FMOD_OK && !isPlaying) {
                _channels[i].active = false;
                _channels[i].channel = nullptr;
            } else if (result == FMOD_OK && isPlaying) {
                activeCount++;
            }
        }
    }
}

bool SoundSystemFMOD::getFreeChannelIndex(int* channelIndex)
{
    removeStoppedSounds();

    for (int i = 0; i < MaxNumSources; ++i) {
        if (!_channels[i].active) {
            *channelIndex = i;
            return true;
        }
        if (_channels[i].channel) {
            FMOD_BOOL isPlaying = false;
            FMOD_Channel_IsPlaying(_channels[i].channel, &isPlaying);
            if (!isPlaying) {
                _channels[i].active = false;
                *channelIndex = i;
                return true;
            }
        }
    }
    return false;
}

bool SoundSystemFMOD::getBufferId(const SoundDesc& sound, FMOD_SOUND** fmodSound)
{
    for (int i = 0; i < (int)_buffers.size(); ++i) {
        if (_buffers[i].framePtr == sound.frames) {
            *fmodSound = _buffers[i].sound;
            return true;
        }
    }

    if (!sound.isValid()) {
        LOGE("Err: sound is invalid @ getBufferId! %s\n", sound.name.c_str());
        return false;
    }

    int bitsPerSample = sound.byteWidth * 8;
    int wavHeaderSize = sizeof(WAVHeader);
    int totalSize = wavHeaderSize + sound.size;
    char* wavBuffer = new char[totalSize];
    
    WAVHeader header;
    header.init(sound.channels, sound.frameRate, bitsPerSample, sound.size);
    memcpy(wavBuffer, &header, wavHeaderSize);
    memcpy(wavBuffer + wavHeaderSize, sound.frames, sound.size);

    FMOD_CREATESOUNDEXINFO exinfo;
    memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
    exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
    exinfo.length = totalSize;

    FMOD_MODE mode = FMOD_OPENMEMORY | FMOD_CREATESAMPLE | FMOD_3D;

    FMOD_SOUND* newSound = nullptr;
    FMOD_RESULT result = FMOD_System_CreateSound(system, wavBuffer, mode, &exinfo, &newSound);
    
    delete[] wavBuffer;
    
    errIdString = "CreateSound";
    if (!checkError(result, errIdString)) {
        LOGE("Failed to create FMOD sound: %s\n", sound.name.c_str());
        return false;
    }

    result = FMOD_Sound_Set3DMinMaxDistance(newSound, 5.0f, 16.0f);
    errIdString = "Sound_Set3DMinMaxDistance";
    checkError(result, errIdString);

    Buffer buffer;
    buffer.inited = true;
    buffer.sound = newSound;
    buffer.framePtr = sound.frames;
    *fmodSound = newSound;
    _buffers.push_back(buffer);

    sound.destroy();
    return true;
}
