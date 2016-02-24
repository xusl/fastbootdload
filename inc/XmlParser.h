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

class ConfigXml:  public XmlParser {
  public:
    ConfigXml();
    ~ConfigXml();

  private:
    virtual bool              ElementHandler(CString& name, CComBSTR &value);
};