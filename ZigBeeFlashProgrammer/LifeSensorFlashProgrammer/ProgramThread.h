/****************************************************************************
 *
 * MODULE:             ProrgamThread.h
 *
 * COMPONENT:          $RCSfile: Firmware.c,v $
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

#ifndef  PROGRAMTHREAD_H_INCLUDED
#define  PROGRAMTHREAD_H_INCLUDED

#if defined __cplusplus
extern "C" {
#endif

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include <stdint.h>

//#include "Utils.h"
#include "programmer.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
#if 0
/** Structure defining all operations ang arguments */
typedef struct
{
    tsConnection    sConnection;
    tsUtilsThread   sThread;

    const char*     pcFirmwareFile;
    const char*     pcDumpFlashFile;
    const char*     pcLoadEEPROMFile;
    const char*     pcDumpEEPROMFile;
    const char*     pcLoadRamFile;

    uint8_t*        au8MacAddress;

    uint32_t*       pu32AesKeyGet;
    uint32_t*       pu32AesKeySet;

    uint32_t*       apu32UserDataGet[3];    /**< 3 lots of 128 bits of user data for loading from index sector */
    uint32_t*       apu32UserDataSet[3];    /**< 3 lots of 128 bits of user data for programming into index sector */

    tsDeviceConfig* psDeviceConfigGet;
    tsDeviceConfig* psDeviceConfigSet;

    teEepromErase   eEepromErase;
    int             iInitialSpeed;
    int             iProgramSpeed;
    int             iVerify;
    int             iVerbosity;
    int             iForce;

    uint32_t        u32FlashOffset;
    uint32_t        u32EepromOffset;

    uint32_t        u32FlashIndex;
    uint32_t        u32ConnectionNum;
    teStatus        eStatus;
} tsProgramThreadArgs;
#endif

class LifeSensorFlashProgrammerDlg;
class DownloadPortCtl;
typedef struct {
    const char*     pcFirmwareFile;
    const char*     pcDumpFlashFile;
    teEepromErase   eEepromErase;
    const char*     pcLoadEEPROMFile;
    uint8_t*        au8MacAddress;
    int             iInitialSpeed;
    int             iProgramSpeed;
    int             iVerify;
    int             iVerbosity;
    tsConnection    sConnection;
    HANDLE          hThread;
    LifeSensorFlashProgrammerDlg  *hProgrammerDlg;
    int             bWriteMACAddress;

    uint32_t        u32FlashOffset;
    uint32_t        u32EepromOffset;

    uint32_t        u32FlashIndex;
    uint32_t        u32ConnectionNum;
    teStatus        eStatus;
    uint32_t*       pu32AesKeyGet;
    uint32_t*       pu32AesKeySet;

    uint32_t*       apu32UserDataGet[3];    /**< 3 lots of 128 bits of user data for loading from index sector */
    uint32_t*       apu32UserDataSet[3];    /**< 3 lots of 128 bits of user data for programming into index sector */
    tsDeviceConfig* psDeviceConfigGet;
    tsDeviceConfig* psDeviceConfigSet;
    DownloadPortCtl *hPortCtl;
} tsProgramThreadArgs;


/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

//void *pvProgramThread(tsUtilsThread *psThreadInfo);

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

#if defined __cplusplus
}
#endif

#endif  /* PROGRAMTHREAD_H_INCLUDED */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/



