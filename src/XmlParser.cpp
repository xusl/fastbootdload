#include "StdAfx.h"
#include "XmlParser.h"
#import "msxml6.dll" raw_interfaces_only
#include "log.h"
#include "utils.h"

XmlParser::XmlParser():
    m_pXmlBuf(NULL)
{
    ::CoInitialize(NULL);
    mHR = spDoc.CoCreateInstance(__uuidof(DOMDocument));    //创建文档对象
    //CHKHR(spDoc->put_validateOnParse(VARIANT_FALSE));
    //CHKHR(spDoc->put_resolveExternals(VARIANT_FALSE));
}

XmlParser::~XmlParser() {
    if (SUCCEEDED(mHR))
        spDoc.Release();
    ::CoUninitialize();
    RELEASE_ARRAY(m_pXmlBuf);
}
/*
HRESULT loadXML(
BSTR bstrXML,
VARIANT_BOOL* varIsSuccessful
);
*/
void XmlParser::Parse(PCCH pXmlBuf) {
    HRESULT hr;
    VARIANT_BOOL bFlag;
    RELEASE_ARRAY(m_pXmlBuf);
    XML_Value.clear();
    m_pXmlBuf = MultiStrToWideStr(pXmlBuf);

    //hr = spDoc->loadXML((BSTR)pXmlBuf, &bFlag);
    hr = spDoc->loadXML(m_pXmlBuf, &bFlag);
    DoParse(spDoc);
}
void XmlParser::Parse(const wchar_t * xmlPath) {
    HRESULT hr;
    VARIANT_BOOL bFlag;
    XML_Value.clear();
    hr = spDoc->load(CComVariant(xmlPath), &bFlag);       //load xml文件
    DoParse(spDoc);
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
EXIT:
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

ConfigXml::ConfigXml() {
    ;
}

ConfigXml::~ConfigXml() {
    ;
}

bool ConfigXml::ElementHandler(CString& name, CComBSTR& value) {

        return true;
}
