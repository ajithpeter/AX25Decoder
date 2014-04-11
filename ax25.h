#ifndef AX25_H
#define AX25_H

#include "sound.h"

#define FMARK 1200.0
#define FSPACE 2200.0
#define BAUD 1200

#define SAMPLES_PER_BIT ((int)(AUDIO_SAMPLE_RATE / BAUD))
#define BITS_PER_SAMPLE ((1.0 / (float) SAMPLES_PER_BIT))

void initCorrelator(void);
void processAudioSample(float sample);

#endif
