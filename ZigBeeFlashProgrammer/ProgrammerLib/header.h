
typedef unsigned long long     uint64;
typedef signed char       int8;         /* 8 bit signed */
typedef unsigned char     uint8;      /* 8 bit unsigned */
typedef short             int16;              /* 16 bit signed */
typedef unsigned short    uint16;    /* 16 bit unsigned */
typedef int               int32;                /* 32 bit signed */
typedef unsigned int      uint32;      /* 32 bit unsigned */
#define word		uint16
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
typedef _int64 uint64_t;

#define VERSION


#define FALSE   0
#define TRUE    1

#ifndef WIN32
#ifdef LIBPROGRAMMER_BUILD
#define LIBPROGRAMMER __declspec(dllexport)
#else
#define LIBPROGRAMMER __declspec(dllimport)
#endif /* LIBPROGRAMMER_BUILD */

#else /* WIN32 */

#define LIBPROGRAMMER
#define FREE_IF(x)  do {\
                      if ((x) != NULL) {\
                        free((x));\
                        (x) = NULL;\
                      } \
                    }while(0)


#define DELETE_IF(x)  do {\
                      if ((x) != NULL) {\
                        delete (x);\
                        (x) = NULL;\
                      } \
                    }while(0)

#endif /* WIN32 */