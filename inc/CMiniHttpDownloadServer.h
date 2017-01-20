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
    BOOL IsServerWorks() { return m_ServerWork;}

    void StartHttpServer();
    void StopHttpServer() {
      m_ServerWork = FALSE;
    }

private:
    BOOL SendFile(SOCKET sock, CString &file, int uiPort);
    BOOL ParseRequest(string& request, CString &sendFile, int *uiPort);
    BOOL BuildHttpHeader(string& header, int dataSize);
    //char const* BuildHttpServerResponse(const char *path, size_t  *contentLength);
    char const* BuildHttpServerResponse(LPCWSTR fn, unsigned *_sz);
    void HandleServerException(CString msg, SOCKET sockConn, SOCKET sockSrv, const char ** ppContent);
    BOOL BuildUpdateCommand(CString file, CString &cmd);
    void UpdateMessage(int uiPort, CString errormsg);

    void gmt_time_string(char *buf, size_t buf_len);
    //BOOL FindFile(CString fileName, CString &filePath);

private:
    PVOID m_CallBackData;
    HttpServerGetFile m_GetFileCB;
    HttpServerMessage m_SetMsgCB;
    BOOL m_ServerWork;
    u_short m_ServerPort;
};