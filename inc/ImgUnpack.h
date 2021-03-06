#pragma once
#include "stdafx.h"
#include "define.h"
#include <map>
#include <string>
#include <XmlParser.h>

using namespace std;

class ImgUnpack{
public:
    ImgUnpack();
    ~ImgUnpack();

    BOOL UnpackDlImg(const wchar_t *lpath, const wchar_t* config_file);
    map<string,FileBufStruct> GetFileBuffer() { return m_downloadFileBuffer;};
    const XmlParser *GetConfXmlParser() { return m_LocalConfig;};

private:
  bool ReadVersionInfo();
  bool ParseQCN();

private:
    FilePosInfoNewS*                m_filesName;
    uint8*                          pImgData;
    uint8*                          packageDataBuf;
    uint8*                          staticQcnBuf;
    uint8*                          dynamicNvData;
    uint8*                          qcnDataBuffer;
    uint8                           imgVersion;
    uint32                          qcnLens;
    bool                            bParsePacketOk;
    map<string,FileBufStruct>       m_downloadFileBuffer;
    map<string, string>             versionMap;
    vector<string>                  m_efsFileName;
    vector<string>                  nvItemsIgnore;
    XmlParser                      *m_LocalConfig;
};
