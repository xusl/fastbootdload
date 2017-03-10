#pragma once
#include <string>
#include "ConfigIni.h"
using namespace std;

typedef BOOL ( *HttpServerGetFile) (PVOID data, string filename, CString& filePath);
typedef VOID ( *HttpServerMessage) (PVOID data, int uiPort, CString message);

class CMiniHttpDownloadServer {
public:
    CMiniHttpDownloadServer(PVOID data, HttpServerGetFile getFile, HttpServerMessage msg, u_short port=80);
    ~CMiniHttpDownloadServer();

    //void server_listen(const char*path );
    BOOL IsServerWorks() {     
    	EnterCriticalSection(&mStatusCs);
  		LeaveCriticalSection(&mStatusCs);
      return m_ServerWork;
    }
    static UINT StartHttpServer(LPVOID wParam);
    void StopHttpServer();
    u_short GetPort() { return m_ServerPort; }

private:
    BOOL BindPort(SOCKET sock, u_short port);
    BOOL SendFile(SOCKET sock, CString &file, int uiPort);
    BOOL ParseRequest(string& request, CString &sendFile, int *uiPort);
    BOOL BuildHttpHeader(string& header, int dataSize);
    //char const* BuildHttpServerResponse(const char *path, size_t  *contentLength);
    char const* BuildHttpServerResponse(LPCWSTR fn, unsigned *_sz);
    void HandleServerException(CString msg, SOCKET sockConn, SOCKET sockSrv, const char ** ppContent);
    //BOOL BuildUpdateCommand(CString file, CString &cmd);
    void UpdateMessage(int uiPort, CString errormsg);
    void Run();
    void gmt_time_string(char *buf, size_t buf_len);
    //BOOL FindFile(CString fileName, CString &filePath);
    
private:
    PVOID m_CallBackData;
    HttpServerGetFile m_GetFileCB;
    HttpServerMessage m_SetMsgCB;
    BOOL m_ServerWork;
    u_short m_ServerPort;
    CRITICAL_SECTION mStatusCs;
    //CWinThread* mThread;
};