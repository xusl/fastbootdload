
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

//#include <stdint.h>
//#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
//#include <termios.h>

//#include <sys/select.h>
//#include <sys/signal.h>
#include <sys/types.h>
//#include <sys/time.h>
//#include <sys/ioctl.h>

#include <errno.h>

//#include <linux/types.h>

#ifdef FTDI
#include <ftdi.h>
#include <libusb-1.0/libusb.h>
#endif /* FTDI */

#include "programmer.h"

#include "programmer_private.h"
#include "uart.h"
//#include "dbg.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

#ifdef DEBUG_UART
#define TRACE_UART	TRUE
#else
#define TRACE_UART	FALSE
#endif

#define MAX_ERROR_STRING_LEN

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

typedef struct
{
    tsPRG_PrivateContext sPriv;
#ifdef FTDI
    char *pcDevice;
#endif /* FTDI */
    int iFileDescriptor;
    uint32_t u32BaudRate;
} tsCommsPrivate;

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

#ifdef FTDI
teStatus ePRG_FTDI_ModeProgramming(tsCommsPrivate *psCommsPriv);
teStatus ePRG_FTDI_ModeRunning(tsCommsPrivate *psCommsPriv);
#endif /* FTDI */


/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

static struct termios sOptions;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/


/* For now only UART devices are supported. */
teStatus ePRG_ConnectionListInit(tsPRG_Context *psContext, uint32_t *pu32NumConnections, tsConnection **pasConnections)
{
    if ((psContext == NULL) || (pu32NumConnections == NULL) || (pasConnections == NULL))
    {
        return E_PRG_NULL_PARAMETER;
    }
    
    *pu32NumConnections = 0;
    
    return E_PRG_OK;
}


teStatus ePRG_ConnectionListDestroy(tsPRG_Context *psContext, uint32_t u32NumConnections, tsConnection **pasConnections)
{
    return E_PRG_OK;
}


/****************************************************************************
 *
 * NAME: UART_eInitialise
 *
 * DESCRIPTION:
 * Initialise a UART
 *
 * RETURNS:
 * teStatus
 *
 ****************************************************************************/

teStatus ePRG_ConnectionUartOpen(tsPRG_Context *psContext, tsConnection *psConnection)
{
    tsCommsPrivate *psCommsPriv;
    
    //DBG_vPrintf(TRACE_UART, "Using UART device %s\n", psConnection->pcName);
    
    psCommsPriv = realloc(psContext->pvPrivate, sizeof(tsCommsPrivate));
    if (!psCommsPriv)
    {
        return E_PRG_OUT_OF_MEMORY;
    }
    psContext->pvPrivate = psCommsPriv;
    
    memset(psCommsPriv, 0, sizeof(tsCommsPrivate));
    memcpy(&psCommsPriv->sPriv.sConnection, psConnection, sizeof(tsConnection));

    psCommsPriv->sPriv.sConnection.pcName = strdup(psConnection->pcName);
    if (!psCommsPriv->sPriv.sConnection.pcName)
    {
        return ePRG_SetStatus(psContext, E_PRG_OUT_OF_MEMORY, "");
    }
    
#ifdef FTDI    
    if (ePRG_FTDI_ModeProgramming(psCommsPriv) != E_PRG_OK)
    {
        //DBG_vPrintf(TRACE_UART, "Could not set FTDI into programming mode\n");
    }
    else
    {
        /* If we managed to reset the device correctly, 
         * the bootloader will now be running at 1Mbaud for 100ms 
         * - wait for it to return to normal speed */
        vPRG_WaitMs(200);
    }
#endif /* FTDI */

    //open the device
    psCommsPriv->iFileDescriptor = open(psConnection->pcName, O_RDWR | O_NOCTTY);
    
    if (psCommsPriv->iFileDescriptor < 0)
    {
        return ePRG_SetStatus(psContext, E_PRG_ERROR, "opening device %s (%s)\n", psConnection->pcName, pcPRG_GetLastErrorMessage(psContext));
    }

    if (tcgetattr(psCommsPriv->iFileDescriptor, &sOptions) == -1)
    {
        return ePRG_SetStatus(psContext, E_PRG_ERROR, "opening device %s (%s)\n", psConnection->pcName, pcPRG_GetLastErrorMessage(psContext));
    }

    sOptions.c_iflag &= ~(INPCK | ISTRIP | INLCR | IGNCR | ICRNL | IUCLC | IXON | IXANY | IXOFF);
    sOptions.c_iflag = IGNBRK | IGNPAR;
    sOptions.c_oflag &= ~(OPOST | OLCUC | ONLCR | OCRNL | ONOCR | ONLRET);
    sOptions.c_cflag &= ~(CSIZE | CSTOPB | PARENB | CRTSCTS);
    sOptions.c_cflag |= CS8 | CREAD | HUPCL | CLOCAL;
    sOptions.c_lflag &= ~(ISIG | ICANON | ECHO | IEXTEN);

    fcntl(psCommsPriv->iFileDescriptor, F_SETFL, O_NONBLOCK);
    
    psCommsPriv->sPriv.sConnection.eType = E_CONNECT_SERIAL;
    
    return ePRG_ConnectionUartUpdate(psContext, psConnection);
}
 
 
/****************************************************************************
 *
 * NAME: ePRG_UartClose
 *
 * DESCRIPTION:
 * Close the specified UART
 *
 * RETURNS:
 * teStatus
 *
 ****************************************************************************/
teStatus ePRG_ConnectionUartClose(tsPRG_Context *psContext)
{
    tsCommsPrivate *psCommsPriv;

    if(psContext == NULL)
    {
        return E_PRG_NULL_PARAMETER;
    }
    psCommsPriv = (tsCommsPrivate *)psContext->pvPrivate;

    close(psCommsPriv->iFileDescriptor);
    
    free(psCommsPriv->sPriv.sConnection.pcName);
    
    return E_PRG_OK;
}


/****************************************************************************
 *
 * NAME: eUART_SetBaudRate
 *
 * DESCRIPTION:
 * Sets the baud rate of the specified UART
 *
 * RETURNS:
 * teStatus
 *
 ****************************************************************************/
teStatus ePRG_ConnectionUartUpdate(tsPRG_Context *psContext, tsConnection *psConnection)
{
    tsCommsPrivate *psCommsPriv;
    int iBaud;
    
    if(psContext == NULL)
    {
        return E_PRG_NULL_PARAMETER;
    }
    psCommsPriv = (tsCommsPrivate *)psContext->pvPrivate;

    //DBG_vPrintf(TRACE_UART, "Changing baud rate to %d\n", psConnection->uDetails.sSerial.u32BaudRate);

    switch (psConnection->uDetails.sSerial.u32BaudRate)
    {
    
    case 38400:     iBaud = B38400;
    	break;

    case 115200:    iBaud = B115200;
		break;

    case 230400:    iBaud = B230400;
		break;

    case 460800:    iBaud = B460800;
		break;

    case 500000:    iBaud = B500000;
		break;

    case 921600:    iBaud = B921600;
    	break;

    case 1000000:   iBaud = B1000000;
        break;
        
    default:
        //DBG_vPrintf(TRACE_UART, "Unsupported baud rate: %d\n", psConnection->uDetails.sSerial.u32BaudRate);
        return E_PRG_BAD_PARAMETER;
    }       
    
    if(tcflush(psCommsPriv->iFileDescriptor, TCIOFLUSH) == -1)
    {
        return ePRG_SetStatus(psContext, E_PRG_ERROR, "flushing uart (%s)\n", pcPRG_GetLastErrorMessage(psContext));
    }

    if(cfsetispeed(&sOptions, iBaud) == -1)
    {
        return ePRG_SetStatus(psContext, E_PRG_ERROR, "setting input speed (%s)\n", pcPRG_GetLastErrorMessage(psContext));
    }

    if(cfsetospeed(&sOptions, iBaud) == -1)
    {
        return ePRG_SetStatus(psContext, E_PRG_ERROR, "setting output speed (%s)\n", pcPRG_GetLastErrorMessage(psContext));
    }

    if(tcsetattr(psCommsPriv->iFileDescriptor, TCSANOW, &sOptions) == -1)
    {
        return ePRG_SetStatus(psContext, E_PRG_ERROR, "changing port settings (%s)\n", pcPRG_GetLastErrorMessage(psContext));
    }
    
    psCommsPriv->sPriv.sConnection.uDetails.sSerial.u32BaudRate = psConnection->uDetails.sSerial.u32BaudRate;
    
    return ePRG_SetStatus(psContext, E_PRG_OK, "");
}


/****************************************************************************
 *
 * NAME: eUART_Flush
 *
 * DESCRIPTION:
 * Flush the specified UART
 *
 * RETURNS:
 * teStatus
 *
 ****************************************************************************/
teStatus eUART_Flush(tsPRG_Context *psContext)
{
    tsCommsPrivate *psCommsPriv;
	uint8_t u8Data;
	int iBytesRead;
    
    if(psContext == NULL)
    {
        return E_PRG_NULL_PARAMETER;
    }
    psCommsPriv = (tsCommsPrivate *)psContext->pvPrivate;
    
    if (psCommsPriv->sPriv.sConnection.eType != E_CONNECT_SERIAL)
    {
        return ePRG_SetStatus(psContext, E_PRG_INVALID_TRANSPORT, "");
    }

    if(tcflush(psCommsPriv->iFileDescriptor, TCIOFLUSH) != 0)
    {
    	return ePRG_SetStatus(psContext, E_PRG_ERROR, "%s", strerror(errno));
    }

    do
    {
        eUART_Read(psContext, 100, 1, &u8Data, &iBytesRead);
    } while(iBytesRead > 0);


	return ePRG_SetStatus(psContext, E_PRG_OK, "");

}


/****************************************************************************
 *
 * NAME: eUART_Read
 *
 * DESCRIPTION:
 * Reads from the specified UART
 *
 * RETURNS:
 * teStatus
 *
 ****************************************************************************/
teStatus eUART_Read(tsPRG_Context *psContext, int iTimeoutMicroseconds, int iBufferLen, uint8_t *pu8Buffer, int *piBytesRead)
{
    tsCommsPrivate *psCommsPriv;
    int         		iResult = -1;
    struct timeval      sTimeout;
    fd_set              sDescriptors;
    int                 iMaxFd;
    
    if((pu8Buffer == NULL) || (psContext == NULL) || (piBytesRead == NULL))
    {
        return E_PRG_NULL_PARAMETER;
    }
    psCommsPriv = (tsCommsPrivate *)psContext->pvPrivate;
    
    if (psCommsPriv->sPriv.sConnection.eType != E_CONNECT_SERIAL)
    {
        return ePRG_SetStatus(psContext, E_PRG_INVALID_TRANSPORT, "");
    }
    
    *piBytesRead = 0;
    
    sTimeout.tv_usec = iTimeoutMicroseconds % 1000000;
    sTimeout.tv_sec = iTimeoutMicroseconds / 1000000;
    
    FD_ZERO(&sDescriptors);
    iMaxFd = psCommsPriv->iFileDescriptor;
    FD_SET(psCommsPriv->iFileDescriptor, &sDescriptors);
    
    iResult = select(iMaxFd + 1, &sDescriptors, NULL, NULL, &sTimeout);
    if (iResult < 0)
    {
        return ePRG_SetStatus(psContext, E_PRG_ERROR_READING, "(%s)", strerror(errno));
    }
    else if (iResult == 0)
    {
        return ePRG_SetStatus(psContext, E_PRG_ERROR_READING, "timeout");
    }
    else
    {
        if (FD_ISSET(psCommsPriv->iFileDescriptor, &sDescriptors))
        {
            iResult = read(psCommsPriv->iFileDescriptor, (uint8_t*)pu8Buffer, iBufferLen);
            if (iResult > 0)
            {
                *piBytesRead = iResult;
            }
            else
            {
                iResult = 0;
            }
        }
        else
        {
            return ePRG_SetStatus(psContext, E_PRG_ERROR_READING, "no data");
        }
    }
    
    return ePRG_SetStatus(psContext, E_PRG_OK, "");
}
 

/****************************************************************************
 *
 * NAME: eUART_Write
 *
 * DESCRIPTION:
 * Write to the specified UART
 *
 * RETURNS:
 * teStatus
 *
 ****************************************************************************/
teStatus eUART_Write(tsPRG_Context *psContext, uint8_t *pu8Data, int iLength)
{
    tsCommsPrivate *psCommsPriv;
    int iBytesWritten;
    int iTotalBytesWritten = 0;
    
    if((pu8Data == NULL) || (psContext == NULL))
    {
        return E_PRG_NULL_PARAMETER;
    }
    psCommsPriv = (tsCommsPrivate *)psContext->pvPrivate;
    
    if (psCommsPriv->sPriv.sConnection.eType != E_CONNECT_SERIAL)
    {
        return ePRG_SetStatus(psContext, E_PRG_INVALID_TRANSPORT, "");
    }

    do
    {
        iBytesWritten = write(psCommsPriv->iFileDescriptor, &pu8Data[iTotalBytesWritten], iLength - iTotalBytesWritten);
        if(iBytesWritten < 0)
        {
            if(errno != EAGAIN)
            {
                return ePRG_SetStatus(psContext, E_PRG_ERROR_WRITING, "(%s)", strerror(errno));
            }
        }
        else
        {
        	iTotalBytesWritten += iBytesWritten;
        }
    } while(iTotalBytesWritten < iLength);

    return ePRG_SetStatus(psContext, E_PRG_OK, "");
    
}


/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

#ifdef FTDI

teStatus ePRG_FTDI_ModeProgramming(tsCommsPrivate *psCommsPriv)
{
    const int aaiBitModes[4][2] = 
    {
        { 0xC0, 0x20 },
        { 0xC4, 0x20 },
        { 0xCC, 0x20 },
        { 0x00, 0x20 },
    };
    int i;
    int iFd;
    int iResult;
    struct ftdi_context *psFTDI;
    char acSerialSysFSPath[1024];
    
    if ((psFTDI = ftdi_new()) == 0)
    {
        fprintf(stderr, "ftdi_new failed\n");
        return E_PRG_ERROR;
    }
    
    _snprintf(acSerialSysFSPath, 1024, "/sys/bus/usb-serial/devices/%s/serial_no", strstr(psCommsPriv->pcDevice, "tty"));
    iFd = open(acSerialSysFSPath, O_RDONLY);
    
    //DBG_vPrintf(TRACE_UART, "Open \"%s\": %d\n", acSerialSysFSPath, iFd);
    
    if (iFd)
    {
        /* Great - we can read the serial number */
        char acSerialNo[64];
        
        memset(acSerialNo, 0, sizeof(acSerialNo));
        
        if (read(iFd, acSerialNo, sizeof(acSerialNo)) > 0)
        {
            //DBG_vPrintf(TRACE_UART, "Serial number: %s\n", acSerialNo);
            
            iResult = ftdi_usb_open_desc  (psFTDI, 0x0403, 0x6001, NULL, acSerialNo);   
            
            //DBG_vPrintf(TRACE_UART, "Open FTDI device: %d(%s)\n", iResult, ftdi_get_error_string (psFTDI));
            
            if (iResult == 0)
            {
                for (i = 0; i < (sizeof(aaiBitModes) / sizeof(aaiBitModes[0])); i++)
                {
                    //DBG_vPrintf(TRACE_UART, "Setting bit mode 0x%02X, 0x%02X\n", aaiBitModes[i][0], aaiBitModes[i][1]);
                    if (ftdi_set_bitmode(psFTDI, aaiBitModes[i][0], aaiBitModes[i][1]))
                    {
                        //DBG_vPrintf(TRACE_UART, "Error bitbanging device\n");
                        break;
                    }
                    vPRG_WaitMs(10);
                }                
            }
            ftdi_disable_bitbang(psFTDI);
            
            // Now we need to re-attach the kernel driver... which isn't present in libusb / libFTDI :-(
            libusb_reset_device(psFTDI->usb_dev);
            //DBG_vPrintf(TRACE_UART, "Reset device: %s\n",ftdi_get_error_string (psFTDI));
        }
        close(iFd);
    }
    ftdi_free(psFTDI);
    
    return E_PRG_ERROR;
}

#endif /* FTDI */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
