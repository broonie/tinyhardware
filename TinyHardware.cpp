/*
** Copyright 2011 Wolfson Microelectronics plc
**
** Portions taken from Nexus S AudioHardware implementation:
**
** Copyright 2010, The Android Open-Source Project
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

#include <math.h>

#define LOG_TAG "AudioHardware"

extern "C" {
#include "tinyalsa/asoundlib.h"
};

#include "TinyHardware.h"

namespace android {

TinyAudioHardware::TinyAudioHardware()
    : mOutput(NULL), mInput(NULL), mMixer(::mixer_open(0)), mMicMute(false)
{
}

TinyAudioHardware::~TinyAudioHardware()
{
    closeOutputStream((AudioStreamOut *)mOutput);
    closeInputStream((AudioStreamIn *)mInput);
    if (mMixer)
	mixer_close(mMixer);
}

status_t TinyAudioHardware::initCheck()
{
    if (mMixer)
	return NO_ERROR;
    else
	return NO_INIT;
}

void mixer_ctl_set(struct mixer *mixer, const char *name, int val)
{
    struct mixer_ctl *ctl = mixer_get_ctl_by_name(mixer, name);
    int ret = mixer_ctl_set_value(ctl, 0, val);
    if (ret != 0)
	LOGE("Failed to set %s to %d\n", name, val);
    mixer_ctl_set_value(ctl, 1, val);
}

void mixer_ctl_set(struct mixer *mixer, const char *name, const char *val)
{
    struct mixer_ctl *ctl = mixer_get_ctl_by_name(mixer, name);
    int ret = mixer_ctl_set_enum_by_string(ctl, val);
    if (ret != 0)
	LOGE("Failed to set %s to %s\n", name, val);
}

AudioStreamOut* TinyAudioHardware::openOutputStream(uint32_t devices,
						    int *format,
						    uint32_t *channels,
						    uint32_t *sampleRate,
						    status_t *status)
{
    AutoMutex lock(mLock);

    LOGI("OPENOUT: %p\n", mOutput);

    // Currently we only allow one output stream, in future we should
    // support systems with multiple paths to the hardware.
    if (mOutput) {
        if (status) {
            *status = INVALID_OPERATION;
        }
        return 0;
    }

    // Hard code an output path for Speyside speakers for now
    mixer_ctl_set(mMixer, "DSP1RX", "AIF1");
    mixer_ctl_set(mMixer, "DAC1L Mixer DSP1 Switch", 1);
    mixer_ctl_set(mMixer, "DAC1R Mixer DSP1 Switch", 1);
    mixer_ctl_set(mMixer, "DSP1 Playback Switch", 1);
    mixer_ctl_set(mMixer, "DSP1 Playback Volume", 96);
    mixer_ctl_set(mMixer, "DAC1 Switch", 1);
    mixer_ctl_set(mMixer, "DAC1 Volume", 96);
    mixer_ctl_set(mMixer, "DSP2RX", "AIF1");
    mixer_ctl_set(mMixer, "DAC2L Mixer DSP1 Switch", 0);
    mixer_ctl_set(mMixer, "DAC2R Mixer DSP1 Switch", 0);
    mixer_ctl_set(mMixer, "DAC2L Mixer DSP2 Switch", 1);
    mixer_ctl_set(mMixer, "DAC2R Mixer DSP2 Switch", 1);
    mixer_ctl_set(mMixer, "DSP2 Playback Switch", 1);
    mixer_ctl_set(mMixer, "DSP2 Playback Volume", 96);
    mixer_ctl_set(mMixer, "DAC2 Switch", 1);
    mixer_ctl_set(mMixer, "DAC2 Volume", 96);
    mixer_ctl_set(mMixer, "DAC2 Sidetone", 24);
    mixer_ctl_set(mMixer, "Sub Speaker Switch", 1);
    mixer_ctl_set(mMixer, "Sub Speaker Volume", 57);
    mixer_ctl_set(mMixer, "Sub Speaker DC Volume", 3);
    mixer_ctl_set(mMixer, "Sub Speaker AC Volume", 3);
    mixer_ctl_set(mMixer, "Speaker Switch", 1);
    mixer_ctl_set(mMixer, "Output 2 Volume", 15);
    mixer_ctl_set(mMixer, "DSP1 EQ Mode", "Sub HPF");
    mixer_ctl_set(mMixer, "DSP2 EQ Mode", "Sub LPF");

    mixer_ctl_set(mMixer, "Sub IN1 Switch", 1);
    mixer_ctl_set(mMixer, "Sub IN2 Switch", 1);
    mixer_ctl_set(mMixer, "SPKL", "DAC1L");
    mixer_ctl_set(mMixer, "SPKR", "DAC1R");
    mixer_ctl_set(mMixer, "DSP1 EQ Switch", 1);
    mixer_ctl_set(mMixer, "DSP2 EQ Switch", 1);

    TinyAudioStreamOut* out = new TinyAudioStreamOut();
    status_t lStatus = out->set(this, format, channels, sampleRate);
    if (status) {
        *status = lStatus;
    }
    if (lStatus == NO_ERROR) {
        mOutput = out;
    } else {
        delete out;
    }

    LOGI("OPENOUT DONE: %p %d\n", mOutput, lStatus);

    return mOutput;
}

void TinyAudioHardware::closeOutputStream(AudioStreamOut* out) {
    LOGI("CLOSE OUTPUT");
    if (mOutput && out == mOutput) {
        delete mOutput;
        mOutput = 0;
    }
}

AudioStreamIn* TinyAudioHardware::openInputStream(
        uint32_t devices, int *format, uint32_t *channels,
	uint32_t *sampleRate, status_t *status,
	AudioSystem::audio_in_acoustics acoustics)
{
    // check for valid input source
    if (!AudioSystem::isInputDevice((AudioSystem::audio_devices)devices)) {
        return 0;
    }

    AutoMutex lock(mLock);

    // Only one input stream allowed for now
    if (mInput) {
        if (status) {
            *status = INVALID_OPERATION;
        }
        return 0;
    }

    // create new output stream
    TinyAudioStreamIn* in = new TinyAudioStreamIn();
    status_t lStatus = in->set(this, format, channels, sampleRate, acoustics);
    if (status) {
        *status = lStatus;
    }
    if (lStatus == NO_ERROR) {
        mInput = in;
    } else {
        delete in;
    }
    return mInput;
}

void TinyAudioHardware::closeInputStream(AudioStreamIn* in) {
    if (mInput && in == mInput) {
        delete mInput;
        mInput = 0;
    }
}

status_t TinyAudioHardware::setVoiceVolume(float v)
{
    // If we ever want to do this via the RIL that'll be fun...
    return NO_ERROR;
}

status_t TinyAudioHardware::setMasterVolume(float v)
{
    // Implement: set master volume
    // return error - software mixer will handle it
    return INVALID_OPERATION;
}

status_t TinyAudioHardware::setMicMute(bool state)
{
    mMicMute = state;
    return NO_ERROR;
}

status_t TinyAudioHardware::getMicMute(bool* state)
{
    *state = mMicMute;
    return NO_ERROR;
}

status_t TinyAudioHardware::dumpInternals(int fd, const Vector<String16>& args)
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;
    ::write(fd, result.string(), result.size());
    return NO_ERROR;
}

status_t TinyAudioHardware::dump(int fd, const Vector<String16>& args)
{
    dumpInternals(fd, args);
    if (mInput) {
        mInput->dump(fd, args);
    }
    if (mOutput) {
        mOutput->dump(fd, args);
    }
    return NO_ERROR;
}

// ----------------------------------------------------------------------------

status_t TinyAudioStreamOut::set(
        TinyAudioHardware *hw,
        int *pFormat,
        uint32_t *pChannels,
        uint32_t *pRate)
{
    LOGI("TinyAudioStreamOut::set(): asked for rate %dHz, %d channels",
	 *pRate, *pChannels);

    struct pcm_config pcmConfig;

    memset(&pcmConfig, 0, sizeof(pcmConfig));
    pcmConfig.channels = 2;
    pcmConfig.period_size = 2048;
    pcmConfig.period_count = 12;
    pcmConfig.format = PCM_FORMAT_S16_LE;

    // Try 44.1kHz first...
    pcmConfig.rate = 44100;
    mPcm = pcm_open(0, 0, PCM_OUT, &pcmConfig);

    // ...but fall back to 48kHz
    if (!mPcm || !::pcm_is_ready(mPcm)) {
	pcm_close(mPcm);
	pcmConfig.rate = 48000;
	mPcm = pcm_open(0, 0, PCM_OUT, &pcmConfig);
    }

    if (!mPcm || !::pcm_is_ready(mPcm)) {
	LOGE("TinyAudioStreamOut::set() failed: %s\n",
	     ::pcm_get_error(mPcm));
	pcm_close(mPcm);
        return BAD_VALUE;
    }

    mSampleRate = pcmConfig.rate;
    *pRate = pcmConfig.rate;
    *pFormat = AudioSystem::PCM_16_BIT;
    *pChannels = AudioSystem::CHANNEL_OUT_STEREO;

    LOGI("TinyAudioStreamOut::set(): rate %dHz, %d channels",
	 *pRate, *pChannels);

    return NO_ERROR;
}

TinyAudioStreamOut::~TinyAudioStreamOut()
{
    pcm_close(mPcm);
}

ssize_t TinyAudioStreamOut::write(const void* buffer, size_t bytes)
{
    Mutex::Autolock _l(mLock);
    if (!::pcm_is_ready(mPcm)) {
	// This should never happen...
	return BAD_VALUE;
    }

    if (pcm_write(mPcm, buffer, bytes) != 0)
	LOGE("TinyAudioStreamOut::write() failed: %s\n",
	     ::pcm_get_error(mPcm));

    return NO_ERROR;
}

status_t TinyAudioStreamOut::standby()
{
    if (pcm_stop(mPcm) != 0)
	LOGE("standby() failed: %s\n",
	     ::pcm_get_error(mPcm));
    return NO_ERROR;
}

status_t TinyAudioStreamOut::dump(int fd, const Vector<String16>& args)
{
    String8 result;
    ::write(fd, result.string(), result.size());
    return NO_ERROR;
}

status_t TinyAudioStreamOut::setParameters(const String8& keyValuePairs)
{
    AudioParameter param = AudioParameter(keyValuePairs);
    String8 key = String8(AudioParameter::keyRouting);
    status_t status = NO_ERROR;
    int device;
    LOGV("setParameters() %s", keyValuePairs.string());

    if (param.getInt(key, device) == NO_ERROR) {
        mDevice = device;
        param.remove(key);
    }

    if (param.size()) {
        status = BAD_VALUE;
    }
    return status;
}

String8 TinyAudioStreamOut::getParameters(const String8& keys)
{
    AudioParameter param = AudioParameter(keys);
    String8 value;
    String8 key = String8(AudioParameter::keyRouting);

    if (param.get(key, value) == NO_ERROR) {
        param.addInt(key, (int)mDevice);
    }

    LOGV("getParameters() %s", param.toString().string());
    return param.toString();
}

status_t TinyAudioStreamOut::getRenderPosition(uint32_t *dspFrames)
{
    // Don't bother just yet, there are no actual users of this.
    return INVALID_OPERATION;
}

// ----------------------------------------------------------------------------

// record functions
status_t TinyAudioStreamIn::set(
        TinyAudioHardware *hw,
        int *pFormat,
        uint32_t *pChannels,
        uint32_t *pRate,
        AudioSystem::audio_in_acoustics acoustics)
{
    if (pFormat == 0 || pChannels == 0 || pRate == 0) return BAD_VALUE;
    LOGV("TinyAudioStreamIn::set(%p, %d, %d, %d, %u)", hw, fd, *pFormat, *pChannels, *pRate);
    // check values
    if ((*pFormat != format()) ||
        (*pChannels != channels()) ||
        (*pRate != sampleRate())) {
        LOGE("Error opening input channel");
        *pFormat = format();
        *pChannels = channels();
        *pRate = sampleRate();
        return BAD_VALUE;
    }

    mAudioHardware = hw;
    return NO_ERROR;
}

TinyAudioStreamIn::~TinyAudioStreamIn()
{
}

ssize_t TinyAudioStreamIn::read(void* buffer, ssize_t bytes)
{
    AutoMutex lock(mLock);
    return NO_INIT;
}

status_t TinyAudioStreamIn::dump(int fd, const Vector<String16>& args)
{
    String8 result;
    return NO_ERROR;
}

status_t TinyAudioStreamIn::setParameters(const String8& keyValuePairs)
{
    AudioParameter param = AudioParameter(keyValuePairs);
    String8 key = String8(AudioParameter::keyRouting);
    status_t status = NO_ERROR;
    int device;
    LOGV("setParameters() %s", keyValuePairs.string());

    if (param.getInt(key, device) == NO_ERROR) {
        mDevice = device;
        param.remove(key);
    }

    if (param.size()) {
        status = BAD_VALUE;
    }
    return status;
}

String8 TinyAudioStreamIn::getParameters(const String8& keys)
{
    AudioParameter param = AudioParameter(keys);
    String8 value;
    String8 key = String8(AudioParameter::keyRouting);

    if (param.get(key, value) == NO_ERROR) {
        param.addInt(key, (int)mDevice);
    }

    LOGV("getParameters() %s", param.toString().string());
    return param.toString();
}

//------------------------------------------------------------------------------
//  Factory
//------------------------------------------------------------------------------

extern "C" AudioHardwareInterface* createAudioHardware(void) {
    return new TinyAudioHardware();
}

}; // namespace android
