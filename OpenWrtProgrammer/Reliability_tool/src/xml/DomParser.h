/************************************************************************/
/*  Parser XML 
DESC:

CHANGE HISTORY:
when        who             what
----------  ---------       --------------------------------------------------------
2010-10-20  zhongping.huang   Add parser mac Info.plist
2010-09-02  jianwen.he		  Add function "ParseFromBuf()"
2010-08-31  zhongping.huang   Init first version
*/
/************************************************************************/



#ifndef DOMPARSER_H
#define DOMPARSER_H

#include "../define/define.h"
#include "../../version.h"        //add by jie.li 2011-06-10

typedef struct _XmlDataStruct{
        typeQString key;
        typeQString value;
}XmlDataStruct;

typedef typeQVector<XmlDataStruct> XMLDataVector;

class DomParser
{
public:
    DomParser(typeQTreeWidget *tree);
    bool readFile(const typeQString fileName);
    bool ParseFromBuf(typeQByteArray& xmlBuffer);
    bool ParseSsidFromBuf(typeQByteArray& xmlBuffer, WebsXMLInfo* pxmlinfo);
	
	/************************************************************************/
	/*   config.xml                                                                   */
	/************************************************************************/
    typeQString m_strPackage;       //Package
    typeQString m_strPCB;
    typeQString strvendor;
    typeQString strnand;
    typeQString strsdram;
    typeQString m_strFlashValue;
    typeQString m_RFswitch;
    typeQString m_strPartMandatory;
    typeQString m_strPartition;
    typeQString m_strFirInterVer;
    typeQString m_strFirInterMandatory;
    typeQString m_strFirExterVer;
    typeQString m_strFirExterMandatory;
    typeQString m_strQCNMandatory;
    typeQString m_strQCN;
    //add by yanbin.wan 2013-01-28
    typeQString m_strQ6ResourceMandatory;
    typeQString m_strQ6Resource;
    typeQString m_strLinuxKernelMandatory;
    typeQString m_strLinuxKernel;
    typeQString m_strLinuxAppMandatory;
    typeQString m_strLinuxApp;
    typeQString m_strLiunxSysMandatory;
    typeQString m_strLinuxSys;
    //end add
    //add by jie.li 2011-09-28
    typeQString m_strLTEMandatory;
    typeQString m_strLTE;
    //end add
    //add by jie.li 2012-04-06 for Diversity NV
    typeQString m_strDiversity;
    //end add
    typeQString m_strDashboardMandatory;
    typeQString m_strWinVer;
    typeQString m_strMacVer;
    typeQString m_strLinuxVer;
    typeQString m_strWEBUIVer;          //add by jie.li 2011-11-29 for Y580
    typeQString m_strUSBMandatory;
    typeQString m_strUSBWinVer;
    typeQString m_strUSBMacVer;
    typeQString m_strUSBLinuxVer;
    typeQString m_strCodePU;
    typeQString m_strCodePP;
    typeQString m_strCodePF;
    typeQString m_strCodePC;
    typeQString m_strCodePN;
    typeQString m_strActPU;
    typeQString m_strActPP;
    typeQString m_strActPF;
    typeQString m_strActPC;
    typeQString m_strActPN;
    typeQString m_strHSU_Comp_id;
    typeQString m_strVoice;
    typeQString	m_strIMEI;
    typeQString	m_strSpecial_AT;
    typeQString	m_strDisk_attrI;
    typeQString	m_strDevmopde;
    typeQString	m_strLed;
    typeQString	m_strEFS;
    typeQString	m_strDigPort;
    typeQString m_strBand;
    typeQString	m_strMode;
    typeQString	m_strOrder;
    typeQString	m_strDomain;
    typeQString m_strCustMandatory;
    typeQString m_strCustomer;

    typeQString strIgUMTS2100;
    typeQString	strIgUMTS1700;
    typeQString	strIgUMTS1900;
    typeQString	strIgUMTS900;
    typeQString	strIgUMTS850;
    typeQString	strIgUMTS800;      //add by jie.li 2011-11-29 for Y580
    typeQString	strIgUMTS1800;     //add by jie.li 2011-06-10
    typeQString	strIgEGSM900;
    typeQString	strIgDCS1800;
    typeQString	strIgGSM850;
    typeQString	strIgPCS1900;

    //add by jie.li 2011-11-29 for LTE
    typeQString	strIgLTE2100;
    typeQString	strIgLTE1800;
    typeQString	strIgLTE2600;
    typeQString	strIgLTE900;
    typeQString	strIgLTE800;
    //end add

    //add by jie.li 2012-10-16
    typeQString	strIgLTE700;
    typeQString	strIgLTE1700;
    //end add

    //add by yanbin.wan 2013-01-25
    typeQString strIgLTE850;
    typeQString strIgLTE1900;
    //end add

    typeQStringList CompareList;
    typeQString m_strFrmMandatory;
    XmlDataStruct xmlData;
    XMLDataVector m_XMLMap;

    typeQString m_CfgXMLVer;
	/************************************************************************/
	/*  dynamic_nv.xml                                                                    */
	/************************************************************************/
    typeQString m_strBandVersion;
    typeQVector<typeQString>    m_UMTS2100Band;
    typeQVector<typeQString>	m_UMTS1700Band;
    typeQVector<typeQString>	m_UMTS1900Band;
    typeQVector<typeQString>	m_UMTS900Band;
    typeQVector<typeQString>	m_UMTS850Band;
    typeQVector<typeQString>	m_UMTS1800Band;    //add by jie.li 2011-06-10
    typeQVector<typeQString>	m_EGSM900Band;
    typeQVector<typeQString>	m_DCS1800Band;
    typeQVector<typeQString>	m_GSM850Band;
    typeQVector<typeQString>	m_PCS1900Band;
    typeQVector<typeQString>	m_DefaultBand;
    //add by jie.li 2011-11-29 for LTE
    typeQVector<typeQString>	m_LTE2100Band;
    typeQVector<typeQString>	m_LTE1800Band;
    typeQVector<typeQString>	m_LTE2600Band;
    typeQVector<typeQString>	m_LTE900Band;
    typeQVector<typeQString>	m_LTE800Band;
    //end add

    //add by yanbin.wan 2013-01-25
    typeQVector<typeQString>	m_LTE850Band;
    typeQVector<typeQString>	m_LTE1900Band;
    //end add
    typeQVector<typeQString>    m_DiversityBand;
	
    /************************************************************************/
    /*  info.plist                                                                    */
    /************************************************************************/
    typeQString             pcVersion;
    typeQString             pcInstallPath;

private:
        typeQTreeWidget* treeWidget;

		/************************************************************************/
		/*   config.xml                                                                   */
		/************************************************************************/
        void ParserContentElement(const typeQDomElement &element);
        void ParserPackageElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserCustomerIDElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);

        void ParserHardwareElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserPCBElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserBandElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserFlashElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserRF_switcherElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        //add by jie.li 2012-04-06 for Diversity NV
        void ParserDivCalibrationElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        //end add

        void ParserSoftwareElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserPartitionElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserFirInterVerElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserFirExterVerElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserQCNElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserLTEBANDCFGElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);

        void ParserDashboardElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserWinVerElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserMacVerElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserLinuxVerVerElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        //add by jie.li 2011-11-29 for Y580
        void ParserWEBUIVerVerElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        //end add
        void ParserUSB_DriverElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserUSBWinVerElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserUSBMacVerElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserUSBLinuxVerVerElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);

		
        void ParserSimLockElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserActionElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserCodeElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserCodePNElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserCodePUElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserCodePPElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserCodePCElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserCodePFElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserActPNElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserActPUElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserActPPElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserActPCElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserActPFElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);

        void ParserSW_configElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserBand_confElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);

        //add by yanbin.wan 2013-01-28
        void ParserQ6ResourceElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserLinuxKernelElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserLinuxAppElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserLinuxSysElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        //end add


		/************************************************************************/
		/*  dynamic_nv.xml                                                                    */
		/************************************************************************/
        void ParserNVBandElement(const typeQDomElement &element);
        void ParserUMTS2100Element(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserUMTS1700Element(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserUMTS1900Element(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserUMTS900Element(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserUMTS850Element(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        //add by jie.li 2011-06-10
        void ParserUMTS1800Element(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        //end add
        void ParserEGSM900Element(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserDCS1800Element(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserGSM850Element(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserPCS1900Element(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserDefaultElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        //add by jie.li 2011-11-29 for LTE
        void ParserLTE2100Element(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserLTE1800Element(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserLTE2600Element(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserLTE900Element(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserLTE800Element(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        //end add

        //add by yanbin.wan 2013-01-25
        void ParserLTE850Element(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserLTE1900Element(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        //end add


        //add by jie.li 2012-04-06 for Diversity NV
        void ParserDiversityElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        //end add

        /************************************************************************/
        /*  info.plist                                                                    */
        /************************************************************************/
        void ParserListElement(const typeQDomElement &element);
        void ParserDictElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserkeyElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);
        void ParserStringElement(const typeQDomElement &element,typeQTreeWidgetItem *parent);

        //add by jie.li to parse the webs_config.xml
        void ParserWebsCfgElement(const typeQDomElement &element, WebsXMLInfo* pxmlinfo);
        void ParserDeviceElement(const typeQDomElement &element,typeQTreeWidgetItem *parent, WebsXMLInfo* pxmlinfo);
        void ParserDevNameElement(const typeQDomElement &element,typeQTreeWidgetItem *parent, WebsXMLInfo* pxmlinfo);
        void ParserCustModelNameElement(const typeQDomElement &element,typeQTreeWidgetItem *parent, WebsXMLInfo* pxmlinfo);
};


#endif // DOMPARSER_H
