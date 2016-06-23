
/****************************************************************************
 *
 * MODULE:             Jennic Module Programmer
 *
 * COMPONENT:          Serial port handling
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
//#include <unistd.h>
//#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "log.h"

#if defined WIN32
#include <Windows.h>
#endif /* WIN32 */

#include "programmer.h"

#include "programmer_private.h"
#include "ChipID.h"
#include "JN51xx_BootLoader.h"
#include "uart.h"
//#include "dbg.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

#ifdef DEBUG_PROGRAMMER
#define TRACE_PROGRAMMER    TRUE
#else
#define TRACE_PROGRAMMER    FALSE
#endif

#ifndef O_BINARY
# ifdef _O_BINARY
# define O_BINARY _O_BINARY
# else
# define O_BINARY 0
# endif
#endif

#define BL_MAX_CHUNK_SIZE   248

/* JN513x / JN514x definitions */
#define JN514X_ROM_ID_ADDR                      0x00000004
#define JN514X_MAC_ADDRESS_LOCATION             0x00000030
#define JN514X_MIB_MAC_ADDRESS_LOCATION         0x00000010


/* JN516x definitions */

/* Location of MAC address in Index sector */
#define JN516X_MAC_INDEX_SECTOR_PAGE            5
#define JN516X_MAC_INDEX_SECTOR_WORD            7

/* Location of MAC address in memory map */
#define JN516X_CUSTOMER_MAC_ADDRESS_LOCATION    0x01001570
#define JN516X_MAC_ADDRESS_LOCATION             0x01001580

/* Location of bootloader information in memory map */
#define JN516X_BOOTLOADER_VERSION_ADDRESS       0x00000062
#define JN516X_BOOTLOADER_ENTRY                 0x00000066

/* Location of device configuration in memory map */
#define JN516X_INDEX_SECTOR_DEVICE_CONFIG_ADDR  0x01001500

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
#define BIN_LENGTH 3581
unsigned char bin_extension[BIN_LENGTH] = {
0x07,0x03,0x00,0x08,0x12,0x34,0x56,0x78,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,
0x08,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x04,0x10,0x01,0x00,0x03,0x71,
0x04,0x71,0x00,0x1d,0x04,0x00,0x05,0x5a,0x04,0x00,0x05,0x24,0x0a,0x14,0x2a,0x54,
0x00,0x00,0x02,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x47,0xa4,0x40,0x05,
0x45,0x04,0xa6,0x04,0xc7,0x38,0xe0,0x10,0x8c,0xe0,0x17,0x08,0x00,0x40,0x38,0xe0,
0x0a,0x8c,0xe0,0x38,0x00,0x00,0x80,0x2c,0x01,0x20,0x0d,0x40,0x2c,0xe1,0x60,0x00,
0xf8,0x2c,0xe1,0x20,0x2c,0xe1,0x60,0x38,0x40,0xe3,0xd3,0x04,0x74,0xff,0x98,0xe0,
0xa2,0x00,0x80,0x00,0x8c,0xe0,0x00,0x00,0x00,0x80,0x8c,0x00,0x00,0x00,0x00,0x00,
0x98,0xe0,0xe3,0x01,0x00,0x00,0x8c,0xe0,0x00,0x00,0x00,0x80,0xd8,0xe0,0xc2,0x20,
0x8c,0xe0,0x00,0x00,0x00,0x80,0x38,0xe0,0xe2,0x8c,0xe0,0x00,0x00,0x00,0x80,0x6c,
0xe3,0x20,0x08,0xe4,0x6c,0x67,0x20,0x2d,0x03,0x00,0x2c,0xc3,0x20,0x2c,0xa3,0x10,
0x2d,0x43,0x30,0x98,0xe0,0x04,0x01,0x00,0x00,0x8c,0xe0,0x10,0x00,0x00,0x80,0xd8,
0xe0,0xc3,0x08,0x8c,0xe0,0x00,0x00,0x00,0x80,0x8c,0xe0,0x50,0x00,0x00,0x80,0x94,
0xe7,0xff,0xfe,0x00,0x00,0x41,0x07,0x2f,0x00,0x48,0x8c,0x40,0x00,0x00,0x00,0x00,
0x8c,0x40,0x00,0x00,0x00,0x00,0x98,0xe0,0xe3,0x01,0x00,0x00,0x8c,0xe0,0x00,0x00,
0x00,0x80,0x38,0xe0,0xa2,0x8c,0xe0,0x00,0x00,0x00,0x80,0x8c,0x00,0x10,0x00,0x00,
0x80,0x8c,0x00,0x38,0x00,0x00,0x80,0x8c,0x00,0x17,0x08,0x00,0x40,0x00,0xea,0x8c,
0xe0,0x00,0x00,0x00,0x80,0x8c,0xe3,0x40,0x08,0x00,0x80,0xd2,0xce,0x8f,0x40,0x8c,
0xe3,0x60,0x08,0x00,0x80,0xd2,0xce,0x6a,0x40,0x8c,0xe3,0x50,0x08,0x00,0x80,0xd2,
0xce,0x5d,0x80,0x8c,0xe3,0x70,0x08,0x00,0x80,0x60,0xe7,0x52,0x3e,0x07,0x3c,0x60,
0xe0,0x3d,0x6c,0xe7,0xf9,0x60,0x67,0x12,0x0c,0x80,0x00,0x60,0x47,0xb4,0x40,0x00,
0x47,0xa8,0x00,0x00,0x60,0x98,0x80,0x00,0x00,0xc0,0x00,0xa4,0x00,0x10,0xd0,0x00,
0x00,0x00,0x60,0x98,0x80,0x00,0x00,0x80,0x00,0xa4,0x00,0x70,0xd0,0x00,0x00,0x00,
0x60,0x98,0x80,0x00,0x00,0x40,0x00,0xa4,0x00,0x00,0xd0,0x00,0x00,0x47,0xc8,0x00,
0xa4,0x40,0x34,0x00,0x00,0x00,0xa4,0x40,0x53,0xff,0xff,0xff,0x04,0xc3,0x24,0xa3,
0x00,0x24,0xe3,0x80,0x6c,0xe7,0x10,0x08,0xa7,0x24,0x63,0xc0,0x6c,0x63,0x18,0x24,
0xe6,0x40,0x6c,0xe7,0x08,0x08,0x67,0x08,0x65,0x47,0xd2,0x48,0x47,0xae,0x41,0x98,
0x80,0x00,0x00,0x80,0x00,0x00,0x60,0x84,0xc0,0x23,0x88,0x00,0x20,0x41,0x06,0xd0,
0x00,0x80,0x98,0x60,0x00,0x00,0x80,0x00,0xa4,0x00,0xf5,0x50,0x00,0x00,0x38,0x60,
0x10,0x00,0x88,0xa4,0x00,0x8b,0x20,0x00,0x00,0x98,0xe0,0xa0,0x48,0x00,0x20,0x8c,
0xe0,0x2c,0x0c,0x00,0x40,0x98,0xe0,0xcb,0x88,0x00,0x20,0x8c,0xe0,0x1c,0x0c,0x00,
0x40,0x0e,0xf0,0x98,0xe0,0x05,0x61,0x80,0x00,0xd3,0xde,0x71,0x00,0x01,0xf8,0x0e,
0x30,0x84,0xe0,0x23,0x88,0x00,0x20,0x3c,0x27,0x00,0x00,0xe8,0x64,0xe0,0x3a,0x80,
0xe0,0x23,0x88,0x00,0x20,0x98,0x80,0x00,0x00,0x80,0x00,0x00,0x60,0x41,0x07,0xd0,
0x00,0x80,0x98,0x60,0x00,0x00,0x80,0x00,0xa4,0x00,0xf2,0x50,0x00,0x00,0x01,0xe0,
0x00,0x60,0xa4,0x00,0xcc,0xd0,0x00,0x00,0x34,0x63,0x80,0x40,0x03,0x1d,0x00,0x60,
0xa4,0x00,0x56,0xd0,0x00,0x00,0x05,0x63,0x05,0x43,0x01,0x80,0x0c,0x88,0x01,0xf8,
0x00,0x60,0xa4,0x00,0xc8,0xd0,0x00,0x00,0x34,0x63,0x80,0x41,0x03,0xb0,0x98,0xc0,
0xf9,0x61,0x80,0x00,0xd3,0x8c,0xf5,0x7f,0x98,0xe0,0x05,0x61,0x80,0x00,0xd2,0xde,
0x71,0x00,0x01,0x60,0x0d,0xe0,0x00,0x60,0xa4,0x00,0x4c,0xd0,0x00,0x00,0xd8,0xe1,
0x10,0x40,0x08,0xec,0xc0,0x67,0x3f,0x7f,0x60,0x6a,0x1a,0x3e,0x03,0x50,0x01,0x98,
0x01,0xe0,0xd3,0x56,0xc7,0xdf,0x44,0x8b,0x59,0x41,0x0a,0xe9,0x00,0xef,0x20,0xe1,
0xa0,0xc4,0xe1,0x20,0x80,0x38,0xc0,0x4c,0xd2,0x8e,0x60,0x48,0xd3,0xce,0x61,0x40,
0x38,0xc0,0x84,0xd2,0x8e,0x62,0xb8,0xd3,0xce,0x6b,0x00,0x38,0xc0,0xf8,0xd2,0xce,
0x69,0x94,0x0c,0x32,0x38,0xc0,0xa4,0xd2,0x8e,0x66,0xa8,0x38,0xc0,0xe4,0xd2,0xce,
0x69,0x14,0x0e,0xd6,0x38,0xc0,0x5c,0xd2,0x8e,0x6e,0x38,0xd3,0xce,0x62,0x80,0x38,
0xc0,0x6c,0xd2,0x8e,0x66,0x30,0x38,0xc0,0x1c,0xd2,0xce,0x67,0x64,0x0d,0x20,0x38,
0xc0,0x3c,0xd2,0x8e,0x67,0x78,0x38,0xc0,0x16,0xd2,0xce,0x67,0xa4,0x0d,0xca,0x38,
0xe0,0x9c,0x20,0xe1,0x20,0x38,0xe0,0x28,0xd2,0xd6,0x74,0x64,0xc5,0x61,0xe0,0x80,
0x6d,0x6b,0x18,0xc4,0xe1,0x10,0x80,0x6c,0xe7,0x08,0x61,0x6b,0x39,0xc4,0xe1,0x50,
0x80,0x61,0x6b,0x39,0xc5,0x41,0xd0,0x80,0x6d,0x4a,0x18,0xc4,0xe1,0x30,0x80,0x6c,
0xe7,0x08,0x61,0x4a,0x39,0xc4,0xe1,0x70,0x80,0x61,0x4a,0x39,0xc4,0xc1,0xf0,0x80,
0x6c,0xc6,0x18,0xc4,0xe1,0x08,0x80,0x6c,0xe7,0x08,0x60,0xc6,0x39,0xc4,0xe1,0x48,
0x80,0x60,0xc6,0x39,0xc4,0xa1,0xc8,0x80,0x6c,0xa5,0x18,0xc4,0xe1,0x28,0x80,0x6c,
0xe7,0x08,0x60,0xa5,0x39,0xc4,0xe1,0x68,0x80,0x60,0xa5,0x39,0xc5,0x21,0xa8,0x80,
0x6d,0x29,0x10,0xc4,0x41,0x88,0x80,0x6c,0x42,0x10,0xc4,0xe1,0xb0,0x80,0x6c,0xe7,
0x10,0xc5,0x01,0x90,0x80,0x6d,0x08,0x10,0xc4,0x61,0xa0,0x80,0xc4,0x81,0x60,0x80,
0x60,0xa9,0x29,0x60,0xc2,0x31,0x60,0xe7,0x51,0x61,0x08,0x59,0xa4,0x00,0x05,0x3f,
0xff,0xff,0xd0,0x60,0x31,0x38,0x0e,0xe0,0x38,0xe0,0xec,0x20,0xe1,0x20,0xd0,0x70,
0xb3,0xd8,0xc4,0xc1,0xa0,0x80,0x00,0x68,0x41,0x06,0xa0,0x00,0x60,0xa4,0x00,0x36,
0xc0,0x00,0x00,0x20,0x01,0xa0,0x0c,0x96,0x38,0xe0,0xcc,0x20,0xe1,0x20,0x30,0x61,
0x60,0x98,0x80,0x3f,0x00,0x00,0x08,0x00,0xa2,0xa4,0x00,0xf5,0x1f,0xff,0xdf,0x20,
0x01,0xe0,0x30,0xe0,0x61,0x20,0xe1,0x10,0x20,0xe1,0x90,0x20,0x01,0xa0,0x00,0x86,
0x0c,0xfa,0x38,0xe0,0x04,0x20,0xe1,0x20,0xd8,0x61,0xa0,0x80,0xa4,0x00,0x09,0xbf,
0xff,0xff,0x05,0xc3,0xc5,0x41,0x90,0x80,0x30,0x61,0x60,0x04,0x8e,0x04,0xaa,0xa4,
0x00,0x9e,0x1f,0xff,0xdf,0x30,0x8a,0x40,0x20,0x01,0xa0,0x0f,0x2a,0x38,0xe0,0x64,
0x20,0xe1,0x20,0x00,0x82,0x01,0xc0,0xd0,0x50,0xbe,0x28,0xa4,0x00,0x8a,0x1f,0xff,
0xdf,0x41,0x03,0x50,0x00,0xe7,0x20,0xe1,0xa0,0x0e,0x80,0x20,0x01,0xa0,0x6c,0xe3,
0x11,0x20,0xe1,0x60,0x20,0x61,0xe0,0x00,0x82,0x0f,0x12,0x38,0xe0,0x96,0x20,0xe1,
0x20,0x00,0x8a,0x01,0xc0,0xd0,0x50,0xb9,0x88,0xa4,0x00,0x60,0x40,0x00,0x00,0x41,
0x03,0x50,0x00,0xe7,0x20,0xe1,0xa0,0x0e,0x80,0x20,0x01,0xa0,0x6c,0xe3,0x09,0x20,
0xe1,0x60,0x6c,0xe3,0x11,0x20,0xe1,0xe0,0x20,0x61,0x10,0x00,0x8a,0x0f,0xdc,0x38,
0xe0,0x14,0x20,0xe1,0x20,0x0d,0xf3,0xd8,0x61,0xa0,0x80,0xa4,0x00,0x80,0xbf,0xff,
0xff,0x05,0xc3,0x38,0xe0,0x44,0x20,0xe1,0x20,0x20,0x01,0xa0,0x0d,0x4c,0xd8,0x61,
0xa0,0x80,0xa4,0x00,0x57,0x3f,0xff,0xff,0x05,0xc3,0xc5,0x41,0x90,0x80,0x38,0xe0,
0xdc,0x20,0xe1,0x20,0x20,0x01,0xa0,0x00,0x60,0xa4,0x00,0x1a,0xe0,0x00,0x00,0x00,
0x68,0xa4,0x00,0x3a,0x60,0x00,0x00,0x6c,0x6e,0x61,0x3e,0x03,0x1a,0x34,0x8e,0xfc,
0x30,0xa1,0x60,0x04,0xca,0xa4,0x00,0x98,0x60,0x00,0x00,0x30,0x8a,0x40,0x0c,0x24,
0xd8,0x61,0xa0,0x80,0xa4,0x00,0x15,0x3f,0xff,0xff,0x05,0xc3,0x30,0xeb,0x5f,0x3e,
0x07,0x6a,0x38,0xe0,0xbc,0x20,0xe1,0x20,0x20,0x01,0xa0,0x00,0x60,0xa4,0x00,0x28,
0xe0,0x00,0x00,0x01,0x80,0x0c,0xc8,0x60,0xed,0x65,0x3c,0xc7,0x02,0x38,0xc0,0x02,
0x64,0xe7,0x32,0x3e,0x07,0x58,0x21,0x61,0x10,0x00,0x60,0xa4,0x00,0x40,0x60,0x00,
0x00,0x6d,0x4e,0x61,0x3e,0x0a,0x52,0xd8,0xa1,0x10,0x40,0x08,0xac,0x04,0x6a,0x34,
0x8e,0xfc,0xd8,0xa5,0x80,0xff,0x04,0xcb,0xa4,0x00,0x61,0xa0,0x00,0x00,0x04,0x6a,
0xa4,0x00,0xe4,0x60,0x00,0x00,0x41,0x03,0xd8,0x60,0xec,0x5c,0x3e,0x07,0x62,0x09,
0xcb,0xd3,0xda,0xc6,0xdf,0x0e,0x20,0x30,0xe7,0x01,0x20,0xe1,0x20,0x00,0x88,0x0e,
0xc0,0x00,0xe7,0x20,0xe1,0xa0,0x00,0x84,0x0d,0x80,0x00,0x84,0x01,0xc0,0x30,0xe4,
0x80,0x3e,0x07,0x30,0x00,0xa0,0x30,0x41,0x20,0x0e,0xc0,0x60,0xe2,0x2c,0x24,0xe7,
0x00,0x60,0xc6,0x3a,0x00,0xb8,0xd3,0x48,0x5a,0xff,0xd8,0xe1,0x10,0x40,0x08,0xe5,
0xc0,0xc7,0x3f,0xbf,0x31,0x44,0x80,0x00,0x60,0xa4,0x00,0x33,0xe0,0x00,0x00,0x34,
0x63,0x04,0x40,0x03,0xaf,0x00,0x60,0x3e,0x0a,0x20,0xa4,0x00,0x87,0xe0,0x00,0x00,
0x01,0x60,0x0c,0x90,0x00,0x60,0xa4,0x00,0xf5,0xe0,0x00,0x00,0x34,0x63,0x04,0x40,
0x03,0xaf,0xd8,0xe1,0x10,0x40,0x08,0xeb,0x00,0x60,0xc4,0x87,0x3f,0xbf,0xa4,0x00,
0xbd,0xe0,0x00,0x00,0x01,0x78,0xd3,0x54,0xb7,0xbf,0x00,0x60,0xa4,0x00,0x91,0xe0,
0x00,0x00,0x34,0x63,0x04,0x40,0x03,0xaf,0xc4,0xe1,0x20,0x80,0x38,0xc0,0x84,0xd2,
0xce,0x68,0x23,0x98,0x60,0x00,0x00,0x80,0x00,0x00,0x80,0xa4,0x00,0x3e,0x60,0x00,
0x00,0x98,0x60,0x00,0x00,0x40,0x00,0x00,0x80,0xa4,0x00,0x76,0x60,0x00,0x00,0x98,
0xe0,0x04,0x85,0xe0,0x00,0x0c,0x40,0xcc,0xe1,0x60,0x40,0x00,0xff,0xcc,0xe1,0x20,
0x40,0xcc,0xe1,0x60,0x40,0x41,0x07,0x4f,0x47,0xd2,0x70,0x00,0x10,0x46,0xc0,0x3f,
0x00,0x60,0xa4,0x40,0x57,0x60,0x00,0x00,0x84,0x60,0xa3,0x88,0x00,0x20,0xa4,0x40,
0x7b,0x60,0x00,0x00,0x47,0xa8,0x00,0xa4,0x00,0x00,0xe0,0x00,0x00,0xa4,0x00,0x70,
0xe0,0x00,0x00,0x41,0x03,0x5f,0xa4,0x00,0xdf,0x60,0x00,0x00,0x47,0xb8,0x00,0x47,
0xa2,0x00,0xa4,0x00,0x6b,0xff,0xff,0xff,0x38,0x60,0xf9,0xa4,0x00,0x9b,0xff,0xff,
0xff,0x00,0x60,0xa4,0x00,0x8b,0xff,0xff,0xff,0x05,0x83,0x00,0x60,0xa4,0x00,0xe3,
0xff,0xff,0xff,0x05,0x63,0x00,0x60,0xa4,0x00,0xbd,0xff,0xff,0xff,0x05,0x43,0xa4,
0x00,0x85,0xff,0xff,0xff,0x6d,0x6b,0x10,0x61,0x4a,0x59,0x6d,0x8c,0x08,0x60,0x6a,
0x61,0x47,0xb2,0x00,0x47,0xa4,0x40,0x05,0x44,0x3e,0x03,0x38,0x2c,0x01,0x00,0x00,
0x60,0x00,0x80,0x00,0xa0,0x00,0xc0,0x01,0x00,0xa4,0x00,0x4b,0xa0,0x00,0x00,0x81,
0x40,0xa3,0x88,0x00,0x20,0x47,0xb4,0x40,0x47,0xae,0x20,0x05,0xe3,0x05,0xc4,0x05,
0x65,0x05,0x46,0x2c,0x05,0x00,0x98,0xe0,0xff,0xff,0xff,0x00,0x2c,0xe6,0x00,0x01,
0x80,0x0e,0x08,0x3e,0x0c,0x18,0x20,0x61,0x80,0x30,0x81,0x20,0x30,0xa1,0x70,0xa4,
0x00,0x6d,0x80,0x00,0x00,0x04,0x61,0x04,0x8e,0x00,0xa8,0xa4,0x00,0x7f,0x80,0x00,
0x00,0x41,0x03,0xf8,0x04,0x61,0xa4,0x00,0x49,0x40,0x00,0x00,0x2c,0xeb,0x40,0xd3,
0x8e,0x3e,0x00,0x2c,0x6b,0x00,0x2c,0xea,0x40,0xd3,0x86,0x7e,0x00,0x2c,0x6a,0x00,
0x01,0x98,0xd3,0x5e,0xc8,0x3f,0x47,0xbe,0x20,0x47,0xae,0x68,0x05,0xe3,0x00,0x60,
0xa4,0x00,0x87,0x20,0x00,0x00,0x2c,0x61,0x20,0x41,0x0f,0x18,0x30,0x81,0x10,0x98,
0xa0,0x13,0x88,0x00,0x20,0x98,0xc0,0x33,0x88,0x00,0x20,0xa4,0x00,0xbe,0xff,0xff,
0xff,0x01,0xc0,0x0c,0x7c,0x41,0x8f,0x70,0x3e,0x0e,0x1a,0xa4,0x00,0x36,0x20,0x00,
0x00,0x0c,0x5c,0x3e,0x0e,0x68,0x21,0xa1,0x92,0x31,0x41,0x12,0x04,0x6d,0x30,0x81,
0x32,0x30,0xa1,0x6a,0xa4,0x00,0x8c,0x80,0x00,0x00,0x31,0x81,0x10,0x04,0x6a,0x04,
0x8c,0x00,0xa0,0xa4,0x00,0x6e,0x80,0x00,0x00,0x29,0x61,0xea,0x04,0x6a,0xa4,0x00,
0x37,0x80,0x00,0x00,0x41,0x03,0xe0,0xd0,0x00,0xb4,0xd0,0x04,0x6a,0x04,0x8c,0xa4,
0x00,0x42,0x40,0x00,0x00,0xd2,0x96,0x33,0x80,0x04,0x6a,0xa4,0x00,0xb7,0x80,0x00,
0x00,0x05,0x63,0x8c,0x80,0x53,0x88,0x00,0x20,0x01,0x80,0xd3,0x88,0x38,0x40,0x0f,
0x20,0x04,0x6a,0xa4,0x00,0xed,0x80,0x00,0x00,0x38,0xe0,0x30,0xd2,0xc6,0x71,0x10,
0x0e,0x78,0x04,0x6a,0xa4,0x00,0x4d,0x80,0x00,0x00,0x01,0x88,0x31,0x41,0x12,0x8c,
0xe0,0x73,0x88,0x00,0x20,0xd3,0x96,0x7f,0x80,0x04,0x6a,0x8c,0x80,0x53,0x88,0x00,
0x20,0xa4,0x00,0xa9,0x80,0x00,0x00,0x04,0x6a,0x38,0x80,0x30,0xa4,0x00,0x96,0x80,
0x00,0x00,0x0c,0xe0,0x04,0x6a,0x38,0x80,0x30,0xa4,0x00,0x3a,0x80,0x00,0x00,0x40,
0x8c,0xf0,0x30,0x61,0x12,0x30,0x8b,0x80,0xa4,0x00,0x76,0x80,0x00,0x00,0x30,0x61,
0x12,0x30,0x81,0x10,0xa4,0x00,0xbd,0x80,0x00,0x00,0x05,0x43,0x00,0x60,0xa4,0x00,
0xf7,0x40,0x00,0x00,0x04,0x6d,0x30,0x81,0x32,0x04,0xaa,0xa4,0x00,0xcc,0x00,0x00,
0x00,0x04,0x6d,0xa4,0x00,0x24,0xc0,0x00,0x00,0x01,0xd8,0x2c,0xe1,0x60,0xd3,0x4e,
0xee,0x0f,0x0e,0x60,0x04,0x6a,0xa4,0x00,0x42,0x80,0x00,0x00,0x05,0x63,0x04,0x6a,
0x38,0x80,0x30,0xa4,0x00,0x40,0x80,0x00,0x00,0x0e,0x57,0x47,0xbe,0x68,0x47,0xac,
0x20,0x05,0x63,0x00,0x6f,0x40,0x04,0xb4,0x2c,0xe4,0x40,0x2c,0xe1,0x20,0x2c,0xe4,
0x60,0x2c,0xe1,0x10,0x31,0x41,0x20,0x04,0x6a,0x04,0x85,0xa4,0x00,0xc4,0x80,0x00,
0x00,0x04,0x6b,0x00,0x80,0x04,0xaa,0x38,0xc0,0x30,0xa4,0x00,0x24,0x40,0x00,0x00,
0x00,0x60,0x47,0xbc,0x20,0x47,0xa6,0x20,0x05,0x83,0x05,0x64,0x05,0xa5,0x40,0x04,
0x60,0x41,0x05,0xe0,0x00,0x6f,0x0c,0x08,0x00,0x68,0xa4,0x00,0xb9,0xc0,0x00,0x00,
0x05,0x43,0x31,0xc1,0x20,0x04,0x6c,0x00,0x80,0x04,0xae,0x38,0xc0,0x30,0xa4,0x00,
0x04,0x40,0x00,0x00,0x41,0x0a,0x90,0x8c,0x00,0x29,0x00,0x00,0x80,0x2c,0xe1,0x60,
0x2c,0xeb,0x00,0x2c,0xe1,0x50,0x2c,0xeb,0x20,0x04,0x6e,0xa4,0x00,0xf3,0x00,0x00,
0x00,0x28,0x6d,0x00,0x00,0x60,0x47,0xb6,0x20,0x47,0xa6,0x20,0x05,0x83,0x05,0xa4,
0x05,0x45,0x00,0x68,0xa4,0x00,0xca,0xc0,0x00,0x00,0x05,0xc3,0x25,0x6c,0x80,0x04,
0x6c,0xa4,0x00,0x3d,0x00,0x00,0x00,0x04,0xc3,0x04,0x6b,0x38,0x80,0x30,0x04,0xad,
0xa4,0x00,0x73,0x80,0x00,0x00,0x41,0x8a,0xf4,0x31,0x41,0x20,0x24,0x6c,0x80,0x00,
0x80,0x04,0xaa,0x38,0xc0,0x30,0xa4,0x00,0x1d,0x80,0x00,0x00,0x04,0x6a,0xa4,0x00,
0x3e,0x00,0x00,0x00,0x04,0x83,0x04,0x6c,0x04,0xad,0xa4,0x00,0x8a,0x80,0x00,0x00,
0x00,0xef,0x41,0x03,0x70,0x00,0xe0,0x41,0x0e,0x90,0x8c,0x00,0x29,0x00,0x00,0x80,
0x04,0x67,0x47,0xb6,0x20,0x24,0xe3,0x10,0x94,0xe7,0x87,0xff,0xff,0xff,0x6c,0x84,
0x80,0x60,0xe7,0x21,0x20,0xe3,0x10,0x47,0xd2,0x48,0x24,0x63,0x10,0x34,0x63,0x78,
0x6c,0x63,0x81,0x47,0xd2,0x48,0x6c,0xe4,0x09,0x20,0xe3,0x90,0x6c,0xe4,0x11,0x20,
0xe3,0x50,0x20,0x83,0xd0,0x47,0xd2,0x48,0x24,0xc3,0xd0,0x24,0xe3,0x50,0x6c,0xe7,
0x10,0x08,0xc7,0x24,0x63,0x90,0x6c,0x63,0x08,0x08,0x66,0x47,0xd2,0x48,0x6c,0xe4,
0x11,0x20,0xe3,0x10,0x20,0x83,0x90,0x47,0xd2,0x48,0x04,0xe3,0x24,0x63,0x90,0x24,
0xe7,0x10,0x6c,0xe7,0x10,0x08,0x67,0x3e,0x03,0x1a,0x47,0xd2,0x48,0x24,0xe3,0x10,
0x34,0xe7,0x02,0x38,0xc0,0x2c,0x40,0x07,0x60,0x24,0xc3,0xe0,0x04,0x66,0x47,0xd2,
0x48,0x47,0xa2,0x00,0x05,0x43,0x05,0x84,0xa4,0x00,0x0d,0xff,0xff,0xff,0x6c,0x83,
0x09,0x28,0xaa,0xa0,0x3e,0x03,0x1a,0x08,0xa3,0x24,0xca,0xe0,0x24,0xea,0x10,0x6c,
0xe7,0x10,0x08,0xc7,0x3e,0x06,0x32,0x08,0x86,0x24,0xea,0x60,0x6c,0xe7,0x10,0x00,
0xf8,0x3e,0x07,0x3a,0x08,0xa7,0x61,0x64,0x2c,0x04,0x6a,0xa4,0x00,0xfa,0xff,0xff,
0xff,0x40,0xc3,0x76,0x04,0x6a,0xa4,0x00,0x2a,0xff,0xff,0xff,0x38,0xe0,0x10,0xd2,
0x86,0x70,0x60,0x04,0x6a,0xa4,0x00,0xa2,0xff,0xff,0xff,0x38,0xe0,0x90,0xd2,0x86,
0x78,0xa0,0x04,0x6a,0xa4,0x00,0x91,0xff,0xff,0xff,0x6c,0xe3,0x81,0x3e,0x07,0x42,
0x3e,0x03,0x18,0x04,0xac,0x00,0x80,0x00,0x40,0x0d,0xa0,0xc4,0xc5,0xff,0xff,0xc4,
0xe5,0x7f,0xff,0x6c,0xe7,0x10,0x08,0xc7,0x3e,0x06,0x32,0x08,0x46,0x00,0x98,0x00,
0xb4,0xd3,0x50,0x45,0x7f,0x34,0xe3,0x80,0x40,0x07,0x08,0x6c,0xe4,0x80,0x08,0xec,
0x24,0xe7,0x00,0x6c,0xe7,0x10,0x08,0x47,0x09,0x62,0x0e,0x80,0x61,0x63,0x3c,0x6c,
0xeb,0x09,0x3e,0x0b,0x1a,0x41,0x07,0xef,0x47,0xb2,0x00,0x47,0xa4,0x00,0x05,0x44,
0x04,0x85,0x40,0x03,0xc8,0x40,0x05,0x08,0x40,0x0a,0x88,0xa4,0x00,0x6c,0xff,0xff,
0xff,0xd2,0x94,0x31,0x00,0x00,0x6f,0x0c,0x80,0x00,0x60,0x47,0xb4,0x00,0x6c,0x63,
0x60,0x08,0x64,0x8c,0x60,0x21,0x00,0x00,0x80,0x00,0xec,0x8c,0xe0,0x01,0x00,0x00,
0x80,0x00,0x80,0x0c,0xa0,0x60,0xe5,0x24,0x24,0xe7,0x00,0xd8,0xe7,0x00,0xff,0x8c,
0xe0,0x11,0x00,0x00,0x80,0x00,0x98,0xd3,0x4c,0x47,0x7f,0x47,0xd2,0x48,0x04,0x46,
0x6c,0x63,0x60,0x08,0x64,0x8c,0x60,0x21,0x00,0x00,0x80,0x38,0xe0,0x10,0x8c,0xe0,
0x01,0x00,0x00,0x80,0x00,0x80,0x0c,0x20,0x8c,0xc0,0x71,0x00,0x00,0x80,0x60,0xe5,
0x24,0x20,0xc7,0x00,0x00,0x98,0xd3,0x44,0x44,0xff,0x47,0xd2,0x48,0xcc,0xf0,0x7a,
0x01,0x34,0xc7,0xe0,0x41,0x03,0x88,0x84,0xe0,0x4b,0x88,0x00,0x20,0x00,0xf8,0x80,
0xe0,0x4b,0x88,0x00,0x20,0x90,0xe6,0x00,0x20,0x00,0x20,0x24,0xe7,0x00,0x8c,0xe0,
0x29,0x00,0x00,0x80,0x3c,0x26,0xc0,0x98,0xe0,0x00,0x80,0x80,0x00,0x64,0xe0,0x3a,
0x8c,0xe0,0x09,0x00,0x00,0x80,0x84,0xe0,0x4b,0x88,0x00,0x20,0x3c,0x07,0x00,0x00,
0x68,0x64,0x60,0x1a,0x47,0xd2,0x48,0x04,0xc3,0x88,0xe0,0x8b,0x88,0x00,0x20,0x00,
0xff,0x38,0x60,0xff,0xd3,0x4c,0x70,0xa0,0x84,0xa0,0x4b,0x88,0x00,0x20,0x80,0x00,
0x4b,0x88,0x00,0x20,0x6c,0xe6,0x60,0x8c,0xe0,0x21,0x00,0x00,0x80,0x00,0xe6,0x8c,
0xe0,0x01,0x00,0x00,0x80,0x8c,0xe0,0x47,0xf0,0x00,0x80,0x94,0xe7,0x00,0x00,0x00,
0x08,0x40,0x07,0x2f,0x98,0xe0,0x00,0x00,0x00,0x08,0x8c,0xe0,0x17,0xf0,0x00,0x80,
0x8c,0x00,0x29,0x00,0x00,0x80,0x00,0xe8,0x60,0xe5,0x3a,0x3e,0x07,0x3c,0x60,0xe0,
0x3d,0x6c,0x67,0xf9,0x47,0xd2,0x48,0x47,0xa4,0x00,0x05,0x43,0x88,0xe0,0x8b,0x88,
0x00,0x20,0x00,0xff,0x00,0x6f,0xd3,0x54,0x71,0xc0,0x00,0x60,0xa4,0x00,0x82,0xff,
0xff,0xff,0x6c,0xea,0x60,0x8c,0xe0,0x21,0x00,0x00,0x80,0x00,0xea,0x8c,0xe0,0x01,
0x00,0x00,0x80,0x00,0xe0,0x8c,0x00,0x11,0x00,0x00,0x80,0x00,0xf8,0x38,0xc0,0x08,
0xd2,0xce,0x6a,0xff,0x04,0x6a,0xa4,0x00,0x86,0xff,0xff,0xff,0x00,0x60,0x47,0xb4,
0x00,0x04,0xc3,0xcc,0xf0,0x7f,0x01,0x6c,0xe7,0x31,0x34,0xe7,0xf8,0x38,0xa0,0x90,
0xd2,0x8e,0x55,0x80,0x38,0xa0,0x50,0xd2,0x8e,0x51,0x80,0x98,0x60,0xff,0xff,0x00,
0x00,0x38,0xa0,0x10,0xd2,0xce,0x5e,0x80,0x0f,0x80,0x38,0x60,0xff,0x0e,0x80,0x38,
0x60,0xfc,0x40,0x06,0x90,0x38,0xe0,0x02,0x20,0xe6,0x00,0x3e,0x03,0x1a,0x88,0x60,
0x0b,0x88,0x00,0x20,0x47,0xd2,0x48,0x47,0xa8,0x00,0xa4,0x00,0xc3,0x7f,0xff,0xff,
0x47,0xb8,0x00,0xcc,0xf0,0x40,0x05,0x60,0xe7,0x21,0x60,0x63,0x1b,0x60,0xe7,0x18,
0xcc,0xf0,0x00,0x05,0x47,0xd2,0x48,0xcc,0xf0,0x60,0x05,0x60,0xe7,0x19,0x60,0x84,
0x23,0x60,0xe7,0x20,0xcc,0xf0,0x20,0x05,0x47,0xd2,0x48,0x47,0xac,0x00,0x05,0x45,
0x05,0x66,0x04,0xa7,0x24,0x41,0xf0,0xcc,0xf0,0x40,0x01,0x38,0xe7,0x08,0xcc,0xf0,
0x00,0x01,0xd1,0xd0,0x32,0xc0,0x40,0x03,0x68,0xcc,0xf0,0x40,0x09,0x6c,0xe7,0x89,
0x34,0xe7,0xc0,0x3d,0xe3,0x38,0x64,0xe3,0x3a,0x3e,0x07,0x18,0xcc,0xd0,0x40,0x09,
0x6c,0xe3,0x88,0x94,0xe7,0x00,0x00,0x60,0x00,0x94,0xc6,0xff,0xff,0x9f,0xff,0x60,
0xe7,0x31,0xcc,0xf0,0x00,0x09,0x3c,0x02,0x00,0xd8,0xe0,0x00,0x08,0x64,0xc0,0x3a,
0x40,0x04,0x30,0x98,0xe6,0x00,0x20,0x00,0x00,0x3e,0x07,0x32,0x38,0xe6,0x20,0x3c,
0x2a,0x00,0x64,0xc7,0x32,0x38,0xe6,0x40,0x3c,0x2b,0x00,0x64,0xc7,0x32,0xcc,0xd0,
0x20,0x00,0x00,0xee,0xcc,0xf0,0x28,0x00,0x3e,0x08,0x3c,0x60,0xe0,0x3d,0x6c,0xe7,
0xf9,0xcc,0xf0,0x08,0x00,0xcc,0xb0,0x10,0x00,0x47,0xbc,0x00,0xcc,0x70,0x30,0x00,
0x47,0xd2,0x48,0xcc,0x90,0x00,0x00,0xcc,0xf0,0x60,0x00,0x6c,0x63,0xc0,0x38,0x63,
0x80,0x94,0x63,0x9f,0x00,0x00,0x00,0x94,0xe7,0x60,0xff,0xff,0xff,0x60,0x63,0x39,
0xcc,0x70,0x20,0x00,0x47,0xd2,0x48,0x04,0x83,0x00,0x6e,0xa4,0x40,0x1b,0xff,0xff,
0xff,0xcc,0x70,0x40,0x00,0x3e,0x03,0x18,0x47,0xd2,0x48,0xcc,0x70,0x60,0x00,0x34,
0x63,0x80,0x47,0xd2,0x48,0x00,0xe0,0xd1,0xe0,0x3f,0x80,0x6c,0xe3,0x40,0x90,0xe7,
0x10,0x20,0x00,0x20,0x2c,0xe7,0x40,0x6c,0xe7,0x40,0xd8,0xe7,0x28,0x0d,0x08,0xf0,
0x2c,0xe7,0x40,0x3e,0x07,0x38,0x04,0x67,0x47,0xd2,0x48,0xd1,0xe0,0x33,0x80,0x6c,
0xe3,0x40,0x90,0xe7,0x10,0x20,0x00,0x20,0x2c,0xe7,0x40,0x6c,0xe7,0x40,0xd8,0xe7,
0x00,0x0d,0x08,0xf0,0x2c,0x87,0x00,0x47,0xd2,0x48,0x00,0xe0,0xd1,0xe0,0x3f,0x80,
0x6c,0xe3,0x40,0x90,0xe7,0x10,0x20,0x00,0x20,0x2c,0xe7,0x40,0x6c,0xe7,0x40,0xd8,
0xe7,0x00,0x0d,0x08,0xf0,0x2c,0xe7,0x40,0x3e,0x07,0x38,0x04,0x67,0x47,0xd2,0x48,
0x47,0xa4,0x00,0x31,0x41,0x10,0x04,0xc3,0x0c,0xc0,0x24,0xe4,0x00,0x20,0xe6,0x00,
0x00,0xd8,0x00,0x98,0x00,0xbf,0xd0,0x7e,0x52,0xff,0x47,0xb4,0x00
};

static teStatus ePRG_ChipGetChipId(tsPRG_Context *psContext);
static teStatus ePRG_ChipGetMacAddress(tsPRG_Context *psContext);
static teStatus ePRG_SetUpImage(tsPRG_Context *psContext, tsFW_Info *psFWImage, tsChipDetails *psChipDetails);
static teStatus ePRG_ConfirmAlways(void *pvUser, const char *pcTitle, const char *pcText);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
#define VERSION_MAJOR
#define VERSION_MINOR
#define VERSION_SVN

/** Version string for libprogrammer */
#ifndef VERSION_MAJOR
#error Major Version is not defined!
#else
#ifndef VERSION_MINOR
#error Minor Version is not defined!
#else
#ifndef VERSION_SVN
#error SVN Version is not defined!
#else
const char *pcPRG_Version = VERSION_MAJOR "." VERSION_MINOR " (r" VERSION_SVN ")";
#endif
#endif
#endif

/** Import binary data from FlashProgrammerExtension_JN5168.bin */
/*
#if defined POSIX
extern int _binary_FlashProgrammerExtension_JN5168_bin_start;
#define FLASHPROGRAMMEREXTENSION_JN5168_BIN     ((uint8_t *)    &_binary_FlashProgrammerExtension_JN5168_bin_start)
#elif defined WIN32
extern int binary_FlashProgrammerExtension_JN5168_bin_start;
#define FLASHPROGRAMMEREXTENSION_JN5168_BIN     ((uint8_t *)    &binary_FlashProgrammerExtension_JN5168_bin_start)
#endif
*/
unsigned char *bin_file;
#define FLASHPROGRAMMEREXTENSION_JN5168_BIN bin_file

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/


teStatus LIBPROGRAMMER ePRG_Init(tsPRG_Context *psContext)
{
/*	FILE* fp;
	long longBytes;
	int i;
	unsigned char ch;
	fp = fopen("FlashProgrammerExtension_JN5168.bin","rb+");
	fseek(fp,0,SEEK_SET);
	fseek(fp,0,SEEK_END);
	longBytes=ftell(fp);
    fclose(fp);
	bin_file = (unsigned char*)malloc(longBytes);
    fp = fopen("FlashProgrammerExtension_JN5168.bin","rb+");
	i=0;
	
	while( i < longBytes)
	{
		ch = getc(fp);
		bin_file[i++] = ch;
	}			
*/


StartLogging("LifeSensor.log", "debug,log,info,warn,error", "all");
    bin_file = bin_extension;
	memset(psContext, 0, sizeof(tsPRG_Context));
    
    psContext->pvPrivate = malloc(sizeof(tsPRG_PrivateContext));
    
    if (!psContext->pvPrivate)
    {
        return E_PRG_OUT_OF_MEMORY;
    }
    
    memset(psContext->pvPrivate, 0, sizeof(tsPRG_PrivateContext));
    
    return ePRG_SetStatus(psContext, E_PRG_OK, "");
}



teStatus LIBPROGRAMMER ePRG_Destroy(tsPRG_Context *psContext)
{
    ePRG_FwClose(psContext);

	//free(bin_file);
    free(psContext->pvPrivate);
    
    StopLogging();
    return E_PRG_OK;
}


char * pcPRG_GetLastStatusMessage(tsPRG_Context *psContext)
{
    tsPRG_PrivateContext *psPriv = (tsPRG_PrivateContext *)psContext->pvPrivate;
    
    if (psPriv)
    {
        return psPriv->acStatusMessage;
    }
    return NULL;
}


teStatus LIBPROGRAMMER ePRG_ConnectionOpen(tsPRG_Context *psContext, tsConnection *psConnection)
{
    if ((!psContext) || (!psConnection))
    {
        return E_PRG_NULL_PARAMETER;
    }
    
    switch(psConnection->eType)
    {
        case (E_CONNECT_SERIAL):
            return ePRG_ConnectionUartOpen(psContext, psConnection);
    
        default:
            return E_PRG_INVALID_TRANSPORT;
    }
}


teStatus LIBPROGRAMMER ePRG_ConnectionClose(tsPRG_Context *psContext)
{
    tsPRG_PrivateContext *psPriv;  
    if (!psContext)
    {
        return E_PRG_NULL_PARAMETER;
    }
    
    psPriv = (tsPRG_PrivateContext *)psContext->pvPrivate;
    
    switch(psPriv->sConnection.eType)
    {
        case (E_CONNECT_SERIAL):
            return ePRG_ConnectionUartClose(psContext);
    
        default:
            return E_PRG_INVALID_TRANSPORT;
    }
}


teStatus ePRG_ConnectionUpdate(tsPRG_Context *psContext, tsConnection *psConnection)
{
    tsPRG_PrivateContext *psPriv;    
    teStatus eStatus;
    
    if (!psContext)
    {
        return E_PRG_NULL_PARAMETER;
    }
    psPriv = (tsPRG_PrivateContext *)psContext->pvPrivate;
    
    if (psPriv->sConnection.eType != E_CONNECT_SERIAL)
    {
        // It is invalid to attempt to change baudrate if we're not using the serial transport.
        return ePRG_SetStatus(psContext, E_PRG_INVALID_TRANSPORT, "cannot change baud rate on non-serial transport");
    }
    
    /* Change bootloader to new speed */
    if ((eStatus = eBL_SetBaudrate(psContext, psConnection->uDetails.sSerial.u32BaudRate)) != E_PRG_OK)
    {
        switch (CHIP_ID_PART(psContext->sChipDetails.u32ChipId))
        {
            /* These 4x bootloaders corrupt the CRC byte on the change baud response so are expected to fail */
            case (CHIP_ID_PART(CHIP_ID_JN5148_REV2A)):
            case (CHIP_ID_PART(CHIP_ID_JN5142_REV1A)):
            case (CHIP_ID_PART(CHIP_ID_JN5142_REV1B)):
            case (CHIP_ID_PART(CHIP_ID_JN5142_REV1C)):
                break;
            default:
                return ePRG_SetStatus(psContext, eStatus, "selecting baud rate");
        }
    }
    
    /* change local port to new speed */
    if ((eStatus = ePRG_ConnectionUartUpdate(psContext, psConnection)) != E_PRG_OK)
    {
        return ePRG_SetStatus(psContext, eStatus, "selecting baud rate");
    }
    
    psPriv->sConnection.uDetails = psConnection->uDetails;
    
    return ePRG_SetStatus(psContext, E_PRG_OK, "");
}


teStatus ePRG_ChipGetDetails(tsPRG_Context *psContext)
{
     teStatus eStatus;
    
    if ((eStatus = eUART_Flush(psContext)) != E_PRG_OK)
    {
        return ePRG_SetStatus(psContext, eStatus, "flushing UART");
    }
    
    if ((eStatus = ePRG_ChipGetChipId(psContext)) != E_PRG_OK)
    {
        return ePRG_SetStatus(psContext, eStatus, "reading chip ID");
    }
    
    if ((eStatus = ePRG_ChipGetMacAddress(psContext)) != E_PRG_OK)
    {
        return ePRG_SetStatus(psContext, eStatus, "reading MAC address");
    }
    
    return ePRG_SetStatus(psContext, E_PRG_OK, "");
}


teStatus ePRG_FlashErase(tsPRG_Context *psContext, tcbFW_Progress cbProgress, void *pvUser)
{
    teStatus eStatus;
    uint16_t u16FlashId;
    uint8_t au8Buffer1[BL_MAX_CHUNK_SIZE + 1];
    uint8_t au8Buffer2[BL_MAX_CHUNK_SIZE + 1];

    if (!psContext)
    {
        return E_PRG_NULL_PARAMETER;
    }

    /* Get the flash Id */
    if((eStatus = eBL_FlashIdRead(psContext, &u16FlashId)) != E_PRG_OK)
    {
        return ePRG_SetStatus(psContext, eStatus, "reading flash ID");
    }
    LOGD("FlashId: %04x", u16FlashId);

    /* Set the flash type */
    if((eStatus = eBL_FlashSelectDevice(psContext, u16FlashId)) != E_PRG_OK)
    {
        return ePRG_SetStatus(psContext, eStatus, "selecting flash type");
    }

    /* If its not internal flash, we need to enable write access */
    if(u16FlashId != FLASH_INTERNAL)
    {
        if((eStatus = eBL_FlashStatusRegisterWrite(psContext, 0x00)) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, eStatus, "writing to flash status register");
        }
    }

    /* Erase the flash memory */
    if (cbProgress)
    {
        if (cbProgress(pvUser, "Erasing", "Erasing", 1, 0) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
        }
    }
    if((eStatus = eBL_FlashErase(psContext)) != E_PRG_OK)
    {
        return ePRG_SetStatus(psContext, eStatus, "erasing flash");
    }

    /* Ensure that flash is erased */
    LOGD("Checking flash is blank...");
    memset(au8Buffer2, 0xFF, 64);

    if ((eStatus = eBL_FlashRead(psContext, 0, 64, au8Buffer1)) != E_PRG_OK)
    {
        return ePRG_SetStatus(psContext, eStatus, "reading Flash at address 0x%08X", 0);
    }
    else
    {
        if (memcmp(au8Buffer1, au8Buffer2, 64))
        {
            return ePRG_SetStatus(psContext, E_PRG_ERROR, "flash erase unsuccessful");
        }
        else
        {
            if (cbProgress)
            {
                if (cbProgress(pvUser, "Erasing", "Erasing", 1, 1) != E_PRG_OK)
                {
                    return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
                }
            }
        }
    }
    return ePRG_SetStatus(psContext, E_PRG_OK, "flash erased succesfully");
}


teStatus ePRG_FlashProgram(tsPRG_Context *psContext, tcbFW_Progress cbProgress, tcbFW_Confirm cbConfirm, void *pvUser)
{
    teStatus eStatus;
    int n;
    uint8_t u8ChunkSize;
    tsChipDetails *psChipDetails;
    tsFW_Info *psFWImage;
    
    if (!psContext)
    {
        return E_PRG_NULL_PARAMETER;
    }
    
    psChipDetails = &psContext->sChipDetails;
    psFWImage = &psContext->sFirmwareInfo;

    if (memcmp(&psFWImage->u32ROMVersion, &psChipDetails->u32SupportedFirmware, 4) != 0)
    {
        eStatus = E_PRG_INCOMPATIBLE;
        if (cbConfirm)
        {
            eStatus = cbConfirm(pvUser, "Confirm Operation", "The loaded image does not appear to be compatible with the connected device. Continue?");
        }
        if (eStatus != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, E_PRG_INCOMPATIBLE, "image built for 0x%08X, connected device is 0x%08x", psFWImage->u32ROMVersion, psChipDetails->u32SupportedFirmware);
        }
        // User has given the go ahead to continue anyway. Here be dragons.
    }

    if ((eStatus = ePRG_SetUpImage(psContext, psFWImage, psChipDetails)) != E_PRG_OK)
    {
        return eStatus;
    }
    
    if ((eStatus = ePRG_FlashErase(psContext, NULL, NULL)) != E_PRG_OK)
    {
        return ePRG_SetStatus(psContext, eStatus, "erasing flash");
    }

    if (cbProgress)
    {
        if (cbProgress(pvUser, "Programming", "Writing", psFWImage->u32ImageLength, 0) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
        }
    }

    for(n = 0; n < psFWImage->u32ImageLength; n += u8ChunkSize)
    {
        if((psFWImage->u32ImageLength - n) > 128)
        {
            u8ChunkSize = 128;
        }
        else
        {
            u8ChunkSize = psFWImage->u32ImageLength - n;
        }

        if((eStatus = eBL_FlashWrite(psContext, n, u8ChunkSize, psFWImage->pu8ImageData + n)) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, eStatus, "writing flash at address 0x%08X", n);;
        }
        
        if (cbProgress)
        {
            if (cbProgress(pvUser, "Programming", "Writing", psFWImage->u32ImageLength, n) != E_PRG_OK)
            {
                return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
            }
        }
    }
    
    if (cbProgress)
    {
        if (cbProgress(pvUser, "Programming", "Writing", psFWImage->u32ImageLength, psFWImage->u32ImageLength) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
        }
    }
    
    return ePRG_SetStatus(psContext, E_PRG_OK, "flash written succesfully");
}
    
    
teStatus ePRG_FlashVerify(tsPRG_Context *psContext, tcbFW_Progress cbProgress, void *pvUser)
{
    teStatus eStatus;
    int n;
    uint16_t u16FlashId;
    uint8_t u8ChunkSize;
    uint8_t au8Buffer1[BL_MAX_CHUNK_SIZE + 1];
    tsChipDetails *psChipDetails;
    tsFW_Info *psFWImage;

    if (!psContext)
    {
        return E_PRG_NULL_PARAMETER;
    }

    psChipDetails = &psContext->sChipDetails;
    psFWImage = &psContext->sFirmwareInfo;

    if (cbProgress)
    {
        if (cbProgress(pvUser, "Verifying", "Verifying", psFWImage->u32ImageLength, 0) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
        }
    }

    if ((eStatus = ePRG_SetUpImage(psContext, psFWImage, psChipDetails)) != E_PRG_OK)
    {
        return eStatus;
    }

    /* Get the flash Id */
    if((eStatus = eBL_FlashIdRead(psContext, &u16FlashId)) != E_PRG_OK)
    {
        return ePRG_SetStatus(psContext, eStatus, "reading flash ID");
    }
    LOGD("FlashId: %04x", u16FlashId);

    /* Set the flash type */
    if((eStatus = eBL_FlashSelectDevice(psContext, u16FlashId)) != E_PRG_OK)
    {
        return ePRG_SetStatus(psContext, eStatus, "selecting flash type");
    }

    /* If its not internal flash, we need to enable write access */
    if(u16FlashId != FLASH_INTERNAL)
    {
        if((eStatus = eBL_FlashStatusRegisterWrite(psContext, 0x00)) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, eStatus, "write to flash status register");
        }
    }

    for(n = 0; n < psFWImage->u32ImageLength; n += u8ChunkSize)
    {
        if((psFWImage->u32ImageLength - n) > 128)
        {
            u8ChunkSize = 128;
        }
        else
        {
            u8ChunkSize = psFWImage->u32ImageLength - n;
        }

        if ((eStatus = eBL_FlashRead(psContext, n, u8ChunkSize, au8Buffer1)) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, eStatus, "reading Flash at address 0x%08X", n);
        }
        else
        {
            if (memcmp(psFWImage->pu8ImageData + n, au8Buffer1, u8ChunkSize))
            {
                return ePRG_SetStatus(psContext, E_PRG_VERIFICATION_FAILED, "at address 0x%08X", n);
            }
        }

        if (cbProgress)
        {
            if (cbProgress(pvUser, "Verifying", "Verifying", psFWImage->u32ImageLength, n) != E_PRG_OK)
            {
                return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
            }
        }
    }

    if (cbProgress)
    {
        if (cbProgress(pvUser, "Verifying", "Verifying", psFWImage->u32ImageLength, psFWImage->u32ImageLength) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
        }
    }
    
    return ePRG_SetStatus(psContext, E_PRG_OK, "flash verified succesfully");
}


teStatus LIBPROGRAMMER ePRG_FlashDump(tsPRG_Context *psContext, char *pcDumpFile, tcbFW_Progress cbProgress, void *pvUser)
{
    FILE * iFd;
    int n;
    const uint8_t u8ChunkSize = 128;
    
    if (!psContext || !pcDumpFile)
    {
        return E_PRG_NULL_PARAMETER;
    }

//  iFd = open(pcDumpFile, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	iFd = fopen(pcDumpFile, "wb+");
    if (iFd < 0)
    {
        return ePRG_SetStatus(psContext, E_PRG_FAILED_TO_OPEN_FILE, "\"%s\" (%s)", pcDumpFile, pcPRG_GetLastErrorMessage(psContext));
    }
    
    for(n = 0; n < psContext->sChipDetails.u32FlashSize ; n += u8ChunkSize)
    {
        uint8_t au8Buffer[128];
        teStatus eStatus;
        
        if ((eStatus = eBL_FlashRead(psContext, n, u8ChunkSize, au8Buffer)) != E_PRG_OK)
        {
            fclose(iFd);
            return ePRG_SetStatus(psContext, eStatus, "reading Flash at address 0x%08X", n);
        }
        if (fwrite(au8Buffer,u8ChunkSize,1,iFd) < 0)
//        if (write(iFd, au8Buffer, u8ChunkSize) < 0)
        {
            fclose(iFd);
            return ePRG_SetStatus(psContext, E_PRG_ERROR_WRITING, "file at address 0x%08X", n);
        }
        
        if (cbProgress)
        {
            if (cbProgress(pvUser, "Dumping Flash", "Dumping Flash", psContext->sChipDetails.u32FlashSize, n) != E_PRG_OK)
            {
                fclose(iFd);
                return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
            }
        }
    }
    if (cbProgress)
    {
        if (cbProgress(pvUser, "Dumping Flash", "Dumping Flash", psContext->sChipDetails.u32FlashSize, psContext->sChipDetails.u32FlashSize) != E_PRG_OK)
        {
            fclose(iFd);
            return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
        }
    }
    
    fclose(iFd);
    return ePRG_SetStatus(psContext, E_PRG_OK, "Flash dumped succesfully");
}


teStatus ePRG_MemoryLoadExecute(tsPRG_Context *psContext, tcbFW_Progress cbProgress, tcbFW_Confirm cbConfirm, void *pvUser)
{
    teStatus eStatus;
    int n;
    uint8_t u8ChunkSize;
    tsChipDetails *psChipDetails;
    tsFW_Info *psFWImage;
    if (!psContext || !cbConfirm)
    {
        return E_PRG_NULL_PARAMETER;
    }
    psChipDetails = &psContext->sChipDetails;
    psFWImage = &psContext->sFirmwareInfo;

    if (memcmp(&psFWImage->u32ROMVersion, &psChipDetails->u32SupportedFirmware, 4) != 0)
    {
        eStatus = E_PRG_INCOMPATIBLE;
        if (cbConfirm)
        {
            eStatus = cbConfirm(pvUser, "Confirm Operation", "The loaded image does not appear to be compatible with the connected device. Continue?");
        }
        if (eStatus != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, E_PRG_INCOMPATIBLE, "image built for 0x%08X, connected device is 0x%08x", psFWImage->u32ROMVersion, psChipDetails->u32SupportedFirmware);
        }
        // User has given the go ahead to continue anyway. Here be dragons.
    }
    if ((eStatus = ePRG_SetUpImage(psContext, psFWImage, psChipDetails)) != E_PRG_OK)
    {
        return eStatus;
    }
    LOGD("Text Start 0x%08x - Length %d bytes", psFWImage->u32TextSectionLoadAddress, psFWImage->u32TextSectionLength);
    LOGD("BSS  Start 0x%08x - Length %d bytes", psFWImage->u32BssSectionLoadAddress, psFWImage->u32BssSectionLength);
    LOGD("Reset entry point 0x%08x", psFWImage->u32ResetEntryPoint);
    LOGD("Wake  entry point 0x%08x", psFWImage->u32WakeUpEntryPoint);

    LOGD("Loading .text");
    
    if (cbProgress)
    {
        if (cbProgress(pvUser, "Loading program in RAM", "Loading", psFWImage->u32TextSectionLength + psFWImage->u32BssSectionLength, 0) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
        }
    }
    /* Download text segment */
    for(n = 0; n < psFWImage->u32TextSectionLength; n += u8ChunkSize)
    {   
        uint8_t au8Buffer[BL_MAX_CHUNK_SIZE + 1];
        if((psFWImage->u32TextSectionLength - n) > BL_MAX_CHUNK_SIZE)
        {
            u8ChunkSize = BL_MAX_CHUNK_SIZE;
        }
        else
        {
            u8ChunkSize = psFWImage->u32TextSectionLength - n;
        }
        
        if((eStatus = eBL_MemoryWrite(psContext, psFWImage->u32TextSectionLoadAddress + n, u8ChunkSize, psFWImage->pu8TextData + n)) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, eStatus, "writing chunk at address 0x%08X", psFWImage->u32TextSectionLoadAddress + n);
        }

        /* Verify the memory contents */
        if ((eStatus = eBL_MemoryRead(psContext, psFWImage->u32TextSectionLoadAddress + n, u8ChunkSize, au8Buffer)) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, eStatus, "reading at address 0x%08X", psFWImage->u32TextSectionLoadAddress + n);
        }
        else
        {
            if (memcmp(psFWImage->pu8TextData + n, au8Buffer, u8ChunkSize))
            {
                return ePRG_SetStatus(psContext, E_PRG_VERIFICATION_FAILED, "at address 0x%08X", psFWImage->u32TextSectionLoadAddress + n);
            }
        }

        if (cbProgress)
        {
            if (cbProgress(pvUser, "Loading Image in RAM", "Loading", psFWImage->u32TextSectionLength + psFWImage->u32BssSectionLength, n) != E_PRG_OK)
            {
                return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
            }
        }
    }

    LOGD("Clearing .bss     "); 
    if (cbProgress)
    {
        if (cbProgress(pvUser, "Loading program in RAM", "Clearing .bss", psFWImage->u32TextSectionLength + psFWImage->u32BssSectionLength, psFWImage->u32TextSectionLength) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
        }
    }
    /* Clear BSS segment */
    {
        uint8_t au8Buffer[127];
        memset(au8Buffer, 0, sizeof(au8Buffer));
        for(n = 0; n < psFWImage->u32BssSectionLength; n += u8ChunkSize)
        {
            if((psFWImage->u32BssSectionLength - n) > sizeof(au8Buffer))
            {
                u8ChunkSize = sizeof(au8Buffer);
            }
            else
            {
                u8ChunkSize = psFWImage->u32BssSectionLength - n;
            }
            if((eStatus = eBL_MemoryWrite(psContext, psFWImage->u32BssSectionLoadAddress + n, u8ChunkSize, au8Buffer)) != E_PRG_OK)
            {
                return ePRG_SetStatus(psContext, eStatus, "writing bss at address 0x%08X", psFWImage->u32BssSectionLoadAddress + n);
            }
            
            if (cbProgress)
            {
                if (cbProgress(pvUser, "Loading program in RAM", "Clearing .bss", psFWImage->u32TextSectionLength + psFWImage->u32BssSectionLength, psFWImage->u32TextSectionLength + n) != E_PRG_OK)
                {
                    return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
                }
            }
        }
    }
    if (cbProgress)
    {
        if (cbProgress(pvUser, "Loading program in RAM", "Clearing .bss", psFWImage->u32TextSectionLength + psFWImage->u32BssSectionLength, psFWImage->u32TextSectionLength + psFWImage->u32BssSectionLength) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
        }
    }

    LOGD("Starting module application");
    
    return ePRG_SetStatus(psContext, eBL_MemoryExecute(psContext, psFWImage->u32ResetEntryPoint), "executing program in RAM");
}


teStatus ePRG_EepromErase(tsPRG_Context *psContext, teEepromErase eErase, tcbFW_Progress cbProgress, void *pvUser)
{
    int iEraseAll;
    teStatus eStatus;
    tsChipDetails *psChipDetails;
    tsPRG_PrivateContext *psPriv;    
    
    if (!psContext)
    {
        return E_PRG_NULL_PARAMETER;
    }
    psPriv = (tsPRG_PrivateContext *)psContext->pvPrivate;
    psChipDetails = &psContext->sChipDetails;
    
    if ((CHIP_ID_PART(psChipDetails->u32ChipId) & CHIP_ID_PART(CHIP_ID_JN5168)) == 0)
    {
        return ePRG_SetStatus(psContext, E_PRG_UNSUPPORED_CHIP, "");
    }
    
    switch (eErase)
    {
        case E_ERASE_EEPROM_PDM:
            iEraseAll = 0;
            break;
            
        case E_ERASE_EEPROM_ALL:
            iEraseAll = 1;
            break;
            
        case E_ERASE_EEPROM_NONE:
        default:
            return ePRG_SetStatus(psContext, E_PRG_BAD_PARAMETER, "");
    }
    if (psChipDetails->u32BootloaderVersion == 0x00080006)
    {
        /* For this bootloader we have to load an extension binary in to RAM first */
        tsPRG_Context sContext;
        uint32_t u32BaudRate;
        /* Copy context data for loading the extension programmer */
        sContext = *psContext;
        
        if ((eStatus = ePRG_FwGetInfo(&sContext, FLASHPROGRAMMEREXTENSION_JN5168_BIN)) != E_PRG_OK)
        {
            /* Error with file. FW module has displayed error so just exit. */
            return ePRG_SetStatus(psContext, eStatus, "loading extension binary");
        }
        if (cbProgress)
        {
            if (cbProgress(pvUser, "Erasing EEPROM", "Erasing EEPROM", 1, 0) != E_PRG_OK)
            {
                return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
            }
        }

        if ((eStatus = ePRG_MemoryLoadExecute(&sContext, NULL, ePRG_ConfirmAlways, NULL)) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, eStatus, "loading extension binary into RAM");
        }
        if ((eStatus = eBL_EEPROMErase(psContext, iEraseAll)) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, eStatus, "erasing EEPROM");
        }  
        if ((eStatus = eBL_MemoryExecute(psContext, JN516X_BOOTLOADER_ENTRY)) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, eStatus, "executing bootloader");
        }
        
        if (cbProgress)
        {
            if (cbProgress(pvUser, "Erasing EEPROM", "Erasing EEPROM", 1, 1) != E_PRG_OK)
            {
                return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
            }
        }
        
        u32BaudRate = psPriv->sConnection.uDetails.sSerial.u32BaudRate;

        psPriv->sConnection.uDetails.sSerial.u32BaudRate = 38400;
        
        /* change local port to default speed of built in bootloader */
        if ((eStatus = ePRG_ConnectionUartUpdate(psContext, &psPriv->sConnection)) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, eStatus, "selecting baud rate");
        }
        
        // Wait 500ms for bootloader to switch back to 38400.
        vPRG_WaitMs(500);
        
        psPriv->sConnection.uDetails.sSerial.u32BaudRate = u32BaudRate;
        
        /* Go back to selected speed */
        if ((eStatus = ePRG_ConnectionUpdate(psContext, &psPriv->sConnection)) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, eStatus, "selecting baud rate");
        }
        return ePRG_SetStatus(psContext, E_PRG_OK, "");
    }
    else
    {
        return ePRG_SetStatus(psContext, E_PRG_UNSUPPORED_CHIP, "");
    }
}


teStatus ePRG_EepromDump(tsPRG_Context *psContext, char *pcDumpFile, tcbFW_Progress cbProgress, void *pvUser)
{
    const uint8_t u8ChunkSize = 128;
    FILE * iFd;
    uint32_t n;
    teStatus eStatus;
    tsChipDetails *psChipDetails;
    tsPRG_PrivateContext *psPriv;    
    
    if (!psContext || !pcDumpFile)
    {
        return E_PRG_NULL_PARAMETER;
    }
    psPriv = (tsPRG_PrivateContext *)psContext->pvPrivate;
    psChipDetails = &psContext->sChipDetails;
    
    if ((CHIP_ID_PART(psChipDetails->u32ChipId) & CHIP_ID_PART(CHIP_ID_JN5168)) == 0)
    {
        return ePRG_SetStatus(psContext, E_PRG_UNSUPPORED_CHIP, "");
    }
    
//  iFd = open(pcDumpFile, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IRUSR | S_IWUSR);
	iFd = fopen(pcDumpFile, "wb+");
    if (iFd < 0)
    {
        return ePRG_SetStatus(psContext, E_PRG_FAILED_TO_OPEN_FILE, "\"%s\" (%s)", pcDumpFile, pcPRG_GetLastErrorMessage(psContext));
    }
    
    if (psChipDetails->u32BootloaderVersion == 0x00080006)
    {
        /* For this bootloader we have to load an extension binary in to RAM first */
        tsPRG_Context sContext;
        uint32_t u32BaudRate;
        
        /* Copy context data for loading the extension programmer */
        sContext = *psContext;
        
        if ((eStatus = ePRG_FwGetInfo(&sContext, FLASHPROGRAMMEREXTENSION_JN5168_BIN)) != E_PRG_OK)
        {
            /* Error with file. FW module has displayed error so just exit. */
            fclose(iFd);
            return ePRG_SetStatus(psContext, eStatus, "loading extension binary");
        }
        
        if ((eStatus = ePRG_MemoryLoadExecute(&sContext, NULL, ePRG_ConfirmAlways, NULL)) != E_PRG_OK)
        {
            fclose(iFd);
            return ePRG_SetStatus(psContext, eStatus, "loading extension binary into RAM");
        }
       
        for(n = 0; n < psContext->sChipDetails.u32EepromSize ; n += u8ChunkSize)
        {
            uint8_t au8Buffer[128];
            teStatus eStatus;
            
            if ((eStatus = eBL_EEPROMRead(psContext, n, u8ChunkSize, au8Buffer)) != E_PRG_OK)
            {
                fclose(iFd);
                return ePRG_SetStatus(psContext, eStatus, "reading EEPROM at address 0x%08X", n);
            }
            
            if (fwrite(au8Buffer,u8ChunkSize,1,iFd) < 0)
            {
                fclose(iFd);
                return ePRG_SetStatus(psContext, E_PRG_ERROR_WRITING, "file at address 0x%08X", n);
            }
            
            if (cbProgress)
            {
                if (cbProgress(pvUser, "Dumping EEPROM", "Dumping EEPROM", psContext->sChipDetails.u32EepromSize, n) != E_PRG_OK)
                {
                    fclose(iFd);
                    return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
                }
            }
        }
        if (cbProgress)
        {
            if (cbProgress(pvUser, "Dumping EEPROM", "Dumping EEPROM", psContext->sChipDetails.u32EepromSize, psContext->sChipDetails.u32EepromSize) != E_PRG_OK)
            {
                fclose(iFd);
                return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
            }
        }
        
        fclose(iFd);
        
        if ((eStatus = eBL_MemoryExecute(psContext, JN516X_BOOTLOADER_ENTRY)) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, eStatus, "executing bootloader");
        }
        
        u32BaudRate = psPriv->sConnection.uDetails.sSerial.u32BaudRate;

        psPriv->sConnection.uDetails.sSerial.u32BaudRate = 38400;
        
        /* change local port to default speed of built in bootloader */
        if ((eStatus = ePRG_ConnectionUartUpdate(psContext, &psPriv->sConnection)) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, eStatus, "selecting baud rate");
        }
        
        // Wait 500ms for bootloader to switch back to 38400.
        vPRG_WaitMs(500);
        
        psPriv->sConnection.uDetails.sSerial.u32BaudRate = u32BaudRate;
        
        /* Go back to selected speed */
        if ((eStatus = ePRG_ConnectionUpdate(psContext, &psPriv->sConnection)) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, eStatus, "selecting baud rate");
        }
        return ePRG_SetStatus(psContext, E_PRG_OK, "");
    }
    else
    {
        fclose(iFd);
        return ePRG_SetStatus(psContext, E_PRG_UNSUPPORED_CHIP, "");
    }
}


teStatus ePRG_EepromProgram(tsPRG_Context *psContext, char *pcLoadFile, tcbFW_Progress cbProgress, void *pvUser)
{
    uint8_t u8ChunkSize;
    FILE * iFd;
    uint32_t n;
    teStatus eStatus;
    tsChipDetails *psChipDetails;
    tsPRG_PrivateContext *psPriv;
    
    if (!psContext || !pcLoadFile)
    {
        return E_PRG_NULL_PARAMETER;
    }
    psPriv = (tsPRG_PrivateContext *)psContext->pvPrivate;
    psChipDetails = &psContext->sChipDetails;
    
    if ((CHIP_ID_PART(psChipDetails->u32ChipId) & CHIP_ID_PART(CHIP_ID_JN5168)) == 0)
    {
        return ePRG_SetStatus(psContext, E_PRG_UNSUPPORED_CHIP, "");
    }

	printf("pc LoadFile %s",pcLoadFile);
    iFd = fopen(pcLoadFile, "rb+");
    if (iFd < 0)
    {
        return ePRG_SetStatus(psContext, E_PRG_FAILED_TO_OPEN_FILE, "\"%s\" (%s)", pcLoadFile, pcPRG_GetLastErrorMessage(psContext));
    }
    
    if (psChipDetails->u32BootloaderVersion == 0x00080006)
    {
        /* For this bootloader we have to load an extension binary in to RAM first */
        tsPRG_Context sContext;
        uint32_t u32BaudRate;
        
        /* Copy context data for loading the extension programmer */
        sContext = *psContext;
        
        if ((eStatus = ePRG_FwGetInfo(&sContext, FLASHPROGRAMMEREXTENSION_JN5168_BIN)) != E_PRG_OK)
        {
            /* Error with file. FW module has displayed error so just exit. */
            fclose(iFd);
            return ePRG_SetStatus(psContext, eStatus, "loading extension binary");
        }
        
        if ((eStatus = ePRG_MemoryLoadExecute(&sContext, NULL, ePRG_ConfirmAlways, NULL)) != E_PRG_OK)
        {
            fclose(iFd);
            return ePRG_SetStatus(psContext, eStatus, "loading extension binary into RAM");
        }
       
        for(n = 0; ; n += u8ChunkSize)
        {
            uint8_t au8Buffer[128];
            teStatus eStatus;
            
            u8ChunkSize = fread(au8Buffer,128,1,iFd);
            
            if (u8ChunkSize <= 0)
            {
                LOGD("End of EEPROM load file at address 0x%08X", n);
				break;
            }
            
            if ((n + u8ChunkSize) > psChipDetails->u32EepromSize)
            {
                LOGD("File is larger than the device EEPROM", n);
                break;
            }
            
            LOGD("Write %d bytes to EEPROM address 0x%08X", u8ChunkSize, n);
            
            if ((eStatus = eBL_EEPROMWrite(psContext, n, u8ChunkSize, au8Buffer)) != E_PRG_OK)
            {
                fclose(iFd);
                return ePRG_SetStatus(psContext, eStatus, "loading EEPROM at address 0x%08X", n);
            }
            
            if (cbProgress)
            {
                if (cbProgress(pvUser, "Loading EEPROM", "Loading EEPROM", psContext->sChipDetails.u32EepromSize, n) != E_PRG_OK)
                {
                    fclose(iFd);
                    return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
                }
            }
        }
        if (cbProgress)
        {
            if (cbProgress(pvUser, "Loading EEPROM", "Loading EEPROM", psContext->sChipDetails.u32EepromSize, psContext->sChipDetails.u32EepromSize) != E_PRG_OK)
            {
                fclose(iFd);
                return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
            }
        }
        
        fclose(iFd);
        
        if ((eStatus = eBL_MemoryExecute(psContext, JN516X_BOOTLOADER_ENTRY)) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, eStatus, "executing bootloader");
        }
        
        u32BaudRate = psPriv->sConnection.uDetails.sSerial.u32BaudRate;

        psPriv->sConnection.uDetails.sSerial.u32BaudRate = 38400;
        
        /* change local port to default speed of built in bootloader */
        if ((eStatus = ePRG_ConnectionUartUpdate(psContext, &psPriv->sConnection)) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, eStatus, "selecting baud rate");
        }
        
        // Wait 500ms for bootloader to switch back to 38400.
        vPRG_WaitMs(500);
        
        psPriv->sConnection.uDetails.sSerial.u32BaudRate = u32BaudRate;
        
        /* Go back to selected speed */
        if ((eStatus = ePRG_ConnectionUpdate(psContext, &psPriv->sConnection)) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, eStatus, "selecting baud rate");
        }
        return ePRG_SetStatus(psContext, E_PRG_OK, "");
    }
    else
    {
        fclose(iFd);
        return ePRG_SetStatus(psContext, E_PRG_UNSUPPORED_CHIP, "");
    }
}


teStatus ePRG_IndexSectorWrite(tsPRG_Context *psContext, uint8_t u8Page, uint8_t u8WordNumber, uint8_t au8Data[16], tcbFW_Progress cbProgress, tcbFW_Confirm cbConfirm, void *pvUser)
{
    teStatus eStatus;
    tsChipDetails *psChipDetails;
    tsPRG_PrivateContext *psPriv;    
    
    if (!psContext)
    {
        return E_PRG_NULL_PARAMETER;
    }
    psPriv = (tsPRG_PrivateContext *)psContext->pvPrivate;
    psChipDetails = &psContext->sChipDetails;
    
    if ((CHIP_ID_PART(psChipDetails->u32ChipId) & CHIP_ID_PART(CHIP_ID_JN5168)) == 0)
    {
        return ePRG_SetStatus(psContext, E_PRG_UNSUPPORED_CHIP, "");
    }
    
    if (psChipDetails->u32BootloaderVersion == 0x00080006)
    {
        /* For this bootloader we have to load an extension binary in to RAM first */
        tsPRG_Context sContext;
        uint32_t u32BaudRate;
        
        /* Copy context data for loading the extension programmer */
        sContext = *psContext;
        
        if ((eStatus = ePRG_FwGetInfo(&sContext, FLASHPROGRAMMEREXTENSION_JN5168_BIN)) != E_PRG_OK)
        {
            /* Error with file. FW module has displayed error so just exit. */
            return ePRG_SetStatus(psContext, eStatus, "loading extension binary");
        }
        
        if ((eStatus = ePRG_MemoryLoadExecute(&sContext, cbProgress, ePRG_ConfirmAlways, pvUser)) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, eStatus, "loading extension binary into RAM");
        }
        
        if ((eStatus = eBL_IndexSectorWrite(psContext, u8Page, u8WordNumber, au8Data)) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, eStatus, "writing index sector");
        }

        if ((eStatus = eBL_MemoryExecute(psContext, JN516X_BOOTLOADER_ENTRY)) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, eStatus, "executing bootloader");
        }
        
        u32BaudRate = psPriv->sConnection.uDetails.sSerial.u32BaudRate;

        psPriv->sConnection.uDetails.sSerial.u32BaudRate = 38400;
        
        /* change local port to default speed of built in bootloader */
        if ((eStatus = ePRG_ConnectionUartUpdate(psContext, &psPriv->sConnection)) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, eStatus, "selecting baud rate");
        }
        
        // Wait 500ms for bootloader to switch back to 38400.
        vPRG_WaitMs(500);
        
        psPriv->sConnection.uDetails.sSerial.u32BaudRate = u32BaudRate;
        
        /* Go back to selected speed */
        if ((eStatus = ePRG_ConnectionUpdate(psContext, &psPriv->sConnection)) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, eStatus, "selecting baud rate");
        }
        return ePRG_SetStatus(psContext, E_PRG_OK, "");
    }
    else
    {
        return ePRG_SetStatus(psContext, E_PRG_UNSUPPORED_CHIP, "");
    }
}


teStatus ePRG_MACAddressSet(tsPRG_Context *psContext, uint8_t au8MacAddress[8], tcbFW_Progress cbProgress, tcbFW_Confirm cbConfirm, void *pvUser)
{
    tsChipDetails *psChipDetails;

    if (!psContext)
    {
        return E_PRG_NULL_PARAMETER;
    }

    psChipDetails = &psContext->sChipDetails;
    
    if (CHIP_ID_PART(psChipDetails->u32ChipId) & CHIP_ID_PART(CHIP_ID_JN5168))
    {
        uint8_t au8WriteData[16];
        //uint8_t au8MacAddress[8];
        teStatus eStatus;
        
        memcpy(&au8WriteData[0], &au8MacAddress[0], 8);
        memset(&au8WriteData[8], 0xFF, 8);
        
        /* First we read the existing customer specific MAC address, to make sure it is not already written */
        if ((eStatus = eBL_MemoryRead(psContext, JN516X_CUSTOMER_MAC_ADDRESS_LOCATION, 8, au8MacAddress)) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, eStatus, "reading MAC address");
        }
        
        if (memcmp(au8MacAddress, &au8WriteData[8], 8))
        {
            return ePRG_SetStatus(psContext, E_PRG_ERROR, "MAC address already set");
        }
        return ePRG_IndexSectorWrite(psContext, JN516X_MAC_INDEX_SECTOR_PAGE, JN516X_MAC_INDEX_SECTOR_WORD, au8WriteData, cbProgress, cbConfirm, pvUser);
    }
    else
    {
        /* On older devices, just update the MAC address in the Chip information for the next time we write to flash. */
        memcpy(psChipDetails->au8MacAddress, au8MacAddress, 8);
        return ePRG_SetStatus(psContext, E_PRG_OK, "");
    }
}


void DBG_vPrintfImpl(const char *pcFile, uint32_t u32Line, const char *pcFormat, ...)
{
    va_list argp;
    va_start(argp, pcFormat);
#ifdef DBG_VERBOSE
    printf("%s:%d ", pcFile, u32Line);
#endif /* DBG_VERBOSE */
    vprintf(pcFormat, argp);
    fflush(stdout);
    return;
}


teStatus ePRG_SetStatus(tsPRG_Context *psContext, teStatus eStatus, char *pcAdditionalInfoFmt, ...)
{
    tsPRG_PrivateContext *psPriv = (tsPRG_PrivateContext *)psContext->pvPrivate; 
    const char *pcStatus;
    uint32_t u32Length = 0; 
    va_list ap;
    va_start(ap, pcAdditionalInfoFmt);

    switch (eStatus)
    {
        case(E_PRG_OK):                     pcStatus = "Success"; break;
        case(E_PRG_ERROR):                  pcStatus = "Error"; break;
        case(E_PRG_OUT_OF_MEMORY):          pcStatus = "Low on memory"; break;
        case(E_PRG_ERROR_WRITING):          pcStatus = "Write error"; break;
        case(E_PRG_ERROR_READING):          pcStatus = "Read error"; break;
        case(E_PRG_FAILED_TO_OPEN_FILE):    pcStatus = "Failed to open file"; break;
        case(E_PRG_BAD_PARAMETER):          pcStatus = "Bad parameter"; break;
        case(E_PRG_NULL_PARAMETER):         pcStatus = "NULL parameter"; break;
        case(E_PRG_INCOMPATIBLE):           pcStatus = "Image is not compatible with chip,"; break;
        case(E_PRG_INVALID_FILE):           pcStatus = "Invalid image file"; break;
        case(E_PRG_UNSUPPORED_CHIP):        pcStatus = "Unsupported chip"; break;
        case(E_PRG_ABORTED):                pcStatus = "Aborted"; break;
        case(E_PRG_VERIFICATION_FAILED):    pcStatus = "Verification failed"; break;
        case(E_PRG_INVALID_TRANSPORT):      pcStatus = "Invalid transport"; break;
        case(E_PRG_COMMS_FAILED):           pcStatus = "Communication failure"; break;
        case(E_PRG_UNSUPPORTED_OPERATION):  pcStatus = "Bootloader doesn't support"; break;
        default:                            pcStatus = "Unknown"; break;
    }

    u32Length = _snprintf(psPriv->acStatusMessage, PRG_MAX_STATUS_LENGTH, "%s ", pcStatus);
    
    _vsnprintf(&psPriv->acStatusMessage[u32Length], PRG_MAX_STATUS_LENGTH - u32Length, pcAdditionalInfoFmt, ap);

    va_end(ap);
    return eStatus;
}

#if defined POSIX
char *pcPRG_GetLastErrorMessage(tsPRG_Context *psContext)
{
    tsPRG_PrivateContext *psPriv = (tsPRG_PrivateContext *)psContext->pvPrivate; 
    strerror_r(errno, psPriv->acErrorMsgBuffer, PRG_MAX_STATUS_LENGTH);
    return psPriv->acErrorMsgBuffer;
}
#elif defined WIN32
char *pcPRG_GetLastErrorMessage(tsPRG_Context *psContext)
{
    tsPRG_PrivateContext *psPriv = (tsPRG_PrivateContext *)psContext->pvPrivate; 
    
    DWORD dwLastError = GetLastError(); 
    DWORD dwNumChars;

    dwNumChars = FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        dwLastError,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&psPriv->acErrorMsgBuffer,
        PRG_MAX_STATUS_LENGTH,
        NULL);
    
    // Remove \n\r from end of message.
    psPriv->acErrorMsgBuffer[dwNumChars-2] = '\0';
    
    return psPriv->acErrorMsgBuffer;
}
#endif /* WIN32 */


void vPRG_WaitMs(uint32_t u32TimeoutMs)
{
    LOGD("Delay %dms...", u32TimeoutMs);
#if defined POSIX
    usleep(u32TimeoutMs * 1000);
#elif defined WIN32
    Sleep(u32TimeoutMs);
#endif
    LOGD("waited");
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

static teStatus ePRG_ChipGetChipId(tsPRG_Context *psContext)
{
    uint8_t au8Buffer[6];
    tsChipDetails *psChipDetails = &psContext->sChipDetails;
    
    LOGD("Get Chip ID");
    
    if(psChipDetails == NULL)
    {
        return E_PRG_NULL_PARAMETER;
    }

    memset(psChipDetails, 0, sizeof(tsChipDetails));

    /* Send chip id request */
    if(eBL_ChipIdRead(psContext, &psChipDetails->u32ChipId) != E_PRG_OK)
    {
        LOGD("Error reading chip id");

        /* That failed so it might be an old device that doesn't support the command, try reading it directly */
        if (eBL_MemoryRead(psContext, 0x100000FC, 4, au8Buffer) != E_PRG_OK)
        {
            LOGD("Error Reading processor ID register");
            return E_PRG_ERROR;
        }
        else
        {
            psChipDetails->u32ChipId  = au8Buffer[0] << 24;
            psChipDetails->u32ChipId |= au8Buffer[1] << 16;
            psChipDetails->u32ChipId |= au8Buffer[2] << 8;
            psChipDetails->u32ChipId |= au8Buffer[3] << 0;
        }
    }
    LOGD("Chip ID: 0x%08X", psContext->sChipDetails.u32ChipId);
    
    if (CHIP_ID_PART(psChipDetails->u32ChipId) == CHIP_ID_PART(CHIP_ID_JN5168))
    {
        LOGD("Reading 6x data");

        if (eBL_MemoryRead(psContext, JN516X_BOOTLOADER_VERSION_ADDRESS, 4, au8Buffer) != E_PRG_OK)
        {
            LOGD("Error Reading bootloader version");
            return E_PRG_ERROR;
        }
        else
        {
            psChipDetails->u32BootloaderVersion  = au8Buffer[0] << 24;
            psChipDetails->u32BootloaderVersion |= au8Buffer[1] << 16;
            psChipDetails->u32BootloaderVersion |= au8Buffer[2] << 8;
            psChipDetails->u32BootloaderVersion |= au8Buffer[3] << 0;
            
            LOGD("JN516x Bootloader version 0x%08x", psChipDetails->u32BootloaderVersion);
        }

        if (eBL_MemoryRead(psContext, JN516X_INDEX_SECTOR_DEVICE_CONFIG_ADDR, 4, au8Buffer) != E_PRG_OK)
        {
            LOGD("Error Reading config from flash index sector");
            return E_PRG_ERROR;
        }
        else
        {
            psChipDetails->u32EepromSize    = (4 * 1024) - 64; /* Final segment not usable */
            psChipDetails->u32FlashSize     = (au8Buffer[3] & 0x07) >> 0;
            psChipDetails->u32RamSize       = (au8Buffer[3] & 0x30) >> 4;
            
            psChipDetails->u32SupportedFirmware = (
                (psChipDetails->u32RamSize      << 16) |
                (psChipDetails->u32FlashSize    << 24) |
                (0x08));
            
            psChipDetails->u32RamSize       = ((psChipDetails->u32RamSize + 1) * 8) * 1024;
            psChipDetails->u32FlashSize     = ((psChipDetails->u32FlashSize + 1) * 32) * 1024;
            
            LOGD("JN516x RAM size:     %dk", psChipDetails->u32RamSize / 1024);
            LOGD("JN516x Flash size:   %dk", psChipDetails->u32FlashSize / 1024);
            LOGD("JN516x EEPROM size:  %dk", psChipDetails->u32EepromSize / 1024);
            LOGD("JN516x Bootloader version 0x%08x", psChipDetails->u32BootloaderVersion);
            LOGD("JN516x Supported firmware 0x%08x", psChipDetails->u32SupportedFirmware);
        }
    }
    else
    {
        if (eBL_MemoryRead(psContext, JN514X_ROM_ID_ADDR, 4, au8Buffer) != E_PRG_OK)
        {
            LOGD("Error Reading ROM ID");
            return E_PRG_ERROR;
        }
        else
        {
            psChipDetails->u32SupportedFirmware  = au8Buffer[0] << 24;
            psChipDetails->u32SupportedFirmware |= au8Buffer[1] << 16;
            psChipDetails->u32SupportedFirmware |= au8Buffer[2] << 8;
            psChipDetails->u32SupportedFirmware |= au8Buffer[3] << 0;
        }
    }
    
    switch (CHIP_ID(psChipDetails->u32ChipId))
    {
        case (CHIP_ID(CHIP_ID_JN5148_REV2A)):    psChipDetails->pcChipName = "JN5148-001";  break;
        case (CHIP_ID(CHIP_ID_JN5148_REV2B)):    psChipDetails->pcChipName = "JN5148-001";  break;
        case (CHIP_ID(CHIP_ID_JN5148_REV2C)):    psChipDetails->pcChipName = "JN5148-001";  break;
        case (CHIP_ID(CHIP_ID_JN5148_REV2D)):    psChipDetails->pcChipName = "JN5148J01";   break;
        case (CHIP_ID(CHIP_ID_JN5148_REV2E)):    psChipDetails->pcChipName = "JN5148Z01";   break;
        
        case (CHIP_ID(CHIP_ID_JN5142_REV1A)):    psChipDetails->pcChipName = "JN5142";      break;
        case (CHIP_ID(CHIP_ID_JN5142_REV1B)):    psChipDetails->pcChipName = "JN5142";      break;
        case (CHIP_ID(CHIP_ID_JN5142_REV1C)):    psChipDetails->pcChipName = "JN5142J01";   break;

        case (CHIP_ID(CHIP_ID_JN5168)):         
            if (psChipDetails->u32FlashSize == (64 * 1024))
            {
                                        psChipDetails->pcChipName = "JN5161";      break;
            }
            else if (psChipDetails->u32FlashSize == (160 * 1024))
            {
                                        psChipDetails->pcChipName = "JN5164";      break;
            }
            else if (psChipDetails->u32FlashSize == (256 * 1024))
            {
                                        psChipDetails->pcChipName = "JN5168";      break;
            }
            else
            {
                                        psChipDetails->pcChipName = "JN516x";      break;
            }
        
        default:                        psChipDetails->pcChipName = "Unknown";     break;
    }
    
    LOGD("Chip Name: %s", psContext->sChipDetails.pcChipName);
    
    return E_PRG_OK;
}


static teStatus ePRG_ChipGetMacAddress(tsPRG_Context *psContext)
{
    teStatus eStatus;
    uint8_t au8InvalidMac[8] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    tsChipDetails *psChipDetails = &psContext->sChipDetails;
    
    
    if(psChipDetails == NULL)
    {
        return E_PRG_NULL_PARAMETER;
    }

    switch(CHIP_ID_PART(psChipDetails->u32ChipId))
    {
        case CHIP_ID_PART(CHIP_ID_JN5148_REV2A):
            LOGD("JN5148 ");
            switch (CHIP_ID_VESION(psChipDetails->u32ChipId))
            {
                case CHIP_ID_VESION(CHIP_ID_JN5148_REV2D):
                case CHIP_ID_VESION(CHIP_ID_JN5148_REV2E):
                    LOGD("multi-image bootloader");
                    eStatus = eBL_FlashRead(psContext, JN514X_MIB_MAC_ADDRESS_LOCATION, 8, psChipDetails->au8MacAddress);
                    break;
                    
                default:
                    LOGD("single image bootloader");
                    eStatus = eBL_FlashRead(psContext, JN514X_MAC_ADDRESS_LOCATION, 8, psChipDetails->au8MacAddress);
                    break;
            }
            break;
            
        case CHIP_ID_PART(CHIP_ID_JN5142_REV1A):
        case CHIP_ID_PART(CHIP_ID_JN5142_REV1B):
        case CHIP_ID_PART(CHIP_ID_JN5142_REV1C):
            eStatus = eBL_FlashRead(psContext, JN514X_MIB_MAC_ADDRESS_LOCATION, 8, psChipDetails->au8MacAddress);
            break;
            
        case CHIP_ID_PART(CHIP_ID_JN5168):
            /* First we read the customer specific MAC address, and if its not all F's, we use that */
            eStatus = eBL_MemoryRead(psContext, JN516X_CUSTOMER_MAC_ADDRESS_LOCATION, 8, psChipDetails->au8MacAddress);
            
            LOGD("Customer MAC Address: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X", 
                psChipDetails->au8MacAddress[0] & 0xFF, psChipDetails->au8MacAddress[1] & 0xFF, psChipDetails->au8MacAddress[2] & 0xFF, psChipDetails->au8MacAddress[3] & 0xFF, 
                psChipDetails->au8MacAddress[4] & 0xFF, psChipDetails->au8MacAddress[5] & 0xFF, psChipDetails->au8MacAddress[6] & 0xFF, psChipDetails->au8MacAddress[7] & 0xFF);

            /* If its all F's, read factory assigned MAC */
            if(memcmp(psChipDetails->au8MacAddress, au8InvalidMac, 8) == 0)
            {
                LOGD("No customer MAC address - using factory");
                eStatus = eBL_MemoryRead(psContext, JN516X_MAC_ADDRESS_LOCATION, 8, psChipDetails->au8MacAddress);
                
                LOGD("Factory MAC Address:  %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X", 
                    psChipDetails->au8MacAddress[0] & 0xFF, psChipDetails->au8MacAddress[1] & 0xFF, psChipDetails->au8MacAddress[2] & 0xFF, psChipDetails->au8MacAddress[3] & 0xFF, 
                    psChipDetails->au8MacAddress[4] & 0xFF, psChipDetails->au8MacAddress[5] & 0xFF, psChipDetails->au8MacAddress[6] & 0xFF, psChipDetails->au8MacAddress[7] & 0xFF);
            }
            break;

        default:
            return E_PRG_ERROR;

    }

    return eStatus;
}


static teStatus ePRG_SetUpImage(tsPRG_Context *psContext, tsFW_Info *psFWImage, tsChipDetails *psChipDetails)
{
    /* Depending on chip type, we may need to copy the MAC address into the firmware image
     * Also, some images have a 4 byte header in the binary that needs stripping off, so do that here
     * by adjusting start point and length to suit */
    switch(CHIP_ID_PART(psChipDetails->u32ChipId))
    {
        case CHIP_ID_PART(CHIP_ID_JN5142_REV1A):
        case CHIP_ID_PART(CHIP_ID_JN5142_REV1B):
        case CHIP_ID_PART(CHIP_ID_JN5142_REV1C):
            LOGD("multi-image bootloader");
            memcpy(&psFWImage->pu8ImageData[JN514X_MIB_MAC_ADDRESS_LOCATION], psChipDetails->au8MacAddress, 8);
            break;
     
        case CHIP_ID_PART(CHIP_ID_JN5148_REV2A):
            LOGD("JN5148 ");
            switch (CHIP_ID_VESION(psChipDetails->u32ChipId))
            {
                case CHIP_ID_VESION(CHIP_ID_JN5148_REV2D):
                case CHIP_ID_VESION(CHIP_ID_JN5148_REV2E):
                    LOGD("multi-image bootloader");
                    memcpy(&psFWImage->pu8ImageData[JN514X_MIB_MAC_ADDRESS_LOCATION], psChipDetails->au8MacAddress, 8);
                    break;
                    
                default:
                    LOGD("single image bootloader");
                    memcpy(&psFWImage->pu8ImageData[JN514X_MAC_ADDRESS_LOCATION], psChipDetails->au8MacAddress, 8);
                    break;
            }
            break;
            
        case CHIP_ID_PART(CHIP_ID_JN5168):
            break;

        default:
            return ePRG_SetStatus(psContext, E_PRG_UNSUPPORED_CHIP, "");
            break;
    }
    return ePRG_SetStatus(psContext, E_PRG_OK, "");
}


static teStatus ePRG_ConfirmAlways(void *pvUser, const char *pcTitle, const char *pcText)
{
    return E_PRG_OK;
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
