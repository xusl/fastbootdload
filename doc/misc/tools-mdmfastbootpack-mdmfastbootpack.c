#include <stdio.h>
#include <memory.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fnmatch.h>
//#include <libgen.h>
#include <inttypes.h>

#define FILE_NAME_LEN                       256
#define MAX_FILE_NUM                        50
#define MAX_PATH_NUM                        6
#define IMAGE_DOS_SIGNATURE                 0x5A4D      // MZ
#define FALSE   0
#define TRUE    1
#define OUT_EXE_NAME "ADSU-adb.exe"
#define UNPACK_EXE_NAME "unPack.exe"
//typedef unsigned long       DWORD;
//typedef unsigned short      WORD;
//typedef long                LONG;
//typedef int                 BOOL;
typedef uint32_t DWORD;
typedef uint16_t WORD;
typedef int32_t  LONG;
typedef int      BOOL;
#define TYPE_FILE 0
#define TYPE_DIR  1

typedef struct _IMAGE_DOS_HEADER {      // DOS .EXE header
    WORD   e_magic;                     // Magic number
    WORD   e_cblp;                      // Bytes on last page of file
    WORD   e_cp;                        // Pages in file
    WORD   e_crlc;                      // Relocations
    WORD   e_cparhdr;                   // Size of header in paragraphs
    WORD   e_minalloc;                  // Minimum extra paragraphs needed
    WORD   e_maxalloc;                  // Maximum extra paragraphs needed
    WORD   e_ss;                        // Initial (relative) SS value
    WORD   e_sp;                        // Initial SP value
    WORD   e_csum;                      // Checksum
    WORD   e_ip;                        // Initial IP value
    WORD   e_cs;                        // Initial (relative) CS value
    WORD   e_lfarlc;                    // File address of relocation table
    WORD   e_ovno;                      // Overlay number
    WORD   e_res[4];                    // Reserved words
    WORD   e_oemid;                     // OEM identifier (for e_oeminfo)
    WORD   e_oeminfo;                   // OEM information; e_oemid specific
    WORD   e_res2[10];                  // Reserved words
    LONG   e_lfanew;                    // File address of new exe header
} IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;

typedef struct {
    char  fileName[FILE_NAME_LEN];
    DWORD offset;
    DWORD length;
} TPackFileInfo;


int add_data(const char* path, FILE* out, TPackFileInfo *info, int *index) {
    char name_temp[1024];
    DIR * dir;
    struct dirent *dir_env;
    struct stat stat_file;

    if (out == NULL || path == NULL || info == NULL || index == NULL) {
        fprintf(stderr, "invalid parameter!\n");
        return -1;
    }

    dir=opendir(path);
    if (dir == NULL) {
      fprintf(stderr, "open directory %s faild : %s.\n", path, strerror(errno));
      return -1;
    }

    while((dir_env=readdir(dir)) != NULL) {
        if(strcmp(dir_env->d_name,".")==0 || strcmp(dir_env->d_name,"..")==0) {
            //printf("skip . and ..\n");
            continue;
        }

        strcpy(name_temp,path);
        strcat(name_temp,dir_env->d_name);
        stat(name_temp,&stat_file);

        if( S_ISDIR(stat_file.st_mode)) {
            printf("recurse directory %s.\n", name_temp);
            add_data( path, out, info, index);
        } else if (S_ISREG(stat_file.st_mode) && (*index < MAX_FILE_NUM)){
            (info + *index)->offset = ftell(out);
            (info + *index)->length = stat_file.st_size;
            strcpy((info + *index)->fileName,dir_env->d_name);
            WriteFileToPackage(name_temp, out, NULL);
            *index += 1;
        } else {
          printf("%s unknow status. mode %d, index %d.\n", name_temp, stat_file.st_mode , *index);
        }
    }
    return 0;
}


DWORD getFileLength(const char* szFileName) {
    struct stat buf;
    stat(szFileName,&buf);
    return buf.st_size;
}

BOOL WriteFileToPackage(const char* path, FILE* out, TPackFileInfo *info) {
    FILE* in = NULL;
    unsigned char buffer[4096];
    size_t len;

    if (out == NULL || path == NULL ) {
        fprintf(stderr, "Invalid parameter.\n");
        return FALSE;
    }

    if (info != NULL) {
        info->offset = ftell(out);
        info->length = getFileLength(path);
        strncpy(info->fileName,(const char*)basename(path), FILE_NAME_LEN);
    }

    printf("write file %s.\n", path);

    in = fopen(path, "rb");
    if(NULL==in) {
        printf("open file %s error!\n", path);
        return FALSE;
    }

    while (!feof(in)) {
        len = fread(buffer, 1, sizeof(buffer), in);
        if (0 != ferror(in)) {
            fprintf(stderr, "read error: %d\n", ferror(in));
            break;
        }
        fwrite(buffer, 1, len, out);
        if (0 != ferror(out)) {
            fprintf(stderr, "write error: %d\n", ferror(out));
            break;
        }
    }

    fclose(in);
    return TRUE;
}

int main(int argc, char** argv) {
    TPackFileInfo pkg_info[MAX_FILE_NUM];
    const char * exe_name = NULL;
    const char * unpack_name = NULL;
    const char * data_path[MAX_PATH_NUM] = {0};
    char *path;
    FILE* out_exe;
    //int opt;
    int i;
    int num;
    int index;
    int len;

    printf("---------package exe image file begin--------------\n");
    for (i = 1, num = 0;i < argc && num < MAX_PATH_NUM;) {
      //printf("parameter begin: %c\n", *argv[i]);
      if (*argv[i] == '-') {
        i += 2;
      } else {
        data_path[num] = argv[i];
        //printf("add path %s\n", argv[i]);
        i++;
        num++;
      }
    }

    while ((i = getopt(argc, argv, "ho:u:")) != -1) {
        switch (i) {
            case 'u':
                unpack_name = optarg;
                break;
            case 'o':
                exe_name = optarg;
                break;
            default: /* '?' */
                fprintf(stderr, "Usage: %s [-o ADSU.EXE] [-u unPack.exe] \n",
                        argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    exe_name = exe_name ? exe_name : OUT_EXE_NAME;
    unpack_name = unpack_name ? unpack_name : UNPACK_EXE_NAME;

    out_exe = fopen(exe_name, "wb+");
    if(NULL==out_exe) {
        printf("create file %s error!\n", exe_name);
        return -1;
    }
    //write unpack.exe first.
    WriteFileToPackage(unpack_name, out_exe, pkg_info);
    index = 1;

    for (i = 0 ; i < num; i++ ) {
      printf("data folder %s\n", data_path[i]);
      len = strlen(data_path[i]);
      path = (char *)malloc(len + 2);
      if (path == NULL) {
        fprintf(stderr, "out of memory.\n");
        continue;
      }

      strcpy(path, data_path[i]);
      if (path[len - 1] != '/') {
          strcat(path, "/*");
          len += 2;
      } else {
          strcat(path, "*");
          len ++;
      }

      if (0 == fnmatch(path, exe_name, FNM_PATHNAME|FNM_PERIOD)) {
        fprintf(stderr, "folder %s contain output execute file %s, so ignore it.\n", data_path[i], exe_name);
      } else {
        path[len -1] = '\0';
        add_data(path, out_exe, pkg_info, &index);
      }
      free(path);
    }


    /*write file position info.*/
    DWORD posDataStart = getFileLength(exe_name);
    DWORD bytes = fwrite(pkg_info, 1, sizeof(pkg_info), out_exe);
    if(bytes!=sizeof(pkg_info)) {
        printf("error, dBytesWrite!=sizeof(pkg_info)(%d!=%d).\n", bytes, sizeof(pkg_info));
    }

    rewind(out_exe);

    IMAGE_DOS_HEADER imgHeader;
    bytes = fread(&imgHeader, 1, sizeof(IMAGE_DOS_HEADER), out_exe);
    if(bytes!=sizeof(IMAGE_DOS_HEADER)) {
        fprintf(stderr, "error, dImgFileHeadLength!=sizeof(IMAGE_DOS_HEADER):(%d!=%d)\n",
                bytes, sizeof(IMAGE_DOS_HEADER));
    }

    // magic number 'MZ', so this file is a EXE.
    if (imgHeader.e_magic == IMAGE_DOS_SIGNATURE) {
        memset(imgHeader.e_res2, 0, sizeof(imgHeader.e_res2));
        memcpy(imgHeader.e_res2, &posDataStart, sizeof(DWORD));
        // re-write image DOS header
        fseek(out_exe, 0, SEEK_SET);
        fwrite(&imgHeader, 1, sizeof(IMAGE_DOS_HEADER), out_exe);
    } else {
        printf("imgHeader.e_magic=%X\n", imgHeader.e_magic);
        printf("not a exe file for windows.\n");
    }

    fclose(out_exe);
    printf("---------------package exe image file end-------------------\n");
    return 0;
}
