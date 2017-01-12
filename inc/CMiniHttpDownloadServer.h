#pragma once
#include <string>
using namespace std;
class CMiniHttpDownloadServer {
public:
    CMiniHttpDownloadServer();
    ~CMiniHttpDownloadServer();

    void gmt_time_string(char *buf, size_t buf_len);
    void server_listen(u_short port);
    char const* BuildHttpServerResponse(const char *path, size_t  *contentLength);
    void HandleServerException(CString msg, SOCKET sockConn, SOCKET sockSrv, const char ** ppContent);
    BOOL BuildUpdateCommand(CString file, CString &cmd);
    void UpdateMessage(CString errormsg);
};