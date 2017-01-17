#pragma once
#include <string>
#include "ConfigIni.h"
using namespace std;

class CMiniHttpDownloadServer {
public:
    CMiniHttpDownloadServer(AppConfig *appConfig, u_short port=80);
    ~CMiniHttpDownloadServer();

    void server_listen(const char*path );
    void StartHttpServer(LPCWSTR path);
    void StopHttpServer() {
      m_ServerWork = FALSE;
    }

private:
    char const* BuildHttpServerResponse(const char *path, size_t  *contentLength);
    char const* BuildHttpServerResponse(LPCWSTR fn, unsigned *_sz);
    void HandleServerException(CString msg, SOCKET sockConn, SOCKET sockSrv, const char ** ppContent);
    BOOL BuildUpdateCommand(CString file, CString &cmd);
    void UpdateMessage(CString errormsg);

    void gmt_time_string(char *buf, size_t buf_len);

private:
    AppConfig *mAppConfig;
    BOOL m_ServerWork;
    u_short m_ServerPort;
};