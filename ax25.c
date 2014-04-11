#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <math.h>

#include "sound.h"
#include "ax25.h"

float correlator_array[SAMPLES_PER_BIT];
float mark_correlator_inphase[SAMPLES_PER_BIT];
float mark_correlator_quadrature[SAMPLES_PER_BIT];
float space_correlator_inphase[SAMPLES_PER_BIT];
float space_correlator_quadrature[SAMPLES_PER_BIT];

void initCorrelator()
{
    int i;
    
    printf("Initializing AX.25 Modem Correlators....\n");
    memset(correlator_array, 0, SAMPLES_PER_BIT * sizeof(float));
    
    for (i = 0; i < SAMPLES_PER_BIT; i++)
    {

        mark_correlator_inphase[i] = cos(2.0 * M_PI * FMARK * ((float)i / (float)AUDIO_SAMPLE_RATE));
        mark_correlator_quadrature[i] = sin(2.0 * M_PI * FMARK * ((float)i / (float)AUDIO_SAMPLE_RATE));
        space_correlator_inphase[i] = cos(2.0 * M_PI * FSPACE * ((float)i / (float)AUDIO_SAMPLE_RATE));
        space_correlator_quadrature[i] = sin(2.0 * M_PI * FSPACE * ((float)i / (float)AUDIO_SAMPLE_RATE));
    }
    
}

int numFill = 0;

float correlate(float sample)
{
    int i;
    float retval = 0.0;    
    float mic = 0.0;
    float mqc = 0.0;
    float sic = 0.0;
    float sqc = 0.0;
    
    if (numFill < SAMPLES_PER_BIT)
    {
        correlator_array[numFill++] = sample;
        return 0;
    }
    
    if (numFill == SAMPLES_PER_BIT)
    {
        memmove(correlator_array, correlator_array + 1, (SAMPLES_PER_BIT - 1) * sizeof(float));        
        correlator_array[SAMPLES_PER_BIT - 1] = sample;
    }

    for (i = 0; i < SAMPLES_PER_BIT; i++)
    {
        mic += (mark_correlator_inphase[i] * correlator_array[i]);
        mqc += (mark_correlator_quadrature[i] * correlator_array[i]);
        sic += (space_correlator_inphase[i] * correlator_array[i]);
        sqc += (space_correlator_quadrature[i] * correlator_array[i]);
    }
    mic *= mic;
    mqc *= mqc;        
    sic *= sic;        
    sqc *= sqc;        
   
    retval = (mic + mqc) - (sic + sqc);        
    return (retval > 0) ? 1.0 : -1.0;
}

float old_corr;
float new_corr;
long numSamples = 0;
char oldDataReg = 0;
int bit_clock = (int)(SAMPLES_PER_BIT / 2);
int numOnes = 0;
int preambleDetect = 0;
char buffer[300];
int packCounter = 0;
int skipBit = 0;
int numBits = 0;

unsigned int bitStream = 0;
unsigned char decodeState = 0;
unsigned int bitBuffer = 0x80;
unsigned char packetBuffer[300];
unsigned char* bufPtr;

void displayPacket()
{
    int i;
    int j;
    
    unsigned short crc_check;
    unsigned char dest[8];
    unsigned char src[8];
    unsigned char rptOne[8];
    unsigned char rptTwo[8];
    unsigned char* ptr = packetBuffer;
    
    crc_check = 0xffff;
    
    memset(dest, 0, 8 * sizeof(char));
    memset(src, 0, 8 * sizeof(char));
    memset(rptOne, 0, 8 * sizeof(char));
    memset(rptTwo, 0, 8 * sizeof(char));
    
    for (i = 0; i < (bufPtr - packetBuffer); i++) updateCRC(&crc_check, packetBuffer[i] & 0xff);
    if ((crc_check & 0xffff) == 0xf0b8)
    {
        printf("\n");        
        
        /* Get the destination address */
        for (j = 0; j < 7; j++) dest[j] = *(ptr++) >> 1;
        
        /* Get the source address */
        for (j = 0; j < 7; j++) src[j] = *(ptr++) >> 1;
                
        /* Get the repeater addresses if any */
        if ((packetBuffer[13] & 1) == 0)
        {
            for (j = 0; j < 7; j++) rptOne[j] = *(ptr++) >> 1;

        /* Get the repeater address two, if any */
            if ((packetBuffer[20] & 1) == 0)
            {
                for (j = 0; j < 7; j++) rptTwo[j] = *(ptr++) >> 1;
            }
        }

        /* Get Control Field */
        ptr++;
        /* Get PID field */
        ptr++;
        /* Payload */
        
        for (j = 0; j < 6; j++) if (src[j] != ' ') printf("%c", src[j]);
        if (strlen(rptOne) > 0)
        {
            printf("/");
            for (j = 0; j < 6; j++) if (rptOne[j] != ' ') printf("%c", rptOne[j]);
            if (strlen(rptTwo) > 0)
            {
                printf("/");
                for (j = 0; j < 6; j++) if (rptTwo[j] != ' ') printf("%c", rptTwo[j]);
            }
        }
        printf(">");
        for (j = 0; j < 6; j++) if (dest[j] != ' ') printf("%c", dest[j]);
        printf("\n");
        while (ptr < bufPtr - 2) printf("%c", *(ptr++));
        printf("\n");
    }
}

void processBit(int bit)
{
	bitStream <<= 1;
	bitStream |= !!bit;
	
	if ((bitStream & 0xff) == 0x7e) 
    {
        if ((bufPtr - packetBuffer) > 2)
        {
            displayPacket();
        }        
       
        memset(packetBuffer, 0, 300 * sizeof(char));
        bufPtr = packetBuffer;
        decodeState = 1;
        bitBuffer = 0x80;
		return;
	}
	
	if ((bitStream & 0x7f) == 0x7f) {
		decodeState = 0;
		return;
	}
	
	if (!decodeState)
		return;
		
	if ((bitStream & 0x3f) == 0x3e) /* stuffed bit */
		return;
		
	if (bitStream & 1)
		bitBuffer |= 0x100;
		
	if (bitBuffer & 1) 
    {
        bitBuffer >>= 1;
        *bufPtr = (unsigned char)(bitBuffer & 0xff);
        bufPtr++;
        bitBuffer = 0x80;
		return;
	}

  	bitBuffer >>= 1;
}

void processAudioSample(float sample)
{
    old_corr = new_corr;
    new_corr = correlate(sample);

    if (numFill == SAMPLES_PER_BIT)
    {
        numSamples++;

        if (numSamples > 2)
        {
            if (old_corr != new_corr) bit_clock = (int)(SAMPLES_PER_BIT / 2);
        }
        
        bit_clock = bit_clock + 1;

        if ((bit_clock % SAMPLES_PER_BIT) == 0)
        {
            bit_clock = 0;
            if (oldDataReg == 0)
            {
                oldDataReg = new_corr; 
                return;
            }
            if (oldDataReg == new_corr) processBit(1); else processBit(0);
            oldDataReg = new_corr;
        }    
    }
}
