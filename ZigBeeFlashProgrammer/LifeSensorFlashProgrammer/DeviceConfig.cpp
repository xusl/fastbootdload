/****************************************************************************
 *
 * MODULE:             UI.h
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
#include <stdafx.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

#include "DeviceConfig.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/


/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/** Mapping of device configuration options in human readable form to offsets / values in the device configuration structure */
const static struct
{
    const char *pcOption;
    size_t      iOffset;
    int         iValue;
} asDeviceConfigOptions[] = 
{
    {   "JTAG_DEFAULT",                         offsetof(tsDeviceConfig, eJtag),                    E_DC_JTAG_DEFAULT },
    {   "JTAG_ENABLE",                          offsetof(tsDeviceConfig, eJtag),                    E_DC_JTAG_ENABLE },
    {   "JTAG_DISABLE_FLASH",                   offsetof(tsDeviceConfig, eJtag),                    E_DC_JTAG_DISABLE_FLASH },
    {   "JTAG_DISABLE_CPU",                     offsetof(tsDeviceConfig, eJtag),                    E_DC_JTAG_DISABLE_CPU },
    {   "JTAG_DISABLE",                         offsetof(tsDeviceConfig, eJtag),                    E_DC_JTAG_DISABLE },
    
    {   "VBO_DEFAULT",                          offsetof(tsDeviceConfig, eVbo),                     E_DC_VBO_DEFAULT },
    {   "VBO_195",                              offsetof(tsDeviceConfig, eVbo),                     E_DC_VBO_195 },
    {   "VBO_200",                              offsetof(tsDeviceConfig, eVbo),                     E_DC_VBO_200 },
    {   "VBO_210",                              offsetof(tsDeviceConfig, eVbo),                     E_DC_VBO_210 },
    {   "VBO_220",                              offsetof(tsDeviceConfig, eVbo),                     E_DC_VBO_220 },
    {   "VBO_230",                              offsetof(tsDeviceConfig, eVbo),                     E_DC_VBO_230 },
    {   "VBO_240",                              offsetof(tsDeviceConfig, eVbo),                     E_DC_VBO_240 },
    {   "VBO_270",                              offsetof(tsDeviceConfig, eVbo),                     E_DC_VBO_270 },
    {   "VBO_300",                              offsetof(tsDeviceConfig, eVbo),                     E_DC_VBO_300 },
    
    {   "CRP_DEFAULT",                          offsetof(tsDeviceConfig, eCRP),                     E_DC_CRP_DEFAULT },
    {   "CRP_LEVEL0",                           offsetof(tsDeviceConfig, eCRP),                     E_DC_CRP_LEVEL0 },
    {   "CRP_LEVEL1",                           offsetof(tsDeviceConfig, eCRP),                     E_DC_CRP_LEVEL1 },
    {   "CRP_LEVEL2",                           offsetof(tsDeviceConfig, eCRP),                     E_DC_CRP_LEVEL2 },
    
    {   "EXTERNAL_FLASH_ENCRYPTION_DEFAULT",    offsetof(tsDeviceConfig, eExternalFlashEncrypted),  E_DC_EXTERNAL_FLASH_ENCRYPTED_DEFAULT },
    {   "EXTERNAL_FLASH_ENCRYPTED",             offsetof(tsDeviceConfig, eExternalFlashEncrypted),  E_DC_EXTERNAL_FLASH_ENCRYPTED_TRUE },
    {   "EXTERNAL_FLASH_NOT_ENCRYPTED",         offsetof(tsDeviceConfig, eExternalFlashEncrypted),  E_DC_EXTERNAL_FLASH_ENCRYPTED_FALSE },
    
    {   "EXTERNAL_FLASH_LOAD_DEFAULT",          offsetof(tsDeviceConfig, eExternalFlashLoad),       E_DC_EXTERNAL_FLASH_LOAD_DEFAULT },
    {   "EXTERNAL_FLASH_LOAD_DISABLE",          offsetof(tsDeviceConfig, eExternalFlashLoad),       E_DC_EXTERNAL_FLASH_LOAD_DISABLE },
    {   "EXTERNAL_FLASH_LOAD_ENABLE",           offsetof(tsDeviceConfig, eExternalFlashLoad),       E_DC_EXTERNAL_FLASH_LOAD_ENABLE },
};


/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/


teStatus eDeviceConfigFromString(char *pcOptionString, tsDeviceConfig *psConfig)
{
    const char *pcSeperators = ",";
    char *pcToken;

    pcToken = strtok (pcOptionString, pcSeperators);
    while (pcToken != NULL)
    {
        int i;
        for (i = 0; i < (sizeof(asDeviceConfigOptions) / sizeof(asDeviceConfigOptions[0])); i++)
        {
            if (strcmp(asDeviceConfigOptions[i].pcOption, pcToken) == 0)
            {
                *((char *)(psConfig) + asDeviceConfigOptions[i].iOffset) = asDeviceConfigOptions[i].iValue;
                break;
            }
        }
        if (i >= sizeof(asDeviceConfigOptions) / sizeof(asDeviceConfigOptions[0]))
        {
            fprintf(stderr, "Unknown device configuration option %s\n", pcToken);
            return E_PRG_ERROR;
        }
        
        pcToken = strtok (NULL, pcSeperators);
    }
    
    return E_PRG_OK;
}


teStatus eDeviceConfigToString(char **ppcOptionString, tsDeviceConfig *psConfig)
{
    int i;
    
    *ppcOptionString = (char *)malloc(4096);
    if (!(*ppcOptionString))
    {
        fprintf(stderr, "Memory allocation error\n");
        return E_PRG_OUT_OF_MEMORY;
    }
    (*ppcOptionString)[0] = '\0';
    
    for (i = 0; i < (sizeof(asDeviceConfigOptions) / sizeof(asDeviceConfigOptions[0])); i++)
    {
        if (*((char *)(psConfig) + asDeviceConfigOptions[i].iOffset) == asDeviceConfigOptions[i].iValue)
        {
            strcat(*ppcOptionString, asDeviceConfigOptions[i].pcOption);
            strcat(*ppcOptionString, ",");
        }
    }
     /* Remove trailing comma */
    (*ppcOptionString)[strlen(*ppcOptionString)-1] = '\0';
    return E_PRG_OK;
}


/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/


/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
