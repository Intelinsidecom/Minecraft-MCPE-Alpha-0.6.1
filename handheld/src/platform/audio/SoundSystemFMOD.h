#ifndef SoundSystemFMOD_H__
#define SoundSystemFMOD_H__

#include "SoundSystem.h"
#include "FMOD/fmod.h"

#include <vector>
#include <string>

class SoundSystemFMOD : public SoundSystem
{
public:
    SoundSystemFMOD();
    ~SoundSystemFMOD();

    virtual void init();
    virtual void destroy();

    virtual bool isAvailable() { return available; }
    virtual void enable(bool status);

    virtual void setListenerPos(float x, float y, float z);
    virtual void setListenerAngle(float deg);

    virtual void load(const std::string& name) {}
    virtual void play(const std::string& name) {}
    virtual void pause(const std::string& name) {}
    virtual void stop(const std::string& name) {}

    virtual void playAt(const SoundDesc& sound, float x, float y, float z, float volume, float pitch);

private:
    class Buffer {
    public:
        Buffer()
        :   inited(false),
            sound(nullptr),
            framePtr(nullptr)
        {}
        bool inited;
        FMOD_SOUND* sound;
        char* framePtr;
    };

    class Channel {
    public:
        Channel()
        :   channel(nullptr),
            active(false)
        {}
        FMOD_CHANNEL* channel;
        bool active;
    };

    bool checkError(FMOD_RESULT result, const char* operation);
    void removeStoppedSounds();

    bool getFreeChannelIndex(int* channelIndex);
    bool getBufferId(const SoundDesc& sound, FMOD_SOUND** fmodSound);

    static const int MaxNumSources = 128;

    Vec3 _listenerPos;
    float _rotation;

    bool available;

    FMOD_SYSTEM* system;

    Channel _channels[MaxNumSources];
    std::vector<Buffer> _buffers;
};

#endif /*SoundSystemFMOD_H__*/
