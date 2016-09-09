
//
// source released under European Union Public License
//
#include <StdAfx.h>
//#include "headers.h"
#include "settings.h"
#include "Tftp.h"

void ScanDir ( int (*f)(char *s, DWORD dw), DWORD dwParam, const char *szDirectory);
int IsValidDirectory (const char *path);


int IsValidDirectory (const char *path)
{

int Rc ;
   Rc = GetFileAttributes (path) ;
   return Rc == INVALID_FILE_ATTRIBUTES ? FALSE : Rc & FILE_ATTRIBUTE_DIRECTORY ;

} // IsValidDirectory


////////////////////////
// Creates a line of the dir.txt file
// use a callback function as argument since ScanDir is used either to
// create dir.txt or to dispaly the dir window

void ScanDir ( int (*f)(char *s, DWORD dw), DWORD dwParam, const char *szDirectory)
{
WIN32_FIND_DATA  FindData;
FILETIME    FtLocal;
SYSTEMTIME  SysTime;
char        szLine [256], szFileSpec [_MAX_PATH + 5];
char        szDate [sizeof "jj/mm/aaaa"];
HANDLE      hFind;

    szFileSpec [_MAX_PATH - 1] = 0;
    lstrcpyn (szFileSpec, szDirectory, _MAX_PATH);
    lstrcat (szFileSpec, "\\*.*");
    hFind = FindFirstFile (szFileSpec, &FindData);
    if (hFind !=  INVALID_HANDLE_VALUE)
    do
    {
       // display only files, skip directories
       if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)  continue;
       FileTimeToLocalFileTime (& FindData.ftCreationTime, & FtLocal);
       FileTimeToSystemTime (& FtLocal, & SysTime);
       GetDateFormat (LOCALE_SYSTEM_DEFAULT,
                      DATE_SHORTDATE,
                      & SysTime,
                      NULL,
                      szDate, sizeof szDate);
       szDate [sizeof "jj/mm/aaaa" - 1]=0;    // truncate date
       FindData.cFileName[62] = 0;      // truncate file name if needed
	   // dialog structure allow up to 64 char
       wsprintf (szLine, "%s\t%s\t%d",
                 FindData.cFileName, szDate, FindData.nFileSizeLow);

       (*f) (szLine, dwParam);
    }
    while (FindNextFile (hFind, & FindData));

    FindClose (hFind);

}  // ScanDir



//////////////////////////////////////////
// creates dir.txt files
//////////////////////////////////////////
static int CbkWrite (char *szLine, DWORD dw)
{
DWORD Dummy;
static char EOL [] = "\r\n";
       WriteFile ((HANDLE) dw, szLine, lstrlen (szLine), &Dummy, NULL);
       WriteFile ((HANDLE) dw, EOL, sizeof (EOL)-1, &Dummy, NULL);
       return 0;
}

int CreateIndexFile (void)
{
HANDLE           hDirFile;
static int       Semaph=0;
char szDirFile [_MAX_PATH];

   if (Semaph++!=0)  return 0;

   wsprintf (szDirFile, "%s\\%s", sSettings.szWorkingDirectory, DIR_TEXT_FILE);
    hDirFile =  CreateFile (szDirFile,
                            GENERIC_WRITE,
                            FILE_SHARE_READ,
                            NULL,
                            CREATE_ALWAYS,
                            FILE_ATTRIBUTE_ARCHIVE | FILE_FLAG_SEQUENTIAL_SCAN ,
                            NULL);
    if (hDirFile == INVALID_HANDLE_VALUE) return 0;
    // Walk through directory
    ScanDir (CbkWrite, (DWORD) hDirFile, sSettings.szWorkingDirectory);
    CloseHandle (hDirFile);
    Semaph = 0;
return 1;
} // CreateIndexFile
