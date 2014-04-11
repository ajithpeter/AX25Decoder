#ifndef RTL_H
#define RTL_H

#include "sound.h"
#include <windows.h>

#define CENTER_FREQUENCY 144600000
#define RTL_SAMPLE_RATE 176400 * 6
//#define RTL_SAMPLE_RATE (44100 * 10)
#define DECIMATE (RTL_SAMPLE_RATE / AUDIO_SAMPLE_RATE)

DWORD WINAPI initRTLSDR(LPVOID);

#endif
