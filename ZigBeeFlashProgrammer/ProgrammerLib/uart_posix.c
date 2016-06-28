
/****************************************************************************
 *
 * MODULE:             JN51xx Programmer
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

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <glob.h>
#include <dlfcn.h>
#include <pthread.h>

#include <sys/select.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include <errno.h>

#include <linux/types.h>

#ifdef ENABLE_FTDI
#include <ftdi.h>
#include <libusb.h>
#endif /* ENABLE_FTDI */

#include <programmer.h>

#include "programmer_private.h"
#include "uart.h"
#include "dbg.h"

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
    int iFileDescriptor;
    uint32_t u32BaudRate;
#ifdef ENABLE_FTDI
    struct ftdi_context *psFTDI;
#endif /* ENABLE_FTDI */    
} tsCommsPrivate;

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

#ifdef ENABLE_FTDI
struct ftdi_context *psPRG_FTDI_GetContext(void);
void vLoad_FTDI_Library(void) ;
teStatus ePRG_FTDI_ModeProgramming(tsCommsPrivate *psCommsPriv);
teStatus ePRG_FTDI_ModeRunning(tsCommsPrivate *psCommsPriv);
#endif /* ENABLE_FTDI */


/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

static struct termios sOptions;

#ifdef ENABLE_FTDI
static pthread_once_t   ftdi_load_control = PTHREAD_ONCE_INIT;
static void             *pvFtdiLibrary    = NULL; 

static struct ftdi_context *(*dl_ftdi_new)(
    void
    );
    
static int (*dl_ftdi_usb_find_all)(
    struct ftdi_context *ftdi,
    struct ftdi_device_list **devlist,
    int vendor,
    int product
    );
    
static int (*dl_ftdi_usb_get_strings)(
    struct ftdi_context *ftdi,
    struct libusb_device *dev,
    char *manufacturer,
    int mnf_len,
    char *description,
    int desc_len,
    char *serial,
    int serial_len
    );

static void (*dl_ftdi_list_free)(
    struct ftdi_device_list **devlist
    );
    
static void (*dl_ftdi_free)(
    struct ftdi_context *ftdi
    );
    
static int (*dl_ftdi_usb_open_desc_index)(
    struct ftdi_context *ftdi,
    int vendor,
    int product,
    const char *description,
    const char *serial,
    unsigned int index
    );
    
static int (*dl_ftdi_set_latency_timer)(
    struct ftdi_context *ftdi,
    unsigned char latency
    );
    
static int (*dl_ftdi_usb_close)(
    struct ftdi_context *ftdi
    );
    
static void (*dl_ftdi_deinit)(
    struct ftdi_context *ftdi
    );

static int (*dl_ftdi_set_baudrate)(
    struct ftdi_context *ftdi,
    int baudrate
    );

static char *(*dl_ftdi_get_error_string)(
    struct ftdi_context *ftdi
    );
    
static int (*dl_ftdi_usb_purge_buffers)(
    struct ftdi_context *ftdi
    );
    
static int (*dl_ftdi_read_data)(
    struct ftdi_context *ftdi,
    unsigned char *buf,
    int size
    );
    
static int (*dl_ftdi_write_data)(
    struct ftdi_context *ftdi,
    const unsigned char *buf,
    int size
    );
    
static int (*dl_ftdi_set_bitmode)(
    struct ftdi_context *ftdi,
    unsigned char bitmask,
    unsigned char mode
    );
 
static int (*dl_ftdi_disable_bitbang)(
    struct ftdi_context *ftdi
    );
#endif
    
/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

int globerr(const char *path, int eerrno)
{
    fprintf(stderr, "%s: %s\n", path, strerror(eerrno));
    return 0;   /* let glob() keep going */
}

/* For now only UART devices are supported. */
teStatus ePRG_ConnectionListInit(tsPRG_Context *psContext, uint32_t *pu32NumConnections, tsConnection **pasConnections)
{
    int i;
    glob_t sGlob_results;
    
    if ((psContext == NULL) || (pu32NumConnections == NULL) || (pasConnections == NULL))
    {
        return E_PRG_NULL_PARAMETER;
    }
    DBG_vPrintf(TRACE_UART, "Finding tty devices\n");
    
    *pu32NumConnections = 0;
    
    if (glob("/dev/tty*", 0, globerr, &sGlob_results) == 0)
    {
        tsConnection *asNewConnections;

        DBG_vPrintf(TRACE_UART, "Found %d tty devices\n", sGlob_results.gl_pathc);
        
        asNewConnections = realloc(*pasConnections, (sGlob_results.gl_pathc) * sizeof(tsConnection));
        if (!asNewConnections)
        {
            return ePRG_SetStatus(psContext, E_PRG_OUT_OF_MEMORY, "");
        }
        *pasConnections = asNewConnections;
        
        for (i = 0; i < sGlob_results.gl_pathc; i++)
        {
            asNewConnections[*pu32NumConnections].eType   = E_CONNECT_SERIAL;
            asNewConnections[*pu32NumConnections].pcName  = strdup(sGlob_results.gl_pathv[i]);
            (*pu32NumConnections)++;
        }
    }
    
    globfree(&sGlob_results);
    
#ifdef ENABLE_FTDI
    {
        struct ftdi_context *psFTDI;
        struct ftdi_device_list *psFTDIdevices = NULL;
        struct ftdi_device_list *psFTDIdevice;
        int iNumFTDIs;
        tsConnection *asNewConnections;
        
        if ((psFTDI = psPRG_FTDI_GetContext()) == 0)
        {
            DBG_vPrintf(TRACE_UART, "Failed to get FTDI context\n");
            return E_PRG_OK;
        }
        
        iNumFTDIs = dl_ftdi_usb_find_all(psFTDI, &psFTDIdevices, 0x0403, 0x6001);
        DBG_vPrintf(TRACE_UART, "Found %d FTDI devices\n", iNumFTDIs);
        
        asNewConnections = realloc(*pasConnections, ((*pu32NumConnections) + iNumFTDIs) * sizeof(tsConnection));
        if (!asNewConnections)
        {
            return ePRG_SetStatus(psContext, E_PRG_OUT_OF_MEMORY, "");
        }
        *pasConnections = asNewConnections;
        
        for (i = 0, psFTDIdevice = psFTDIdevices; 
             (i < iNumFTDIs) && (psFTDIdevice != NULL); 
             i++, psFTDIdevice = psFTDIdevice->next)
        {
            char acManufacturer[128];
            char acDescription[128];
            char acSerial[128];
            
            if (dl_ftdi_usb_get_strings(psFTDI, psFTDIdevice->dev,
                acManufacturer, sizeof(acManufacturer),
                acDescription, sizeof(acDescription),
                acSerial, sizeof(acSerial)) < 0)
            {
                DBG_vPrintf(TRACE_UART, "Error retrieving USB device information\n");
                continue;
            }

            asNewConnections[*pu32NumConnections].eType   = E_CONNECT_SERIAL;
            asNewConnections[*pu32NumConnections].pcName  = strdup(acSerial);
            (*pu32NumConnections)++;
        }
        
        dl_ftdi_list_free(&psFTDIdevices);
        dl_ftdi_free(psFTDI);
    }
#endif /* ENABLE_FTDI */
    
    return E_PRG_OK;
}


teStatus ePRG_ConnectionListDestroy(tsPRG_Context *psContext, uint32_t u32NumConnections, tsConnection **pasConnections)
{
    int i;
    
    if ((psContext == NULL) || (pasConnections == NULL))
    {
        return E_PRG_NULL_PARAMETER;
    }
    
    for (i = 0; i < u32NumConnections; i++)
    {
        free((*pasConnections)[i].pcName);
    }
    
    free(*pasConnections);
    *pasConnections = NULL;
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
    struct stat sStat;
#ifdef ENABLE_FTDI
    int ftdi_ret;
#endif
    
    DBG_vPrintf(TRACE_UART, "Using UART device %s\n", psConnection->pcName);
    
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

    if (stat(psConnection->pcName, &sStat) < 0)
    {
        DBG_vPrintf(TRACE_UART, "Could not stat file\n");
#ifdef ENABLE_FTDI   
        DBG_vPrintf(TRACE_UART, "Checking for FTDI serial number\n");
    
        if ((psCommsPriv->psFTDI = psPRG_FTDI_GetContext()) == 0)
        {
            return ePRG_SetStatus(psContext, E_PRG_ERROR, "libFTDI error");
        }
        
        ftdi_ret = dl_ftdi_usb_open_desc_index(psCommsPriv->psFTDI, 0x0403, 0x6001, NULL, psConnection->pcName, 0);
        if (ftdi_ret != 0)
        {
            dl_ftdi_free(psCommsPriv->psFTDI);
            psCommsPriv->psFTDI = NULL;
            DBG_vPrintf(TRACE_UART, "Could not open as FTDI device (%i)\n", ftdi_ret);
            return ePRG_SetStatus(psContext, E_PRG_ERROR, "Could not open serial port - not an FTDI device");
        }
        
        DBG_vPrintf(TRACE_UART, "FTDI device serial %s opened\n", psConnection->pcName);
        
        /* Set up libusb to automatically re-attach the kernel driver */
        /* Why doesn't libFTDI do this itself? I mean why.... */
#ifdef LIBUSB_AUTO_DETACH
        if (libusb_set_auto_detach_kernel_driver(psCommsPriv->psFTDI->usb_dev, 1) < 0)
        {
            DBG_vPrintf(TRACE_UART, "Error setting auto attach kernel driver\n");
        }
#endif
        
        if (psContext->sFlags.bAutoProgramReset)
        {
            if (ePRG_FTDI_ModeProgramming(psCommsPriv) != E_PRG_OK)
            {
                DBG_vPrintf(TRACE_UART, "Could not set FTDI into programming mode\n");
            }
            else
            {
                /* If we managed to reset the device correctly, 
                * the bootloader will now be running at 1Mbaud for 100ms 
                * - wait for it to return to normal speed */
                vPRG_WaitMs(200);
            }
        }
        
        if (dl_ftdi_set_latency_timer(psCommsPriv->psFTDI, 2) != 0)
        {
            DBG_vPrintf(TRACE_UART, "Error setting latency timer\n");
        }
#else
        return ePRG_SetStatus(psContext, E_PRG_ERROR, "Could not open serial port");
#endif /* ENABLE_FTDI */
    }
    else
    {
        if (!S_ISCHR(sStat.st_mode))
        {
            return ePRG_SetStatus(psContext, E_PRG_ERROR, "Could not open serial port - not a character device");
        }
        
        //open the device
        psCommsPriv->iFileDescriptor = open(psConnection->pcName, O_RDWR | O_NOCTTY);
        
        if (psCommsPriv->iFileDescriptor < 0)
        {
            return ePRG_SetStatus(psContext, E_PRG_ERROR, "%s", pcPRG_GetLastErrorMessage(psContext));
        }

        if (tcgetattr(psCommsPriv->iFileDescriptor, &sOptions) == -1)
        {
            return ePRG_SetStatus(psContext, E_PRG_ERROR, "%s", pcPRG_GetLastErrorMessage(psContext));
        }

        sOptions.c_iflag &= ~(INPCK | ISTRIP | INLCR | IGNCR | ICRNL | IUCLC | IXON | IXANY | IXOFF);
        sOptions.c_iflag = IGNBRK | IGNPAR;
        sOptions.c_oflag &= ~(OPOST | OLCUC | ONLCR | OCRNL | ONOCR | ONLRET);
        sOptions.c_cflag &= ~(CSIZE | CSTOPB | PARENB | CRTSCTS);
        sOptions.c_cflag |= CS8 | CREAD | HUPCL | CLOCAL;
        sOptions.c_lflag &= ~(ISIG | ICANON | ECHO | IEXTEN);

        fcntl(psCommsPriv->iFileDescriptor, F_SETFL, O_NONBLOCK);    
    }
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
#ifndef LIBUSB_AUTO_DETACH
    int interface;
    libusb_device *usb_dev;
    libusb_device_handle *usb_dev_handle;
#endif
    tsCommsPrivate *psCommsPriv;

    if(psContext == NULL)
    {
        return E_PRG_NULL_PARAMETER;
    }
    psCommsPriv = (tsCommsPrivate *)psContext->pvPrivate;
#ifdef ENABLE_FTDI
    if (psCommsPriv->psFTDI)
    {
        if (psContext->sFlags.bAutoProgramReset)
        {
            if (ePRG_FTDI_ModeRunning(psCommsPriv) != E_PRG_OK)
            {
                DBG_vPrintf(TRACE_UART, "Could not set FTDI into rumnning mode\n");
            }
        }
        
#ifndef LIBUSB_AUTO_DETACH
        /* Capture our own reference to the FTDI usb device */
        usb_dev = libusb_get_device(psCommsPriv->psFTDI->usb_dev);
        libusb_ref_device(usb_dev);
        interface = psCommsPriv->psFTDI->interface;
#endif

        if (dl_ftdi_usb_close(psCommsPriv->psFTDI) < 0)
        {
            DBG_vPrintf(TRACE_UART, "Error closing FTDI device\n");
        }

#ifndef LIBUSB_AUTO_DETACH
        /* Use the captured reference to temporarily re-open the device
           and re-attach the kernel driver */
        if (libusb_open(usb_dev, &usb_dev_handle) < 0)
        {
            DBG_vPrintf(TRACE_UART, "Error opening libusb device\n");
        }
        else
        {
            if (libusb_attach_kernel_driver(usb_dev_handle, interface) < 0)
            {
                DBG_vPrintf(TRACE_UART, "Failed to re-attach kernel driver\n");
            }

            libusb_close(usb_dev_handle);
        }
        libusb_unref_device(usb_dev);
#endif

        dl_ftdi_deinit(psCommsPriv->psFTDI);
        dl_ftdi_free(psCommsPriv->psFTDI);
    }
    else
#endif /* ENABLE_FTDI */
    {
        close(psCommsPriv->iFileDescriptor);
    }
    
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
    teStatus eStatus;
    
    if(psContext == NULL)
    {
        return E_PRG_NULL_PARAMETER;
    }
    psCommsPriv = (tsCommsPrivate *)psContext->pvPrivate;

    DBG_vPrintf(TRACE_UART, "Changing baud rate to %d\n", psConnection->uDetails.sSerial.u32BaudRate);
    
#ifdef ENABLE_FTDI
    if (psCommsPriv->psFTDI)
    {
        if (dl_ftdi_set_baudrate(psCommsPriv->psFTDI, psConnection->uDetails.sSerial.u32BaudRate) < 0)
        {
            return ePRG_SetStatus(psContext, E_PRG_ERROR, "changing port settings (%s)\n", dl_ftdi_get_error_string(psCommsPriv->psFTDI));
        }
        DBG_vPrintf(TRACE_UART, "Baud rate changed\n");
    }
    else
#endif /* ENABLE_FTDI */
    {
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
            DBG_vPrintf(TRACE_UART, "Unsupported baud rate: %d\n", psConnection->uDetails.sSerial.u32BaudRate);
            return E_PRG_BAD_PARAMETER;
        }
        
        eStatus = eUART_Flush(psContext);
        if (eStatus != E_PRG_OK)
        {
            return eStatus;
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
    }
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
    
#ifdef ENABLE_FTDI
    if (psCommsPriv->psFTDI)
    {
        if (dl_ftdi_usb_purge_buffers(psCommsPriv->psFTDI) < 0)
        {
            DBG_vPrintf(TRACE_UART, "Error purging TX / RX buffers\n");
            return ePRG_SetStatus(psContext, E_PRG_ERROR, "flushing uart (%s)\n", pcPRG_GetLastErrorMessage(psContext));
        }
    }
    else
#endif /* ENABLE_FTDI */
    {
        if(tcflush(psCommsPriv->iFileDescriptor, TCIOFLUSH) != 0)
        {
            return ePRG_SetStatus(psContext, E_PRG_ERROR, "%s", strerror(errno));
        }

        do
        {
            eUART_Read(psContext, 100, 1, &u8Data, &iBytesRead);
        } while(iBytesRead > 0);
    }

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
    int         iResult = -1;

    struct timeval      sTimeout;
#ifdef ENABLE_FTDI
    struct timeval      sNow;
#endif
    fd_set              sDescriptors;
    int                 iMaxFd;
    
    sTimeout.tv_usec = iTimeoutMicroseconds % 1000000;
    sTimeout.tv_sec = iTimeoutMicroseconds / 1000000;
    
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
    
#ifdef ENABLE_FTDI
    if (psCommsPriv->psFTDI)
    {
        int iBytesRead;
        psCommsPriv->psFTDI->usb_read_timeout = iTimeoutMicroseconds / 1000;
        
        if (gettimeofday(&sNow, NULL) < 0)
        {
            return ePRG_SetStatus(psContext, E_PRG_ERROR, "gettimeofday");
        }
        timeradd(&sNow, &sTimeout, &sTimeout);

        do
        {
            // Check time first - don't want this error if we read the right number of bytes
            if (timercmp(&sNow, &sTimeout, >))
            {
                return ePRG_SetStatus(psContext, E_PRG_ERROR_READING, "no data");
            }
            
            iBytesRead = dl_ftdi_read_data(psCommsPriv->psFTDI, &pu8Buffer[*piBytesRead], iBufferLen - *piBytesRead);
            DBG_vPrintf(TRACE_UART, "Read %d bytes\n", iBytesRead);
            if (iBytesRead < 0)
            {
                DBG_vPrintf(TRACE_UART, "Error reading from FTDI device\n");
            }
            *piBytesRead += iBytesRead;
            
            if (gettimeofday(&sNow, NULL) < 0)
            {
                return ePRG_SetStatus(psContext, E_PRG_ERROR, "gettimeofday");
            }
        } while (*piBytesRead < iBufferLen);
    }
    else
#endif /* ENABLE_FTDI */
    {   
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
#ifdef ENABLE_FTDI
        if (psCommsPriv->psFTDI)
        {
            iBytesWritten = dl_ftdi_write_data(psCommsPriv->psFTDI, &pu8Data[iTotalBytesWritten], iLength - iTotalBytesWritten);
        }
        else
#endif /* ENABLE_FTDI */
        {
            iBytesWritten = write(psCommsPriv->iFileDescriptor, &pu8Data[iTotalBytesWritten], iLength - iTotalBytesWritten);
        }
        
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

#ifdef ENABLE_FTDI

struct ftdi_context *psPRG_FTDI_GetContext(void)
{
    (void) pthread_once(&ftdi_load_control, vLoad_FTDI_Library);
    
    if (pvFtdiLibrary != NULL) {
        return dl_ftdi_new();
    }
    
    return 0;
}

void vLoad_FTDI_Library(void)
{
    char *error;
    void *pvLibrary;
    
    /* Attempt to dynamically load libftdi */
    pvLibrary = dlopen("libftdi1.so", RTLD_NOW);
    
    if (pvLibrary == NULL) {
        DBG_vPrintf(TRACE_UART, "Failed to load FTDI library: %s\n", dlerror());
        return;
    }
    
    dlerror();
    *(void **) (&dl_ftdi_new) = dlsym(pvLibrary, "ftdi_new");
    if ((error = dlerror()) != NULL)  {
        DBG_vPrintf(TRACE_UART, "%s\n", error);
        return;
    }
    
    dlerror();
    *(void **) (&dl_ftdi_usb_find_all) = dlsym(pvLibrary, "ftdi_usb_find_all");
    if ((error = dlerror()) != NULL)  {
        DBG_vPrintf(TRACE_UART, "%s\n", error);
        return;
    }

    dlerror();
    *(void **) (&dl_ftdi_usb_get_strings) = dlsym(pvLibrary, "ftdi_usb_get_strings");
    if ((error = dlerror()) != NULL)  {
        DBG_vPrintf(TRACE_UART, "%s\n", error);
        return;
    }

    dlerror();
    *(void **) (&dl_ftdi_list_free) = dlsym(pvLibrary, "ftdi_list_free");
    if ((error = dlerror()) != NULL)  {
        DBG_vPrintf(TRACE_UART, "%s\n", error);
        return;
    }

    dlerror();
    *(void **) (&dl_ftdi_free) = dlsym(pvLibrary, "ftdi_free");
    if ((error = dlerror()) != NULL)  {
        DBG_vPrintf(TRACE_UART, "%s\n", error);
        return;
    }

    dlerror();
    *(void **) (&dl_ftdi_usb_open_desc_index) = dlsym(pvLibrary, "ftdi_usb_open_desc_index");
    if ((error = dlerror()) != NULL)  {
        DBG_vPrintf(TRACE_UART, "%s\n", error);
        return;
    }

    dlerror();
    *(void **) (&dl_ftdi_set_latency_timer) = dlsym(pvLibrary, "ftdi_set_latency_timer");
    if ((error = dlerror()) != NULL)  {
        DBG_vPrintf(TRACE_UART, "%s\n", error);
        return;
    }

    dlerror();
    *(void **) (&dl_ftdi_usb_close) = dlsym(pvLibrary, "ftdi_usb_close");
    if ((error = dlerror()) != NULL)  {
        DBG_vPrintf(TRACE_UART, "%s\n", error);
        return;
    }

    dlerror();
    *(void **) (&dl_ftdi_deinit) = dlsym(pvLibrary, "ftdi_deinit");
    if ((error = dlerror()) != NULL)  {
        DBG_vPrintf(TRACE_UART, "%s\n", error);
        return;
    }

    dlerror();
    *(void **) (&dl_ftdi_set_baudrate) = dlsym(pvLibrary, "ftdi_set_baudrate");
    if ((error = dlerror()) != NULL)  {
        DBG_vPrintf(TRACE_UART, "%s\n", error);
        return;
    }

    dlerror();
    *(void **) (&dl_ftdi_get_error_string) = dlsym(pvLibrary, "ftdi_get_error_string");
    if ((error = dlerror()) != NULL)  {
        DBG_vPrintf(TRACE_UART, "%s\n", error);
        return;
    }

    dlerror();
    *(void **) (&dl_ftdi_usb_purge_buffers) = dlsym(pvLibrary, "ftdi_usb_purge_buffers");
    if ((error = dlerror()) != NULL)  {
        DBG_vPrintf(TRACE_UART, "%s\n", error);
        return;
    }

    dlerror();
    *(void **) (&dl_ftdi_read_data) = dlsym(pvLibrary, "ftdi_read_data");
    if ((error = dlerror()) != NULL)  {
        DBG_vPrintf(TRACE_UART, "%s\n", error);
        return;
    }

    dlerror();
    *(void **) (&dl_ftdi_write_data) = dlsym(pvLibrary, "ftdi_write_data");
    if ((error = dlerror()) != NULL)  {
        DBG_vPrintf(TRACE_UART, "%s\n", error);
        return;
    }

    dlerror();
    *(void **) (&dl_ftdi_set_bitmode) = dlsym(pvLibrary, "ftdi_set_bitmode");
    if ((error = dlerror()) != NULL)  {
        DBG_vPrintf(TRACE_UART, "%s\n", error);
        return;
    }

    dlerror();
    *(void **) (&dl_ftdi_disable_bitbang) = dlsym(pvLibrary, "ftdi_disable_bitbang");
    if ((error = dlerror()) != NULL)  {
        DBG_vPrintf(TRACE_UART, "%s\n", error);
        return;
    }
    
    pvFtdiLibrary = pvLibrary;
}

teStatus ePRG_FTDI_ModeProgramming(tsCommsPrivate *psCommsPriv)
{
    const int aaiBitModes[4][2] = 
    {
        { 0xC0, 0x20 }, /* Drive reset and program low */
        { 0xC4, 0x20 }, /* Drive reset high */
        { 0xCC, 0x20 }, /* Drive program high */
        { 0x0C, 0x20 }, /* Release reset and program */
    };
    int i;
    
    if (psCommsPriv->psFTDI)
    {
        for (i = 0; i < (sizeof(aaiBitModes) / sizeof(aaiBitModes[0])); i++)
        {
            DBG_vPrintf(TRACE_UART, "Setting bit mode 0x%02X, 0x%02X\n", aaiBitModes[i][0], aaiBitModes[i][1]);
            if (dl_ftdi_set_bitmode(psCommsPriv->psFTDI, aaiBitModes[i][0], aaiBitModes[i][1]))
            {
                DBG_vPrintf(TRACE_UART, "Error bitbanging device\n");
                return E_PRG_ERROR;
            }
            vPRG_WaitMs(10);
        }
        DBG_vPrintf(TRACE_UART, "FTDI set into programming mode\n");
        dl_ftdi_disable_bitbang(psCommsPriv->psFTDI);
        return E_PRG_OK;
    }
    
    return E_PRG_ERROR;
}


teStatus ePRG_FTDI_ModeRunning(tsCommsPrivate *psCommsPriv)
{
    const int aaiBitModes[3][2] = 
    {
        { 0x40, 0x20 },
        { 0x00, 0x20 },
        { 0x00, 0x00 }, /* Normal bit mode */
    };
    int i;
    
    if (psCommsPriv->psFTDI)
    {
        if (dl_ftdi_usb_purge_buffers(psCommsPriv->psFTDI) < 0)
        {
            DBG_vPrintf(TRACE_UART, "Error purging TX / RX buffers\n");
            return E_PRG_ERROR;
        }
        
        for (i = 0; i < (sizeof(aaiBitModes) / sizeof(aaiBitModes[0])); i++)
        {
            DBG_vPrintf(TRACE_UART, "Setting bit mode 0x%02X, 0x%02X\n", aaiBitModes[i][0], aaiBitModes[i][1]);
            if (dl_ftdi_set_bitmode(psCommsPriv->psFTDI, aaiBitModes[i][0], aaiBitModes[i][1]))
            {
                DBG_vPrintf(TRACE_UART, "Error bitbanging device\n");
                return E_PRG_ERROR;
            }
            vPRG_WaitMs(10);
        }
        
        DBG_vPrintf(TRACE_UART, "FTDI set into running mode\n");
        dl_ftdi_disable_bitbang(psCommsPriv->psFTDI);
        return E_PRG_OK;
    }
    
    return E_PRG_ERROR;
}

#endif /* ENABLE_FTDI */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
