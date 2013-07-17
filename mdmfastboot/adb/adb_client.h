#ifndef _ADB_CLIENT_H_
#define _ADB_CLIENT_H_

#include "adb.h"

#ifdef __cplusplus
extern "C" {
#endif

/* connect to adb, connect to the named service, and return
** a valid fd for interacting with that service upon success
** or a negative number on failure
*/
int adb_connect(const char *service);
int _adb_connect(const char *service);

/* connect to adb, connect to the named service, return 0 if
** the connection succeeded AND the service returned OKAY
*/
int adb_command(const char *service);

/* connect to adb, connect to the named service, return
** a malloc'd string of its response upon success or NULL
** on failure.
*/
char *adb_query(const char *service);

/* Set the preferred transport to connect to.
*/
void adb_set_transport(transport_type type, const char* serial);

/* Set TCP specifics of the transport to use
*/
void adb_set_tcp_specifics(int server_port);

/* return verbose error string from last operation */
const char *adb_error(void);

/* read a standard adb status response (OKAY|FAIL) and
** return 0 in the event of OKAY, -1 in the event of FAIL
** or protocol error
*/
int adb_status(int fd);

#ifdef __cplusplus
}
#endif

#endif
