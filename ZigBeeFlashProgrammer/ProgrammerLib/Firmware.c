/****************************************************************************
 *
 * MODULE:             Firmware
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

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include <stdio.h>
//#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
//#include <unistd.h>

#ifdef POSIX
#include <arpa/inet.h>
#include <sys/mman.h>
#elif defined WIN32
#include <Winsock2.h>
#endif

#include "windows.h"
#include "programmer.h"

#include "programmer_private.h"
//#include "dbg.h"

unsigned char binary_FlashProgrammerExtension_JN5168_bin_start;
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

#ifdef DEBUG_FIRMWARE
#define TRACE_FIRMWARE      TRUE
#else
#define TRACE_FIRMWARE      FALSE
#endif

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

#if defined POSIX
typedef struct
{
    int         iFirmwareFD;
    uint32_t    u32FileSize;
    uint8_t*    pu8Firmware;
} tsFwPrivate;

#elif defined WIN32
typedef struct
{
    HANDLE      hFile;
    HANDLE      hMapping;

    DWORD       dwFileSize;
    
    uint8_t*    pu8Firmware;
} tsFwPrivate;
#endif

typedef struct
{
    uint8_t     u8ConfigByte0;
    uint8_t     u8ConfigByte1;
    uint16_t    u16SpiScrambleIndex;
    uint32_t    u32TextStartAddress;
    uint32_t    u32TextLength;
    uint32_t    u32ROMVersion;
    uint32_t    u32Unused1;
    uint32_t    u32BssStartAddress;
    uint32_t    u32BssLength;
    uint32_t    u32WakeUpEntryPoint;
    uint32_t    u32ResetEntryPoint;
    uint8_t     au8OadData[12];
    uint8_t     u8TextDataStart;
}tsBL_BinHeaderV1;


typedef struct
{
    uint32_t    u32ROMVersion;
    uint32_t    au32BootImageRecord[4];
    uint32_t    au32MacAddress[2];
    uint32_t    au32EncryptionInitialisationVector[4];
    uint32_t    u32DataSectionInfo;
    uint32_t    u32BssSectionInfo;
    uint32_t    u32WakeUpEntryPoint;
    uint32_t    u32ResetEntryPoint;
    uint8_t     u8TextDataStart;
}tsBL_BinHeaderV2;


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


#if defined POSIX
teStatus ePRG_FwOpen(tsPRG_Context *psContext, char *pcFirmwareFile)
{
    tsPRG_PrivateContext *psPriv;    
    tsFwPrivate *psFwPriv;
    struct stat sb;
    
    if ((!psContext) || (!pcFirmwareFile))
    {
        return E_PRG_NULL_PARAMETER;
    }
    psPriv = (tsPRG_PrivateContext *)psContext->pvPrivate;
    
    psFwPriv = (tsFwPrivate *)psPriv->pvFwPrivate;
    
    if (psFwPriv)
    {
        if (ePRG_FwClose(psContext) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, E_PRG_ERROR, "closing existing file");
        }
    }
    else
    {
        psFwPriv = malloc(sizeof(tsFwPrivate));
        if (!psFwPriv)
        {
            return E_PRG_OUT_OF_MEMORY;
        }
        psPriv->pvFwPrivate = psFwPriv;
    }           
    
    psFwPriv->iFirmwareFD = open(pcFirmwareFile, O_RDONLY);
    if (psFwPriv->iFirmwareFD < 0)
    {
        return ePRG_SetStatus(psContext, E_PRG_FAILED_TO_OPEN_FILE, "\"%s\" (%s)", pcFirmwareFile, pcPRG_GetLastErrorMessage(psContext));
    }
    
    //DBG_vPrintf(TRACE_FIRMWARE, "opened FD %d\n", psFwPriv->iFirmwareFD);
    
    if (fstat(psFwPriv->iFirmwareFD, &sb) == -1)
    {
        return ePRG_SetStatus(psContext, E_PRG_ERROR, "(%s)", pcPRG_GetLastErrorMessage(psContext));
    }
    
    psFwPriv->u32FileSize = (uint32_t)sb.st_size;
    
    psContext->sFirmwareInfo.u32ImageLength = psFwPriv->u32FileSize;
    
    /* Copy-on-write, changes are not written to the underlying file. */
    psFwPriv->pu8Firmware = mmap(NULL, psFwPriv->u32FileSize, PROT_READ | PROT_WRITE, MAP_PRIVATE, psFwPriv->iFirmwareFD, 0);
    //DBG_vPrintf(TRACE_FIRMWARE, "mapped file at %p\n", psFwPriv->pu8Firmware);
    
    if (psFwPriv->pu8Firmware == MAP_FAILED)
    {
        return ePRG_SetStatus(psContext, E_PRG_ERROR, "(%s)", pcPRG_GetLastErrorMessage(psContext));
    }
    
    return ePRG_FwGetInfo(psContext, psFwPriv->pu8Firmware);
}


teStatus ePRG_FwClose(tsPRG_Context *psContext)
{
    tsPRG_PrivateContext *psPriv;    
    tsFwPrivate *psFwPriv;
    
    if (!psContext)
    {
        return E_PRG_NULL_PARAMETER;
    }
    psPriv = (tsPRG_PrivateContext *)psContext->pvPrivate;
    
    psFwPriv = (tsFwPrivate *)psPriv->pvFwPrivate;
    
    if (psFwPriv)
    {
        if (psFwPriv->pu8Firmware)
        {
            //DBG_vPrintf(TRACE_FIRMWARE, "unmapping file at %p\n", psFwPriv->pu8Firmware);
            munmap(psFwPriv->pu8Firmware, psFwPriv->u32FileSize);
        }

        if (psFwPriv->iFirmwareFD)
        {
            //DBG_vPrintf(TRACE_FIRMWARE, "closing FD %d\n", psFwPriv->iFirmwareFD);
            close(psFwPriv->iFirmwareFD);
        }
        
        free(psFwPriv);
        psPriv->pvFwPrivate = NULL;
    }
        
    return ePRG_SetStatus(psContext, E_PRG_OK, "");
}

#elif defined WIN32
teStatus ePRG_FwOpen(tsPRG_Context *psContext, char *pcFirmwareFile)
{
    tsPRG_PrivateContext *psPriv;    
    tsFwPrivate *psFwPriv;

    if ((!psContext) || (!pcFirmwareFile))
    {
        return E_PRG_NULL_PARAMETER;
    }
    psPriv = (tsPRG_PrivateContext *)psContext->pvPrivate;
 
    //DBG_vPrintf(TRACE_FIRMWARE, "Opening file name \"%s\"\n", pcFirmwareFile);
    
    if (psPriv->pvFwPrivate)
    {
        //DBG_vPrintf(TRACE_FIRMWARE, "Closing existing file\n");
        if (ePRG_FwClose(psContext) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, E_PRG_ERROR, "closing existing file");
        }
    }
    
    psFwPriv = malloc(sizeof(tsFwPrivate));
    if (!psFwPriv)
    {
        return E_PRG_OUT_OF_MEMORY;
    }
    psPriv->pvFwPrivate = psFwPriv;

    //DBG_vPrintf(TRACE_FIRMWARE, "Opening file name \"%s\"\n", pcFirmwareFile);
    
    psFwPriv->hFile = CreateFile(pcFirmwareFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (psFwPriv->hFile == INVALID_HANDLE_VALUE)
    {
        return ePRG_SetStatus(psContext, E_PRG_FAILED_TO_OPEN_FILE, "\"%s\" (%s)", pcFirmwareFile, pcPRG_GetLastErrorMessage(psContext));
    }
    
    psFwPriv->dwFileSize = GetFileSize(psFwPriv->hFile, NULL);
    
    if (psFwPriv->dwFileSize == INVALID_FILE_SIZE)
    {
        return ePRG_SetStatus(psContext, E_PRG_ERROR, "(%d)", pcPRG_GetLastErrorMessage(psContext));
    }
    
    psContext->sFirmwareInfo.u32ImageLength = (uint32_t)psFwPriv->dwFileSize;
    
    //DBG_vPrintf(TRACE_FIRMWARE, "Opened file size %d\n", psFwPriv->dwFileSize);
    
    psFwPriv->hMapping = CreateFileMapping(psFwPriv->hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    
    if (psFwPriv->hMapping == INVALID_HANDLE_VALUE)
    {
        return ePRG_SetStatus(psContext, E_PRG_ERROR, "(%s)", pcPRG_GetLastErrorMessage(psContext));
    }
    
    psFwPriv->pu8Firmware = MapViewOfFile(psFwPriv->hMapping, FILE_MAP_COPY, 0, 0, psFwPriv->dwFileSize);
    
    if (!psFwPriv->pu8Firmware)
    {
        return ePRG_SetStatus(psContext, E_PRG_ERROR, "(%s)", pcPRG_GetLastErrorMessage(psContext));
    }

    return ePRG_FwGetInfo(psContext, psFwPriv->pu8Firmware);
}


teStatus ePRG_FwClose(tsPRG_Context *psContext)
{
    tsPRG_PrivateContext *psPriv;    
    tsFwPrivate *psFwPriv;
    
    if (!psContext)
    {
        return E_PRG_NULL_PARAMETER;
    }
    psPriv = (tsPRG_PrivateContext *)psContext->pvPrivate;
    
    psFwPriv = (tsFwPrivate *)psPriv->pvFwPrivate;
    
    if (psFwPriv)
    {
        if (psFwPriv->pu8Firmware)
        {
            //DBG_vPrintf(TRACE_FIRMWARE, "unmapping file at %p\n", psFwPriv->pu8Firmware);
            UnmapViewOfFile(psFwPriv->pu8Firmware);
        }
        
        if (psFwPriv->hMapping != INVALID_HANDLE_VALUE)
        {
            //DBG_vPrintf(TRACE_FIRMWARE, "Closing mapping handle\n");
            CloseHandle(psFwPriv->hMapping);
        }

        if (psFwPriv->hFile)
        {
            //DBG_vPrintf(TRACE_FIRMWARE, "Closing file handle\n");
            CloseHandle(psFwPriv->hFile);
        }
        
        free(psFwPriv);
        psPriv->pvFwPrivate = NULL;
    }
        
    return ePRG_SetStatus(psContext, E_PRG_OK, "");
}
#endif /* POSIX */


teStatus ePRG_FwGetInfo(tsPRG_Context *psContext, uint8_t *pu8Firmware)
{
    tsBL_BinHeaderV2 *psHeader;
    
    if ((!psContext) || (!pu8Firmware))
    {
        return E_PRG_NULL_PARAMETER;
    }
    
    //DBG_vPrintf(TRACE_FIRMWARE, "Getting info on file at 0x%p\n", pu8Firmware);
    
    psHeader = (tsBL_BinHeaderV2 *)pu8Firmware;
	
    // JN5148-J01 onwards uses multiimage bootloader - check for it's magic number.   
    if ((ntohl(psHeader->au32BootImageRecord[0]) == 0x12345678) &&
        (ntohl(psHeader->au32BootImageRecord[1]) == 0x11223344) &&
        (ntohl(psHeader->au32BootImageRecord[2]) == 0x55667788))
    {
        psContext->sFirmwareInfo.u32ROMVersion                = ntohl(psHeader->u32ROMVersion);
        
        psContext->sFirmwareInfo.u32TextSectionLoadAddress    = 0x04000000 + (((ntohl(psHeader->u32DataSectionInfo)) >> 16) * 4);
        psContext->sFirmwareInfo.u32TextSectionLength         = (((ntohl(psHeader->u32DataSectionInfo)) & 0x0000FFFF) * 4);
        psContext->sFirmwareInfo.u32BssSectionLoadAddress     = 0x04000000 + (((ntohl(psHeader->u32BssSectionInfo)) >> 16) * 4);
        psContext->sFirmwareInfo.u32BssSectionLength          = (((ntohl(psHeader->u32BssSectionInfo)) & 0x0000FFFF) * 4);

        psContext->sFirmwareInfo.u32ResetEntryPoint           = ntohl(psHeader->u32ResetEntryPoint);
        psContext->sFirmwareInfo.u32WakeUpEntryPoint          = ntohl(psHeader->u32WakeUpEntryPoint);
        
        /* Pointer to and length of image for flash */
        psContext->sFirmwareInfo.pu8ImageData                 = (uint8_t*)&(psHeader->au32BootImageRecord[0]);
        
        psContext->sFirmwareInfo.u32MacAddressLocation        = 0x10;
        
        /* Pointer to text section in image for RAM */
        psContext->sFirmwareInfo.pu8TextData                  = &(psHeader->u8TextDataStart);

        //DBG_vPrintf(TRACE_FIRMWARE, "Header size:           %d\n", sizeof(tsBL_BinHeaderV2));

        //DBG_vPrintf(TRACE_FIRMWARE, "u32ROMVersion:         0x%08x\n", psContext->sFirmwareInfo.u32ROMVersion);
        //DBG_vPrintf(TRACE_FIRMWARE, "u32DataSectionInfo:    0x%08x\n", ntohl(psHeader->u32DataSectionInfo));
        //DBG_vPrintf(TRACE_FIRMWARE, "u32TextSectionLength:  0x%08x\n", (((ntohl(psHeader->u32DataSectionInfo)) & 0x0000FFFF) * 4));
    }
    else
    {
        tsBL_BinHeaderV1 *psHeader              = (tsBL_BinHeaderV1 *)pu8Firmware;

        psContext->sFirmwareInfo.u32ROMVersion                = ntohl(psHeader->u32ROMVersion);
        
        psContext->sFirmwareInfo.u32TextSectionLoadAddress    = ntohl(psHeader->u32TextStartAddress);
        psContext->sFirmwareInfo.u32TextSectionLength         = ntohl(psHeader->u32TextLength);
        psContext->sFirmwareInfo.u32BssSectionLoadAddress     = ntohl(psHeader->u32BssStartAddress);
        psContext->sFirmwareInfo.u32BssSectionLength          = ntohl(psHeader->u32BssLength);
        
        psContext->sFirmwareInfo.u32ResetEntryPoint           = psHeader->u32ResetEntryPoint;
        psContext->sFirmwareInfo.u32WakeUpEntryPoint          = psHeader->u32WakeUpEntryPoint;
        
        /* Pointer to and length of image for flash */
        psContext->sFirmwareInfo.pu8ImageData                 = &(psHeader->u8ConfigByte0);
        //psContext->sFirmwareInfo.u32ImageLength               = sizeof(tsBL_BinHeaderV1) + psContext->sFirmwareInfo.u32TextSectionLength;
        
        psContext->sFirmwareInfo.u32MacAddressLocation        = 0x30;
        
        /* Pointer to text section in image for RAM */
        psContext->sFirmwareInfo.pu8TextData                  = &(psHeader->u8TextDataStart);
        
    }
    
    return ePRG_SetStatus(psContext, E_PRG_OK, "");
}


/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/

