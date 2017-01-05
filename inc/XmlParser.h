#pragma once
#include "stdafx.h"
#include "define.h"
#include <vector>
#include <map>
#include <string>
#include <msxml.h>
#include <atlstr.h>

using namespace std;


class XmlParser
{
public:
    XmlParser();
    ~XmlParser();

    string       get_XML_Value(string key);
    vector<string> getCompareList();
    bool         getElementsByTagName(BSTR bstrTagName, string &value);
    void         Parse(PCCH pXmlBuf, unsigned int size);
    void         Parse(const wchar_t * xmlPath);

  private:
    void         DoParse(CComPtr<IXMLDOMDocument> &spDoc);
    void         ParseElement(CComPtr<IXMLDOMNode> &spNode, CString& NodeName);
    virtual bool ElementHandler(CString& name, CComBSTR &value);
    int       SkipBOM(PCCH source);

private:
    CComPtr<IXMLDOMDocument> spDoc;
    HRESULT                    mHR;
    //QDomElement               getElementById(QDomElement& parent);
    map<string, string>     XML_Value;
    // map<wstring, wstring>     XML_Value;
    vector<string>           m_mandatoryCompArray;
};

//test

//unsigned int size;
//void *data = load_file(mAppConf.GetPkgConfXmlPath(), &size);
//XmlParser parser1;
//parser1.Parse((PCCH)data, size);
//parser1.Parse("<?wsx version \"1.0\" ?><smil> \
//         <media src = \"welcome1.asf\"/>cdcddddddddd</smil>");
//LOGE(" xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");

//string refs;
//m_LocalConfigXml.getElementsByTagName(L"RECOVERYFS", refs);
//LOGE("RECOVERYFS value %sxxxxxxxxxxxxxxxxxxxxx", refs.c_str());
class PackageConfig:  public XmlParser {
  public:
    PackageConfig();
    ~PackageConfig();
    string GetProjectCode() { return get_XML_Value("Project_Code");}
    string GetCURef() { return get_XML_Value("CURef"); }
    string GetVersion() { return get_XML_Value("External_Ver");}
    string GetCustomerCode() { return get_XML_Value("Customer_Code");}
    string GetFlashType() { return get_XML_Value("Flash_Code");}

 // private:
 //   virtual bool              ElementHandler(CString& name, CComBSTR &value);
};
