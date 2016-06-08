// JN516x Program CLI.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "windows.h"
/****************************************************************************
 *
 * MODULE:             Jennic Module Programmer
 *
 * COMPONENT:          Main file
 *
 * VERSION:            $Name:  $
 *
 * REVISION:           $Revision: 1.2 $
 *
 * DATED:              $Date: 2009/03/02 13:33:44 $
 *
 * STATUS:             $State: Exp $
 *
 * AUTHOR:             Matt Redfearn
 *
 * DESCRIPTION:
 *
 *
 * LAST MODIFIED BY:   $Author: lmitch $
 *                     $Modtime: $
 *
 ****************************************************************************
 *
 * This software is owned by NXP B.V. and/or its supplier and is protected
 * under applicable copyright laws. All rights are reserved. We grant You,
 * and any third parties, a license to use this software solely and
 * exclusively on NXP products [NXP Microcontrollers such as JN5148, JN5142, JN5139]. 
 * You, and any third parties must reproduce the copyright and warranty notice
 * and any other legend of ownership on each copy or partial copy of the 
 * software.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.

 * Copyright NXP B.V. 2012. All rights reserved
 *
 ***************************************************************************/
#define __STDC__ TRUE

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#if defined POSIX
#include <termios.h>
#include <pthread.h>
#elif defined WIN32
#include <conio.h>
#include <Windows.h>
#endif

//#include <sys/time.h>
//#include <unistd.h>

#include "programmer.h"

#define vDelay(a) usleep(a * 1000)

#ifndef VERSION
#error Version is not defined!
#else
const char *Version = "1.0 (r" VERSION ")";
#endif

typedef struct
{
    const char*     pcFirmwareFile;
    const char*     pcDumpFlashFile;
    teEepromErase   eEepromErase;
    const char*     pcLoadEEPROMFile;
    const char*     pcDumpEEPROMFile;

    char*           pcMAC_Address;
    uint64_t        u64MAC_Address;

    int             iInitialSpeed;
    int             iProgramSpeed;
    int             iVerify;
    int             iVerbosity;
    
    int             iThreadNum;
    int             iThreadTotal;
    tsConnection    sConnection;
#if defined POSIX
    pthread_t       sThread;
#elif defined WIN32
    HANDLE          hThread;
#endif
    uint32_t thread_times;
} tsProgramThreadArgs;

static tsProgramThreadArgs sProgramThreadArgs = 
{
    NULL,
    NULL,
    E_ERASE_EEPROM_ALL,
    NULL,
    NULL,
    NULL,
    0,
    38400,
    1000000,
    0,
    0,
	0,
	0,
    E_CONNECT_SERIAL,
	NULL,
	NULL,
	0,
	NULL
};

#if defined POSIX
static void *pvProgramThread(void* pvData);
#elif defined WIN32
static DWORD dwProgramThread(void* pvData);
#endif

void print_usage_exit(char *argv[])
{
    printf("Usage: %s\n", argv[0]);
    printf("  Arguments:\n");
    printf("    -s --serial        <serial device> Serial device for 15.4 device, e.g. COM1, /dev/ttyS1\n");
    printf("  Options:\n");
    printf("    -V --verbosity     <verbosity>     Verbosity level. Increse/decrease amount of debug information. Default %d.\n", sProgramThreadArgs.iVerbosity);
    printf("    -l --list                          List available devices\n");
    printf("    -I --initialbaud   <rate>          Set initial baud rate\n");
    printf("    -P --programbaud   <rate>          Set programming baud rate\n");
    printf("    -f --loadflash     <filename>      Load device flash with the given firmware file.\n");
    printf("    -F --dumpflash     <filename>      Dump device flash contents into a file.\n");
    printf("       --eepromerase=<full,pdm>        Erase the chip EEPROM. Optional argument to specify pdm only or full EEPROM erase.\n");
    printf("    -e --loadeeprom    <filename>      Load device EEPROM contents from a file.\n");
    printf("    -E --dumpeeprom    <filename>      Dump device EEPROM contents into a file.\n");
    printf("    -v --verify                        Verify image. If specified, verify the image programmedwas loaded correctly.\n");
    printf("    -m --mac           <MAC Address>   Set MAC address of device. If this is not specified, the address is read from flash.\n");
    //exit(EXIT_FAILURE);
}

teStatus cbProgress(void *pvUser, const char *pcTitle, const char *pcText, int iNumSteps, int iProgress)
{
    tsProgramThreadArgs *psArgs = (tsProgramThreadArgs *)pvUser;
    
    if (psArgs->iVerbosity > 0)
    {
#if defined POSIX
        //printf("%c[A", 0x1B);
#endif
        printf("%15s: %s: %3d%%\n", psArgs->sConnection.pcName, pcText, (iProgress * 100) / iNumSteps);
    }
    return E_PRG_OK;
}

teStatus cbConfirm(void *pvUser, const char *pcTitle, const char *pcText)
{
    char c;
#if defined POSIX
    static struct termios sOldt, sNewt;

    tcgetattr( STDIN_FILENO, &sOldt);
    sNewt = sOldt;

    sNewt.c_lflag &= ~(ICANON);          
    tcsetattr(STDIN_FILENO, TCSANOW, &sNewt);
#endif /* POSIX */
    
    printf("--- %s ---\n", pcTitle);
    printf("%s\n\n", pcText);
    printf("Y/N\n");

#if defined POSIX
    c = getc(stdin); 
    tcsetattr( STDIN_FILENO, TCSANOW, &sOldt);
#elif defined WIN32
    c = _getch();
#endif /* POSIX */
    
    if ((c == 'Y') || (c == 'y'))
    {
        return E_PRG_OK;
    }
    return E_PRG_ABORTED;
}






