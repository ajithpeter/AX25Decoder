#include <stdio.h>
#include <windows.h>

#include "rtl.h"
#include "sound.h"

BOOL exitLoop = FALSE;

int main(int argc, char** argv)
{
    DWORD tidSound;
    DWORD tidRadio;
    HANDLE threadRadio;
    HANDLE threadSound;
    
    exitLoop = FALSE;
    
    //threadRadio = CreateThread(NULL, 0, initRTLSDR, NULL, 0, &tidRadio);
    threadSound = CreateThread(NULL, 0, initAudio, NULL, 0, &tidSound);
    
    getch();
    exitLoop = TRUE;
    WaitForSingleObject(threadRadio, INFINITE);
    WaitForSingleObject(threadSound, INFINITE);
    return 0;
}
