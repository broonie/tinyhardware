/*
** Copyright 2011, Wolfson Microelectronics plc
** Copyright 2008, The Android Open-Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef ANDROID_AUDIO_HARDWARE_H
#define ANDROID_AUDIO_HARDWARE_H

#include <stdint.h>
#include <sys/types.h>

#include <utils/threads.h>
#include <utils/SortedVector.h>

#include <hardware_legacy/AudioHardwareBase.h>
#include <media/mediarecorder.h>

extern "C" {
struct pcm;
struct mixer;
};

namespace android {

// ----------------------------------------------------------------------------

class TinyAudioHardware;

class TinyAudioStreamOut : public AudioStreamOut {
public:
                        TinyAudioStreamOut() : mAudioHardware(0) {}
    virtual             ~TinyAudioStreamOut();

    virtual status_t    set(TinyAudioHardware *hw,
			    int *pFormat,
			    uint32_t *pChannels,
			    uint32_t *pRate);

    virtual uint32_t    sampleRate() const { return mSampleRate; }
    virtual size_t      bufferSize() const { return 2048; }
    virtual uint32_t    channels() const { return AudioSystem::CHANNEL_OUT_STEREO; }
    virtual int         format() const { return AudioSystem::PCM_16_BIT; }
    virtual uint32_t    latency() const { return 20; }
    virtual status_t    setVolume(float left, float right) { return INVALID_OPERATION; }
    virtual ssize_t     write(const void* buffer, size_t bytes);
    virtual status_t    standby();
    virtual status_t    dump(int fd, const Vector<String16>& args);
    virtual status_t    setParameters(const String8& keyValuePairs);
    virtual String8     getParameters(const String8& keys);
    virtual status_t    getRenderPosition(uint32_t *dspFrames);

private:
    TinyAudioHardware *mAudioHardware;
    Mutex   mLock;
    struct pcm *mPcm;
    uint32_t mDevice;
    uint32_t mSampleRate;
};

class TinyAudioStreamIn : public AudioStreamIn {
public:
                        TinyAudioStreamIn() : mAudioHardware(0) {}
    virtual             ~TinyAudioStreamIn();

    virtual status_t    set(
            TinyAudioHardware *hw,
            int *pFormat,
            uint32_t *pChannels,
            uint32_t *pRate,
            AudioSystem::audio_in_acoustics acoustics);

    virtual uint32_t    sampleRate() const { return 8000; }
    virtual size_t      bufferSize() const { return 320; }
    virtual uint32_t    channels() const { return AudioSystem::CHANNEL_IN_MONO; }
    virtual int         format() const { return AudioSystem::PCM_16_BIT; }
    virtual status_t    setGain(float gain) { return INVALID_OPERATION; }
    virtual ssize_t     read(void* buffer, ssize_t bytes);
    virtual status_t    dump(int fd, const Vector<String16>& args);
    virtual status_t    standby() { return NO_ERROR; }
    virtual status_t    setParameters(const String8& keyValuePairs);
    virtual String8     getParameters(const String8& keys);
    virtual unsigned int  getInputFramesLost() const { return 0; }

private:
    TinyAudioHardware *mAudioHardware;
    Mutex   mLock;
    struct pcm *mPcm;
    uint32_t mDevice;
};


class TinyAudioHardware : public AudioHardwareBase
{
public:
                        TinyAudioHardware();
    virtual             ~TinyAudioHardware();
    virtual status_t    initCheck();
    virtual status_t    setVoiceVolume(float volume);
    virtual status_t    setMasterVolume(float volume);

    // mic mute
    virtual status_t    setMicMute(bool state);
    virtual status_t    getMicMute(bool* state);

    // create I/O streams
    virtual AudioStreamOut* openOutputStream(
            uint32_t devices,
            int *format=0,
            uint32_t *channels=0,
            uint32_t *sampleRate=0,
            status_t *status=0);
    virtual    void        closeOutputStream(AudioStreamOut* out);

    virtual AudioStreamIn* openInputStream(
            uint32_t devices,
            int *format,
            uint32_t *channels,
            uint32_t *sampleRate,
            status_t *status,
            AudioSystem::audio_in_acoustics acoustics);
    virtual    void        closeInputStream(AudioStreamIn* in);

            void            closeOutputStream(TinyAudioStreamOut* out);
            void            closeInputStream(TinyAudioStreamIn* in);
protected:
    virtual status_t        dump(int fd, const Vector<String16>& args);

private:
    status_t                dumpInternals(int fd, const Vector<String16>& args);

    Mutex                   mLock;
    TinyAudioStreamOut   *mOutput;
    TinyAudioStreamIn    *mInput;
    struct mixer            *mMixer;
    bool                    mMicMute;
};

// ----------------------------------------------------------------------------

}; // namespace android

#endif
