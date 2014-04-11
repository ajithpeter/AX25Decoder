#ifndef SOUND_H
#define SOUND_H

#include <windows.h>

#define AUDIO_SAMPLE_RATE   44100
#define AUDIO_FRAMES_PER_BUFFER 16384
#define AUDIO_SAMPLE_BUF_LEN 16384
#define AF_GAIN 5.0

extern CRITICAL_SECTION audioBufferLock;

extern void transferAudioArrayToSoundCardFifo(float* samples, int numSamples);

DWORD WINAPI initAudio(LPVOID param);

#endif
