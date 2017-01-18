#include "StdAfx.h"
#include "utils.h"
#include "log.h"
#include "CMiniHttpDownloadServer.h"

CMiniHttpDownloadServer::CMiniHttpDownloadServer(AppConfig *appConfig, u_short port):
    m_ServerWork (FALSE),
    m_ServerPort(port),
    mAppConfig(appConfig)
{
    ;
}

CMiniHttpDownloadServer::~CMiniHttpDownloadServer() {
    ;
}

void CMiniHttpDownloadServer::UpdateMessage(CString errormsg) {
    ;
}

void CMiniHttpDownloadServer::HandleServerException(CString msg, SOCKET sockConn, SOCKET sockSrv, const char ** ppContent) {
    UpdateMessage(msg);
    //        wprintf(L"socket failed with error: %ld\n", WSAGetLastError());
    if (ppContent != NULL && *ppContent != NULL) {
        free((void *)*ppContent);
        *ppContent = NULL;
    }
    if (INVALID_SOCKET != sockConn)
        closesocket(sockConn);
    if (INVALID_SOCKET != sockSrv)
        closesocket(sockSrv);
}

 void CMiniHttpDownloadServer::gmt_time_string(char *buf, size_t buf_len)
 {
  struct tm newtime;
   __int64 ltime;
   errno_t err;

   _time64( &ltime );
      // Obtain coordinated universal time:
   err = _gmtime64_s( &newtime, &ltime );
   if (err)
   {
      LOGE("Invalid Argument to _gmtime64_s.");
   }
      // Convert to an ASCII representation
   err = asctime_s(buf, buf_len, &newtime);
   if (err)
   {
      LOGE("Invalid Argument to asctime_s.");
   }

 // time_t curtime = time(NULL);
//  strftime(buf, buf_len, "%a, %d %b %Y %H:%M:%S GMT", gmtime_s(&curtime));
  strftime(buf, buf_len, "%a, %d %b %Y %H:%M:%S GMT", &newtime);
  //CString date = CTime::GetCurrentTime().Format(_T("%a, %d %b %Y %H:%M:%S GMT"));
}

BOOL CMiniHttpDownloadServer::FindFile(CString fileName, CString &filePath)
{
    BOOL             found = FALSE;
    HANDLE           hFind;
    WIN32_FIND_DATA  FindData;
    wchar_t          szFileSpec [_MAX_PATH + 5];
    const wchar_t    *szDirectory = mAppConfig->GetPkgDir();

    szFileSpec [_MAX_PATH - 1] = 0;
    lstrcpyn (szFileSpec, szDirectory, _MAX_PATH);
//    lstrcat (szFileSpec, "\\*.*");
    lstrcat (szFileSpec, _T("\\*\\"));
    lstrcat (szFileSpec, fileName);
    hFind = FindFirstFile (szFileSpec, &FindData);
    if (hFind == INVALID_HANDLE_VALUE) {
        LOGE("scan dir initialize failed");
        return FALSE;
    }

    do {
        // display only files, skip directories
        if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            continue;
        LOGD("Find file %S", FindData.cFileName);
        if (fileName != FindData.cFileName )
            continue;
        filePath = szDirectory;
        filePath += _T("\\");
        filePath += FindData.cFileName;
        found = TRUE;
        break;
    }while (FindNextFile (hFind, & FindData));

    FindClose (hFind);

    return found;
}

char const* CMiniHttpDownloadServer::BuildHttpServerResponse(LPCWSTR fn, unsigned *_sz) {
    HANDLE    file;
    char *data = NULL;
    char *content = NULL;
    char temp[64]={0};
    char date[64]={0};
    DWORD file_size;
    DWORD out_bytes;
    *_sz = 0;

    file = CreateFile(fn,
                      GENERIC_READ,
                      FILE_SHARE_READ,
                      NULL,
                      OPEN_EXISTING,
                      0,
                      NULL);

    if (file == INVALID_HANDLE_VALUE) {
        LOGE("load_file: file open failed (rc=%ld)\n", GetLastError());
        return NULL;
    }

    file_size = GetFileSize( file, NULL );

    if (file_size <= 0) {
        LOGE("file empty or negative size %ld\n", file_size);
        CloseHandle(file);
        return NULL;
    }

    sprintf(temp, "%d", file_size);
    gmt_time_string(date, sizeof(date));
    string header= "HTTP/1.1 200 OK\r\n";
    header.append("Content-Length: ").append(temp).append("\r\n");
    header.append("Date: ").append(date).append("\r\n");
    header.append("Content-Type: application/x-sam\r\n");
    header.append("Accept-Ranges: bytes\r\n" );
    header.append("\r\n");
    size_t length=header.size() + (size_t)file_size + 1;

    content= (char*) malloc(sizeof(char)*length);
    if(!content) {
        LOGE("Failed to allocate memory!");
        CloseHandle(file);
        return content;
    }
    memset(content, 0, length);
    memcpy(content, header.c_str(), header.size());
    content[length - 1]='\0';

    data = content + header.size(); //(char*) malloc( file_size + 1 );

    if (!ReadFile(file, data, file_size, &out_bytes, NULL) || out_bytes != file_size) {
        int retry_failed = 0;

        if (GetLastError() == ERROR_NO_SYSTEM_RESOURCES) {
            /* Attempt to read file in 10MB chunks */
            DWORD bytes_to_read = file_size;
            DWORD bytes_read    = 0;
            DWORD block_size    = 10*1024*1024;

            SetFilePointer( file, 0, NULL, FILE_BEGIN );

            while (bytes_to_read > 0) {
                if (block_size > bytes_to_read) {
                    block_size = bytes_to_read;
                }

                if (!ReadFile(file, data+bytes_read, block_size, &out_bytes, NULL) ||
                        out_bytes != block_size) {
                    retry_failed = 1;
                    break;
                }
                bytes_read    += block_size;
                bytes_to_read -= block_size;
            }
        } else {
            retry_failed = 1;
        }

        if (retry_failed) {
            LOGE("could not read %ld bytes from '%s'\n", file_size, fn);
            free(content);
            content = NULL;
            file_size = 0;
        }
    }

    CloseHandle(file);
    *_sz = (unsigned) length;

    return content;
}


void CMiniHttpDownloadServer::StartHttpServer(LPCWSTR path) {
    SOCKET sockConn = INVALID_SOCKET;
    SOCKET sockSrv  = INVALID_SOCKET;
    SOCKADDR_IN  addrClient;
    SOCKADDR_IN addrSrv;
    CString msg;
    size_t length = 0;
    int code;

    sockSrv = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockSrv == INVALID_SOCKET) {
        HandleServerException(_T("Create server socket failed!"), sockConn, sockSrv, NULL);
        return ;
    }

    addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(m_ServerPort);

    if (bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR)) == SOCKET_ERROR) {
        msg.Format(_T("Bind socket port %d failed!"), m_ServerPort);
        HandleServerException(msg, sockConn, sockSrv, NULL);
        return ;
    }

    msg.Format(_T("bind socket success on port :%d"), m_ServerPort);
    UpdateMessage(msg);
    if (listen(sockSrv, SOMAXCONN) == SOCKET_ERROR) {
        HandleServerException(_T("server listen failed!"), sockConn, sockSrv, NULL);
        return ;
    }

    UpdateMessage(_T("Listening on socket..."));
    m_ServerWork = TRUE;
    while(m_ServerWork) {
        unsigned long on = 1;
        char const * content = NULL;
        char recvBuf[101]={0};
        int bytes = 0;
        int sin_size = sizeof(struct sockaddr_in);

        sockConn = accept(sockSrv, (SOCKADDR*)&addrClient, &sin_size);
        UpdateMessage(_T("Send softwre ..., please wait..."));
        ioctlsocket(sockConn, FIONBIO, &on);

        do {
            memset(recvBuf, 0, sizeof recvBuf);
            bytes = recv(sockConn, recvBuf, sizeof(recvBuf) - 1, 0);
        } while(bytes > 0);

        LOGE("RECV:\n %s", recvBuf);

         content = BuildHttpServerResponse(path, &length);

        if(content == NULL) {
            UpdateMessage(_T("Open file failed!"));
            break;
        }

        if (send(sockConn, content , length , 0) == SOCKET_ERROR) {
            HandleServerException(_T("Send software failed!"), sockConn, sockSrv, &content);
            break;
        }

        do {
            memset(recvBuf, 0, sizeof recvBuf);
            bytes = recv(sockConn, recvBuf, sizeof(recvBuf) - 1, 0);
        }while(bytes > 0);
        HandleServerException(_T("Send softwre successfully!"), INVALID_SOCKET, INVALID_SOCKET, &content);
    }
    HandleServerException(_T("Server exist"), sockConn, sockSrv, NULL);
}

#if 0
void CMiniHttpDownloadServer::server_listen(const char*path)
{
    SOCKET sockConn = INVALID_SOCKET;
    SOCKET sockSrv  = INVALID_SOCKET;
    CString msg;
    size_t length = 0;
    __int64 iResult;

//    if (mWSAInitialized == FALSE) {
//        UpdateMessage(_T("Server Startup failed!"));
//        return ;
//    }

  // server_state=false;
    char const * content = BuildHttpServerResponse(path, &length);

    if(content == NULL) {
        UpdateMessage(_T("Open file failed!"));
        return ;
    }

    sockSrv = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockSrv == INVALID_SOCKET) {
        HandleServerException(_T("Create server socket failed!"), sockConn, sockSrv, &content);
        return ;
    }

    SOCKADDR_IN addrSrv;
    addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(m_ServerPort);
    iResult = bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
    if (iResult == SOCKET_ERROR) {
        msg.Format(_T("Bind socket port %d failed!"), m_ServerPort);
        HandleServerException(msg, sockConn, sockSrv, &content);
        return ;
    }
    msg.Format(_T("bind socket success on port :%d"), m_ServerPort);
    UpdateMessage(msg);
    if (listen(sockSrv, SOMAXCONN) == SOCKET_ERROR) {
        HandleServerException(_T("server listen failed!"), sockConn, sockSrv, &content);
        return ;
    }

    UpdateMessage(_T("Listening on socket..."));
    SOCKADDR_IN  addrClient;
    while(m_ServerWork) {
        unsigned long on = 1;
        char recvBuf[101]={0};

        int sin_size = sizeof(struct sockaddr_in);
        sockConn = accept(sockSrv, (SOCKADDR*)&addrClient, &sin_size);
        UpdateMessage(_T("Send softwre ..., please wait..."));
        ioctlsocket(sockConn, FIONBIO, &on);

        int bytes = 0;
        do {
            memset(recvBuf, 0, 101);
            bytes = recv(sockConn, recvBuf, 100, 0);
        } while(bytes > 0);

        iResult = send(sockConn, content , length , 0);

        if (iResult == SOCKET_ERROR) {
            HandleServerException(_T("Send software failed!"), sockConn, sockSrv, &content);
            break;
        }

        do {
            memset(recvBuf, 0, 101);
            bytes = recv(sockConn, recvBuf, 100, 0);
        }while(bytes > 0);
        UpdateMessage(_T("Send softwre successfully!"));

        //closesocket(sockConn);
        //sockConn = INVALID_SOCKET;
    }
    HandleServerException(_T("Server exist"), sockConn, sockSrv, &content);
}


char const* CMiniHttpDownloadServer::BuildHttpServerResponse(const char *path, size_t  *contentLength) {
    char* content = NULL;
    __int64  nLen = 0;

    if (path == NULL || contentLength == NULL) {
        UpdateMessage(_T("Bad parameter of BuildHttpServerResponse"));
        return content;
    }
    *contentLength = 0;

    FILE *pF = fopen(path, "rb" );
    if(pF==NULL) {
        //perror(file_name);
        UpdateMessage(_T("Open file failed!"));
        return content;
    }
    _fseeki64(pF, 0, SEEK_END);
    nLen = _ftelli64(pF);

    char temp[64]={0};
    char date[64]={0};
    sprintf(temp, "%I64d", nLen);
    gmt_time_string(date, sizeof(date));
    string header= "HTTP/1.1 200 OK\r\n";
    header.append("Content-Length: ").append(temp).append("\r\n");
    header.append("Date: ").append(date).append("\r\n");
    header.append("Content-Type: application/x-sam\r\n");
    header.append("Accept-Ranges: bytes\r\n" );
    header.append("\r\n");
    size_t length=header.size() + (size_t)nLen + 1;
    rewind(pF);
    content= (char*) malloc(sizeof(char)*length);
    if(!content) {
        UpdateMessage(_T("Failed to allocate memory!"));
        return content;
    }
    memset(content, 0, length);
    memcpy(content, header.c_str(), header.size());
    int len = header.size();

    fread(content + len, 1, (size_t)nLen, pF);
    content[length - 1]='\0';
    fclose(pF);
    //printf("Header length is %d\n", header.size());
    //printf("Header is \n%s\n", header.c_str());
    *contentLength = length;
    return content;
}

BOOL CMiniHttpDownloadServer::BuildUpdateCommand(CString file, CString &cmd) {
    string Firmware_name=file.GetString();
    int tmp=Firmware_name.find_last_of("\\");
    Firmware_name=Firmware_name.substr(tmp+1,Firmware_name.length());
    const char * filename = basename(file.GetString());

    if(file =="" ) {
        ::MessageBox(NULL,_T("Please select software!"),_T("select software"),MB_OK);
        return FALSE;
    }

    if(mHostIPAddr.empty()) {
        UpdateMessage(_T("Get IP failed!"));
        return FALSE;
    }

    string download_comand="update update -u \"http://"+mHostIPAddr+"/"+Firmware_name+
                            "/\" -f --no-cert-check --no-device-check\r\n\r\n";

    cmd.Format("update update -u \"http://%s/%s/\" -f --no-cert-check --no-device-check\r\n\r\n",
                                mHostIPAddr.c_str(), Firmware_name);

    return TRUE;
}

void CDownloadDlg::OnSend_Comand(SOCKET sockClient, const char * cmd) {
    char s_SN[16]      = {0};
    char s_SSID[51]    = {0};
    char MAC_label[20] = {0};
    char recvBuf[101] = {'\0'};
    string Send_result;
    int i=0;
    int bytes = 0;

    if (sockClient == INVALID_SOCKET) {
        return;
    }

    //recv(sockClient, recvBuf, 100, 0);
    //bool is_set_time_out=SetTimeOut(sockClient, 50000, true);
    bytes = send(sockClient, cmd, strlen(cmd)+1, 1);
    LOGE("Send %d bytes", bytes);

    do {
        memset(recvBuf, '\0', 101);
        bytes = recv(sockClient, recvBuf, 100, 0);
        LOGE("Receive %d bytes", bytes);
        if(strstr(recvBuf,"root@Qualcomm")!=NULL) {
            if(strstr(recvBuf,"Writing data")!=NULL) {
                UpdateMessage(recvBuf);
            }
        }

        i++;
        Send_result+=recvBuf;
        LOGE("%s", recvBuf);
        m_progMac2.SetPos(i);
        //m_progMac2.Invalidate(FALSE);
        if((Send_result.find("[processCommand] Processing send"))!=-1) {
            UpdateMessage(_T("send software successfully!"));
            /*m_progMac2.SetPos(Progress_range);
              m_progMac2.Invalidate(FALSE);
              m_progMac2.SetBarColor(RGB(0,255,0));
              DWORD dwEndTime = ::GetTickCount();
              DWORD dwSpaceTime = (dwEndTime - dwBeginTime)/1000;
              sprintf(s_SSID,"%s_%s_AJ",s_SSID_Prefix,s_WIFI+6);
              strupr(s_WIFI);
              GaliSNfromWIFI(s_SN,s_WIFI);
              sprintf(MAC_label, "%c%c%s%c%c%s%c%c%s%c%c%s%c%c%s%c%c",
              s_WIFI[0], s_WIFI[1], ":", s_WIFI[2], s_WIFI[3], ":",
              s_WIFI[4], s_WIFI[5], ":", s_WIFI[6], s_WIFI[7], ":",
              s_WIFI[8], s_WIFI[9], ":", s_WIFI[10], s_WIFI[11]);
            //DWORD dwNum = WideCharToMultiByte(CP_OEMCP,NULL,Original_Wifi_name,-1,NULL,0,NULL,FALSE);
            //WideCharToMultiByte(CP_OEMCP, NULL, (LPCWSTR)Original_Wifi_name, -1,(LPSTR)s_SSID, dwNum, NULL, FALSE);
            GenSAV(s_SN,s_CommercialRef,s_PCBNo,s_PTS_new,"","","",0,"HDT",dwSpaceTime,0,MAC_label,s_SSID,"","","");
            GetDlgItem(ID_Start)->EnableWindow(true);
            is_downloading=false;
            ::MessageBoxA(NULL,"Download finished, please remove the device for next!","MBO1",MB_SYSTEMMODAL);*/
            m_progMac2.SetWindowText(_T("Send software finished!, Please wait for the device update and then restart "));
            m_progMac2.Invalidate(FALSE);
            Sleep(30000);
            //TODO:: comment by xusl
            //			get_Wifi_connect();
            //is_downloading=false;
            break;
        }

        if((Send_result.find("[ERROR]"))!=-1) {
            UpdateMessage(_T("send software failed!"));
            m_progMac2.SetBarColor(RGB(255,50,50));
            m_progMac2.Invalidate(FALSE);
            GetDlgItem(ID_Start)->EnableWindow(true);
            is_downloading=false;
            ::MessageBox(NULL,_T("Download failed!"),_T("Download"),MB_OK);
            break;
        }
    } while(bytes > 0);
}

DWORD WINAPI CDownloadDlg::Thread_Server_Listen(LPVOID lpPARAM) {
	CDownloadDlg *pThis = (CDownloadDlg *)lpPARAM;
	pThis->server_listen();
	return 1;
}
#endif

