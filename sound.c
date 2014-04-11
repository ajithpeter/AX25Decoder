#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "portaudio.h"
#include "sound.h"
#include "ax25.h"


CRITICAL_SECTION audioBufferLock;
extern BOOL exitLoop;

typedef struct Sample_Queue SampleQueue;
struct Sample_Queue
{
    float* buffer;
    int numElements;
    struct Sample_Queue* next;
};

SampleQueue* sampleQueue = NULL;

void transferAudioArrayToSoundCardFifo(float* samples, int numSamples)
{
    int i;
    SampleQueue* tmp = sampleQueue;
    EnterCriticalSection(&audioBufferLock);

    if (tmp == NULL)
    {
        
        sampleQueue = (SampleQueue*)malloc(1 * sizeof(SampleQueue));
        sampleQueue->buffer = (float*)malloc(numSamples * sizeof(float));
        memcpy(sampleQueue->buffer, samples, numSamples * sizeof(float));
        sampleQueue->numElements = numSamples;
        sampleQueue->next = NULL;
    }
    else
    {
        while (tmp->next != NULL) tmp = tmp->next;
        tmp = (SampleQueue*)malloc(1 * sizeof(SampleQueue));
        tmp->buffer = (float*)malloc(numSamples * sizeof(float));
        memcpy(tmp->buffer, samples, numSamples * sizeof(float));
        tmp->numElements = numSamples;
        tmp->next = NULL;
    }
    LeaveCriticalSection(&audioBufferLock);    
    return;
}

void processAudioBuffer(float sample)
{
}

void popAudioBuffer(float** AudioBuffer)
{
    SampleQueue* tmp = sampleQueue;
    float tmpBuffer[AUDIO_SAMPLE_BUF_LEN];
    int i;
    if (sampleQueue == NULL)
    {
        for (i = 0; i < 2 * AUDIO_SAMPLE_BUF_LEN; i += 2)
        {
            (*AudioBuffer)[i] = 0.0;
            (*AudioBuffer)[i+1] = 0.0;
        }
        return;
    }
    EnterCriticalSection(&audioBufferLock);
    memcpy(tmpBuffer, sampleQueue->buffer, AUDIO_SAMPLE_BUF_LEN * sizeof(float));
    sampleQueue = sampleQueue->next;
    free(tmp->buffer);
    free(tmp);
    LeaveCriticalSection(&audioBufferLock);
    for (i = 0; i < 2 * AUDIO_SAMPLE_BUF_LEN; i += 2)
    {
        (*AudioBuffer)[i] = AF_GAIN * tmpBuffer[i/2];
        (*AudioBuffer)[i+1] = AF_GAIN * tmpBuffer[i/2];
        processAudioSample(AF_GAIN * tmpBuffer[i/2]);
    }
}

static int paCallback( const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{
    int i;
    float* input = (float*)inputBuffer;
    float* output = (float*)outputBuffer;
    
    for (i = 0; i < 2 * framesPerBuffer; i += 2)
    {
        output[i] = input[i];
        output[i+1] = input[i+1];
        processAudioSample(input[i]);
    }
    return 0;
}

void displayAudioDeviceIndices()
{
    int i;
    const PaDeviceInfo* info;
    for (i = 0; i < Pa_GetDeviceCount(); i++)
    {
        info = Pa_GetDeviceInfo(i);
        printf("%d. %s (Input: %d, Output:%d)\n", i, info->name, info->maxInputChannels, info->maxOutputChannels);
    }
}

DWORD WINAPI initAudio(LPVOID param)
{
    PaStreamParameters inputParameters;
    PaStreamParameters outputParameters;
    PaStream* stream;
    PaError err;

    InitializeCriticalSectionAndSpinCount(&audioBufferLock, 0x00000400);

    initCorrelator();


    err = Pa_Initialize();
    if (err != paNoError)
    {
        MessageBox(NULL, TEXT("Unable to Initialize Sound, Exitting"), TEXT("FM Receiver"), MB_ICONERROR | MB_OK);
        exit(-1);
    }

    /* Display Audio Output Devices Available */
    displayAudioDeviceIndices();
    
    
    inputParameters.device = 2;
    inputParameters.channelCount = 2;
    inputParameters.sampleFormat = paFloat32;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;    

    outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
    if (outputParameters.device == paNoDevice) 
    {
        fprintf(stderr,"Error: No default output device.\n");
        exit(-1);
    }
    outputParameters.channelCount = 2;       /* stereo output */
    outputParameters.sampleFormat = paFloat32; /* 32 bit floating point output */
    outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;
    err = Pa_OpenStream(&stream, &inputParameters, &outputParameters, AUDIO_SAMPLE_RATE, AUDIO_FRAMES_PER_BUFFER, paNoFlag, paCallback, NULL);

    if( err != paNoError )
    {
        fprintf(stderr,"Error opening stream...\n");
        exit(-1);
    }
    err = Pa_StartStream( stream );
    if( err != paNoError ) 
    {
        fprintf(stderr,"Error starting stream...\n");
        exit(-1);
    }

    while (exitLoop == FALSE)
    {
        Sleep(10);
        //SwitchToThread();
    }
    
    Pa_StopStream(stream);
    Pa_Terminate();
    DeleteCriticalSection(&audioBufferLock);
    printf("Audio Loop exitted....\n");
    fflush(stdout);
}
