/*=============================================================================
DESC:

CHANGE HISTORY:
when        who        what
----------  ---------  --------------------------------------------------------
2009-04-10  dawang.xu  Init first version

=============================================================================*/
#ifndef __MEMMGR_H__
#define __MEMMGR_H__

#define FILE_LINE __FILE__,__LINE__
#include "log.h"

/* Allocate sizeof(type) size of memory for "ptr".
 * Use "RELEASE" to release the memory.
 * Make sure "ptr == NULL" before allocating memory.
 */
#define NEW(ptr, type)                                        \
do {                                                          \
	ASSERT(ptr == NULL);                                      \
	ptr = new type;                                           \
	if ((ptr) != NULL) {                                      \
		MEMDUMP(/*FILE_LINE,*/ "New " #type " object at 0x%p, size = %d bytes",\
			ptr, sizeof(type));                               \
	}                                                         \
} while (0)

/* Allocate sizeof(type) size of memory for "ptr", then
 * cast the pointer type to "cast_type".
 * Use "RELEASE" to release the memory.
 * Make sure "ptr == NULL" before allocating memory.
 */
#define NEW_CAST(ptr, type, cast_type)                        \
do {                                                          \
	ASSERT(ptr == NULL);                                      \
	ptr = reinterpret_cast<cast_type*>(new type);             \
} while (0)

/* Allocate count*sizeof(type) size of memory for "ptr".
 * Use "RELEASE_ARRAY" to release the memory.
 * Make sure "ptr == NULL" before allocating memory.
 */
#define NEW_ARRAY(ptr, type, count)                           \
do {                                                          \
	ASSERT(ptr == NULL);                                      \
	ptr = new type[count];                                    \
	if ((ptr) != NULL) {                                      \
		memset(ptr, 0, count*sizeof(type));                   \
		MEMDUMP(/*FILE_LINE,*/ "New " #type " array object at 0x%p, size = %d bytes", \
			ptr, count*sizeof(type));                         \
	}                                                         \
} while (0)

/* Allocate count*sizeof(type) size of memory for "ptr", then
 * cast the pointer type to "cast_type".
 * Use "RELEASE_ARRAY" to release the memory.
 * Make sure "ptr == NULL" before allocating memory.
*/
#define NEW_ARRAY_CAST(ptr, type, count, cast_type)           \
do {                                                          \
	ASSERT(ptr == NULL);                                      \
	ptr = reinterpret_cast<cast_type*>(new type[count]);      \
	if ((ptr) != NULL) {                                      \
		memset(ptr, 0, count*sizeof(type));                   \
		MEMDUMP(/*FILE_LINE,*/ "New " #type " array->" #cast_type " object at 0x%p, size = %d bytes",  \
			ptr, count*sizeof(type));                         \
	}                                                         \
} while (0)

/* Release memory previously allocated to "*pptr".
 */
#define RELEASE(pptr)                                         \
do {                                                          \
	if ((pptr) && *(pptr)) {                                  \
		MEMDUMP(/*FILE_LINE,*/ "Delete object at 0x%p", *(pptr)); \
		delete (*(pptr));                                     \
		*(pptr) = NULL;                                       \
	}                                                         \
} while (0)

/* Release (array) memory previously allocated to "*pptr".
 */
#define RELEASE_ARRAY(pptr)                                   \
do {                                                          \
	if ((pptr) && *(pptr)) {                                  \
		MEMDUMP(/*FILE_LINE,*/ "Delete array object at 0x%p", *(pptr));  \
		delete [] *(pptr);                                    \
		*(pptr) = NULL;                                       \
	}                                                         \
} while (0)

#endif //__MEMMGR_H__

