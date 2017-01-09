#include <stdio.h>
#include "utils.h"

/*
 * Eject using SCSI SG_IO commands. Return 1 if successful, 0 otherwise.
 */
#ifndef Q_OS_WIN32      //Mac OS or Linux OS
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <fcntl.h>

typedef struct sg_iovec /* same structure as used by readv() Linux system */
{                       /* call. It defines one scatter-gather element. */
     void  *iov_base;      /* Starting address  */
     size_t iov_len;             /* Length in bytes  */
} sg_iovec_t;


typedef struct sg_io_hdr
{
     int interface_id;           /* [i] 'S' for SCSI generic (required) */
     int dxfer_direction;        /* [i] data transfer direction  */
     unsigned char cmd_len;      /* [i] SCSI command length ( <= 16 bytes) */
     unsigned char mx_sb_len;    /* [i] max length to write to sbp */
     unsigned short iovec_count; /* [i] 0 implies no scatter gather */
     unsigned int dxfer_len;     /* [i] byte count of data transfer */
     void  *dxferp;        /* [i], [*io] points to data transfer memory
                                               or scatter gather list */
     unsigned char  *cmdp; /* [i], [*i] points to command to perform */
     void  *sbp;           /* [i], [*o] points to sense_buffer memory */
     unsigned int timeout;       /* [i] MAX_UINT->no timeout (unit: millisec) */
     unsigned int flags;         /* [i] 0 -> default, see SG_FLAG... */
     int pack_id;                /* [i->o] unused internally (normally) */
     void  * usr_ptr;      /* [i->o] unused internally */
     unsigned char status;       /* [o] scsi status */
     unsigned char masked_status;/* [o] shifted, masked scsi status */
     unsigned char msg_status;   /* [o] messaging level data (optional) */
     unsigned char sb_len_wr;    /* [o] byte count actually written to sbp */
     unsigned short host_status; /* [o] errors from host adapter */
     unsigned short driver_status;/* [o] errors from software driver */
     int resid;                  /* [o] dxfer_len - actual_transferred */
     unsigned int duration;      /* [o] time taken by cmd (unit: millisec) */
     unsigned int info;          /* [o] auxiliary information */
} sg_io_hdr_t;  /* 64 bytes long (on i386) */

/* Yields max scatter gather tablesize allowed by current host adapter */
#define SG_GET_SG_TABLESIZE 0x227F  /* 0 implies can't do scatter gather */
#define SG_GET_VERSION_NUM  0x2282  /* Example: version 2.1.34 yields 20134 */

/* synchronous SCSI command ioctl, (only in version 3 interface) */
#define SG_IO 0x2285   /* similar effect as write() followed by read() */

/* Use negative values to flag difference from original sg_header structure */
#define SG_DXFER_NONE (-1)          /* e.g. a SCSI Test Unit Ready command */
#define SG_DXFER_TO_DEV (-2)        /* e.g. a SCSI WRITE command */
#define SG_DXFER_FROM_DEV (-3)      /* e.g. a SCSI READ command */
#define SG_DXFER_TO_FROM_DEV (-4)   /* treated like SG_DXFER_FROM_DEV with the*/

#define START_STOP              0x1b
#define ALLOW_MEDIUM_REMOVAL    0xa7

#ifndef _IO
#define _IO(x,y)       (((x)<<8)|y)
#endif

#define BLKRRPART  _IO(0x12,95) /* re-read partition table */

int EjectScsi(int fd)
{
    int status, k;
    sg_io_hdr_t io_hdr;
    unsigned char allowRmBlk[6] = {ALLOW_MEDIUM_REMOVAL, 0, 0, 0, 0, 0};
    unsigned char startStop1Blk[6] = {START_STOP, 0, 0, 0, 1, 0};
    unsigned char startStop2Blk[6] = {START_STOP, 0, 0, 0, 2, 0};
    unsigned char inqBuff[2];
    unsigned char sense_buffer[32];

    if ((ioctl(fd, SG_GET_VERSION_NUM, &k) < 0) || (k < 30000)) {
      printf("not an sg device, or old sg driver\n");
      return 0;
    }

    memset(&io_hdr, 0, sizeof(sg_io_hdr_t));
    io_hdr.interface_id = 'S';
    io_hdr.cmd_len = 6;
    io_hdr.mx_sb_len = sizeof(sense_buffer);
    io_hdr.dxfer_direction = SG_DXFER_NONE;
    io_hdr.dxfer_len = 0;
    io_hdr.dxferp = inqBuff;
    io_hdr.sbp = sense_buffer;
    io_hdr.timeout = 2000;

    io_hdr.cmdp = allowRmBlk;
    status = ioctl(fd, SG_IO, (void *)&io_hdr);
    if (status < 0)
        return 0;

    io_hdr.cmdp = startStop1Blk;
    status = ioctl(fd, SG_IO, (void *)&io_hdr);
    if (status < 0)
        return 0;

    io_hdr.cmdp = startStop2Blk;
    status = ioctl(fd, SG_IO, (void *)&io_hdr);
    if (status < 0)
        return 0;

    /* force kernel to reread partition table when new disc inserted */
    status = ioctl(fd, BLKRRPART);
    return 1;
}

#endif

#ifdef Q_OS_WIN32

#include "tchar.h"

#endif
