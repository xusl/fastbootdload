#include "StdAfx.h"
#include "XmlParser.h"
#import "msxml6.dll" // raw_interfaces_only
#include "log.h"
#include "utils.h"

XmlParser::XmlParser()//:
    //m_pXmlBuf(NULL)
{
    ::CoInitialize(NULL);
    mHR = spDoc.CoCreateInstance(__uuidof(DOMDocument));    //创建文档对象
}

XmlParser::~XmlParser() {
    if (SUCCEEDED(mHR))
        spDoc.Release();
    ::CoUninitialize();
    //RELEASE_ARRAY(m_pXmlBuf);
}

void XmlParser::Parse(PCCH pXmlBuf, unsigned int size) {
#if 1
    IStream *I=NULL;
    CComPtr<IXMLDOMParseError> spParseError;
    CComBSTR bstrReason;
    LARGE_INTEGER liTemp = {0};
    HRESULT hr;
    VARIANT_BOOL bFlag;
    int bomSize = SkipBOM(pXmlBuf);
    int len = strlen(pXmlBuf);

    XML_Value.clear();

    //创建一个保存在全局内存的流对象
    CreateStreamOnHGlobal(NULL, true, &I);

    //从流对象的开始处写入相应的内容
    I->Seek(liTemp, STREAM_SEEK_SET, NULL);
    I->Write(pXmlBuf + bomSize, size - bomSize, NULL);

#if 0
    DWORD dwSize=0;
    STATSTG stats={0};
    char text[4096] = {0};
    ULARGE_INTEGER uiPos = {0};
    //获取流对象里内容的大小
    I->Seek(liTemp, STREAM_SEEK_CUR, &uiPos);
    dwSize = (DWORD)uiPos.QuadPart;

    //从流对象开始处读取其所有内容
    I->Seek(liTemp, STREAM_SEEK_SET, NULL);
    I->Read(text, sizeof(text), NULL);

    //获取流对象里内容的状态（提取其中的流对象里的内容的大小信息）
    I->Stat(&stats, 0);
    dwSize = (DWORD)stats.cbSize.QuadPart;
    printf("已读取到%d字节的内容/n", dwSize);
#endif

    I->Seek(liTemp, STREAM_SEEK_SET, NULL);
    hr = spDoc->load(CComVariant(I), &bFlag);

    spDoc->get_parseError(&spParseError);
    spParseError->get_reason(&bstrReason);
    if (bstrReason != NULL)
        wprintf((wchar_t*)bstrReason);

    DoParse(spDoc);
    I->Release();

#else

    if(CharToBSTR(data, &m_pXmlBuf) != 1)
        return;
    //m_pXmlBuf = (BSTR)pXmlBuf;

    if (m_pXmlBuf != NULL) {
        //CHKHR(spDoc->put_validateOnParse(VARIANT_FALSE));
        //CHKHR(spDoc->put_resolveExternals(VARIANT_FALSE));
        spDoc->put_validateOnParse(VARIANT_FALSE);
        spDoc->put_resolveExternals(VARIANT_FALSE);
        hr = spDoc->loadXML(m_pXmlBuf, &bFlag);

        spDoc->get_parseError(&spParseError);
        spParseError->get_reason(&bstrReason);
        if (bstrReason != NULL)
            wprintf((wchar_t*)bstrReason);

        DoParse(spDoc);
        SysFreeString(m_pXmlBuf);
    }
#endif
}
void XmlParser::Parse(const wchar_t * xmlPath) {
    HRESULT hr;
    VARIANT_BOOL bFlag;
    XML_Value.clear();
    hr = spDoc->load(CComVariant(xmlPath), &bFlag);       //load xml文件
    DoParse(spDoc);
}

int XmlParser::SkipBOM(PCCH source) {
    int skip = 0;
//escape BOM
if (source[0] == (char)0XEF && source[1] == (char)0XBB && source[2] == (char)0XBF)
    skip = 3; //UTF-8
else if (source[0] == (char)0XFE && source[1] == (char)0XFF || source[1] == (char)0XFE && source[0] == (char)0XFF)
    skip = 2;//UTF-16
else if (source[0] == (char)0X00 && source[1] == (char)0X00 && source[2] == (char)0XFE && source[3] == (char)0XFF ||
         source[3] == (char)0X00 && source[2] == (char)0X00 && source[1] == (char)0XFE && source[0] == (char)0XFF)
    skip = 4;//UTF-16

    return skip;
}

void XmlParser::DoParse(CComPtr<IXMLDOMDocument> &spDoc) {
    CComPtr<IXMLDOMNodeList> spNodeList;
    CComPtr<IXMLDOMElement>    spElement;
    CComBSTR strTagName;
    CComBSTR name;
    long lCount;
    HRESULT hr;

    hr = spDoc->get_documentElement(&spElement);   //获取根结点
    if (spElement == NULL) {
        LOGE("None element exist!");
        return;
    }
    hr = spElement->get_tagName(&strTagName);

    cout << "------TagName------" << CString(strTagName) << endl;

    hr = spElement->get_childNodes(&spNodeList);   //获取子结点列表
    hr = spNodeList->get_length(&lCount);

    for (long i=0; i<lCount; ++i) {
        //CComVariant varNodeValue;
        CComPtr<IXMLDOMNode> spNode;
        hr = spNodeList->get_item(i, &spNode);         //获取结点
        hr = spNode->get_nodeName(&name);
        ParseElement(spNode, CString(name));
        spNode.Release();
    }

    spNodeList.Release();
    spElement.Release();
}

void XmlParser::ParseElement(CComPtr<IXMLDOMNode> &spNode, CString& NodeName) {
    CComPtr<IXMLDOMNodeList> spChildNodeList;
    long childLen;
    HRESULT hr;
    VARIANT_BOOL bHasChildren;
    DOMNodeType NodeType;

    hr = spNode->get_nodeType(&NodeType);     //获取结点信息的类型

    if (NODE_ELEMENT != NodeType) {
        return;
    }

    hr = spNode->hasChildNodes(&bHasChildren);

    if (!bHasChildren) {
        CComBSTR value;
        CComBSTR name;
        hr = spNode->get_nodeName(&name);            //获取结点名字
        if (wcscmp(L"#text", name)) {
         return;
        }
        hr = spNode->get_text(&value);                //获取结点的值
        ElementHandler(NodeName, value);
        return;
    }

    hr = spNode->get_childNodes(&spChildNodeList);
    hr = spChildNodeList->get_length(&childLen);
    for (int j=0; j<childLen; ++j) {
        CComPtr<IXMLDOMNode> spChildNode;
        CComBSTR value;
        CComBSTR name;

        hr = spChildNodeList->get_item(j, &spChildNode);
        hr = spChildNode->hasChildNodes(&bHasChildren);
        hr = spChildNode->get_nodeName(&name);            //获取结点名字

        if (bHasChildren) {
            ParseElement(spChildNode, CString(name));
        } else if (wcscmp(L"#text", name) == 0){
            hr = spChildNode->get_text(&value);                //获取结点的值
            ElementHandler(NodeName, value);
        }
        spChildNode.Release();
    }
//EXIT:
    spChildNodeList.Release();
}

bool XmlParser::ElementHandler(CString& name, CComBSTR &value) {
    bool result = false;
    CString value_text =CString(value);

    PCHAR pname = WideStrToMultiStr(name.GetString());
    PCHAR pvalue = WideStrToMultiStr(value_text.GetString());

    LOGI("Name: %s,  Value:%s", pname, pvalue);

    //PCHAR pname = WideStrToMultiStr(name.GetBuffer(0));
    //PCHAR pvalue = WideStrToMultiStr(value_text.GetBuffer(0));

    if(pname != NULL && pvalue != NULL) {
        string item_name(pname);
        string item_value(pvalue);
        XML_Value.insert(std::pair<string, string>(item_name, item_value));
        result = true;
    }

    //name.ReleaseBuffer();
    //value_text.ReleaseBuffer();
    RELEASE_ARRAY(pname);
    RELEASE_ARRAY(pvalue);
    return result;
}

bool XmlParser::getElementsByTagName(BSTR bstrTagName, string &value) {
    CComPtr<IXMLDOMElement>   spElement;
    CComPtr<IXMLDOMNodeList> spNodeList;
    VARIANT_BOOL bHasChildren;
    HRESULT hr;
    long lCount;
    CComBSTR text;

    hr = spDoc->get_documentElement(&spElement);   //获取根结点
    if (spElement == NULL) {
        LOGE("None element exist!");
        return false;
    }
    spElement->getElementsByTagName(bstrTagName, &spNodeList);
    hr = spNodeList->get_length(&lCount);
    for (long i=0; i<lCount; ++i) {
        CComPtr<IXMLDOMNode> spNode;
        hr = spNodeList->get_item(i, &spNode);         //获取结点
        spNode->hasChildNodes(&bHasChildren);
        hr = spNode->get_text(&text);

        PCHAR pvalue = WideStrToMultiStr(CString(text).GetString());
        value = pvalue;
        RELEASE_ARRAY(pvalue);
        //ParseElement(spNode);
        spNode.Release();
    }

    spNodeList.Release();
    spElement.Release();
    return true;
}


string XmlParser::get_XML_Value(string key) {
    map<string, string>::iterator it = XML_Value.find(key);
    if(it != XML_Value.end())
        return XML_Value[key];
    return "";

    //if(XML_Value.contains(key)) {
    //    return XML_Value[key];
    //}
    //return "";
}

vector<string> XmlParser::getCompareList() {
    return m_mandatoryCompArray;
}

PackageConfig::PackageConfig():
    XmlParser() {
    ;
}

PackageConfig::~PackageConfig() {
    ;
}

//bool ConfigXml::ElementHandler(CString& name, CComBSTR& value) {
//
//       return true;
//}
#if 0
void flash_image::read_package_version(const wchar_t * package_conf){
  CComPtr<MSXML2::IXMLDOMDocument> spDoc;
  CComPtr<MSXML2::IXMLDOMNodeList> spNodeList;
  CComPtr<MSXML2::IXMLDOMElement> spElement;
  CComBSTR strTagName;
  VARIANT_BOOL bFlag;
  long lCount;
  HRESULT hr;

  ::CoInitialize(NULL);
  hr = spDoc.CoCreateInstance(__uuidof(MSXML2::DOMDocument));    //创建文档对象
  hr = spDoc->load(CComVariant(package_conf), &bFlag);       //load xml文件
  hr = spDoc->get_documentElement(&spElement);   //获取根结点
  if (spElement == NULL) {
    ERROR("No %S exist", package_conf);
    return;
    }
  hr = spElement->get_tagName(&strTagName);

  //cout << "------TagName------" << CString(strTagName) << endl;

  hr = spElement->get_childNodes(&spNodeList);   //获取子结点列表
  hr = spNodeList->get_length(&lCount);

  for (long i=0; i<lCount; ++i) {
    CComVariant varNodeValue;
    CComPtr<MSXML2::IXMLDOMNode> spNode;
    MSXML2::DOMNodeType NodeType;
    CComPtr<MSXML2::IXMLDOMNodeList> spChildNodeList;

    hr = spNodeList->get_item(i, &spNode);         //获取结点
    hr = spNode->get_nodeType(&NodeType);     //获取结点信息的类型

    if (NODE_ELEMENT == NodeType) {
      long childLen;
      hr = spNode->get_childNodes(&spChildNodeList);
      hr = spChildNodeList->get_length(&childLen);

      //cout << "------NodeList------" << endl;

      for (int j=0; j<childLen; ++j) {
        CComPtr<MSXML2::IXMLDOMNode> spChildNode;
        CComBSTR value;
        CComBSTR name;

        hr = spChildNodeList->get_item(j, &spChildNode);
        hr = spChildNode->get_nodeName(&name);            //获取结点名字
        hr = spChildNode->get_text(&value);                //获取结点的值
        //cout << CString(name) << endl;
        //cout << CString(value) << endl << endl;

        //parse_pkg_sw(CString(name), CString(value));

        spChildNode.Release();
      }
    }

    spNode.Release();
    spChildNodeList.Release();
  }

  spNodeList.Release();
  spElement.Release();
  spDoc.Release();
  ::CoUninitialize();
}

 int flash_image::parse_pkg_sw(CString & node, CString & text) {
    if (node == L"Linux_Kernel_Ver")
        a5sw_kern_ver = text;

    else if (node == L"Linux_SYS_Ver")
        a5sw_sys_ver = text;

    else if (node == L"Linux_UserData_Ver")
        a5sw_usr_ver = text;

    else if (node == L"Q6_Resource_Ver")
        fw_ver = text;

    else if (node == L"QCN")
        qcn_ver = text;

    return 0;
}
#endif

