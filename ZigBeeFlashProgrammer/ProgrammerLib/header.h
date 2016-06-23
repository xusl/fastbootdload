
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

#endif /* WIN32 */