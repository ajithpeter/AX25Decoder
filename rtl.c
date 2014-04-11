#include <stdio.h>
#include <math.h>
#include <windows.h>

#include "rtl-sdr.h"
#include "rtl.h"
#include "sound.h"
#include "SampleFilter.h"

SampleFilter f;

float previous_inPhase = 0.0;
float previous_quadrature = 0.0;
float audioSampleArray[AUDIO_SAMPLE_BUF_LEN];
int audioSampleArrayCount = 0;
rtlsdr_dev_t *dev;
extern BOOL exitLoop;

void multComplex(float xr, float xi, float yr, float yi, float *zr, float *zi)
{
    *zr = xr*yr - xi*yi;
	*zi = xi*yr + xr*yi;
}

int count = 0;

static void rtlsdr_callback(unsigned char *buf, uint32_t len, void *ctx)
{
    int i;
    float inPhase;
    float quadrature;
    float discReal;
    float discImag;
    float audioSample;
    float limitModulus;
 
    for (i = 0; i < (len - 1); i+= 2)
    {
        inPhase = ((float)buf[i] - 128.0) / (float)255.0;
        quadrature = ((float)buf[i + 1] - 128.0) / (float)255.0;
        multComplex(inPhase, quadrature, previous_inPhase, -1.0 * previous_quadrature, &discReal, &discImag);
        limitModulus = sqrt(discReal * discReal + discImag * discImag);
        discReal /= limitModulus;
        discImag /= limitModulus;
        audioSample = atan2(discImag, discReal) / (2.0 * M_PI);
        
        SampleFilter_put(&f, audioSample);
                       
        count++;
        if (count == DECIMATE)
        {
            count = 0;     

            audioSampleArray[audioSampleArrayCount] = SampleFilter_get(&f);
            audioSampleArrayCount++;
    
            if (audioSampleArrayCount == AUDIO_SAMPLE_BUF_LEN)
            {
                transferAudioArrayToSoundCardFifo(audioSampleArray, AUDIO_SAMPLE_BUF_LEN);
                audioSampleArrayCount = 0;
            }
        }        
        previous_inPhase = inPhase;
        previous_quadrature = quadrature;
    }
    
    if (exitLoop == TRUE) rtlsdr_cancel_async(dev);
}



DWORD WINAPI initRTLSDR(LPVOID param)
{
    int numDevices;
    int index;
    int i;
        
    numDevices = rtlsdr_get_device_count();
    if (numDevices == 0) return;
    
    SampleFilter_init(&f);
    
    index = 0;
    rtlsdr_open(&dev, index);
    rtlsdr_set_center_freq(dev, CENTER_FREQUENCY);
    rtlsdr_set_sample_rate(dev, RTL_SAMPLE_RATE);
    rtlsdr_set_offset_tuning(dev, 1);
    rtlsdr_set_tuner_gain_mode(dev, 1);
    rtlsdr_set_tuner_gain(dev, 490);
    rtlsdr_set_agc_mode(dev, 1);
    rtlsdr_reset_buffer(dev);
    rtlsdr_read_async(dev, rtlsdr_callback, NULL, 32, 65536);

    /* This code is executed after the callback issues a rtlsdr_cancel_async */
    rtlsdr_close(dev);
    printf("Radio Loop exit...\n");
    fflush(stdout);
}
