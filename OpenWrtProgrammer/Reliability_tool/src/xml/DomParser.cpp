

/************************************************************************/
/*  Parser XML 
DESC:

CHANGE HISTORY:
when        who             what
----------  ---------       --------------------------------------------------------
2010-10-20  zhongping.huang   Add parser mac Info.plist
2010-09-24  jianwen.he        Bug fixed and clean up the XML-KEY.
2010-08-31  zhongping.huang   Init first version

*/
/************************************************************************/
#include "DomParser.h"

DomParser::DomParser(typeQTreeWidget *tree)
{
    treeWidget = tree;
    m_XMLMap.clear();
}

bool DomParser::ParseFromBuf(typeQByteArray& xmlBuffer)
{
    typeQByteArray& buffe = xmlBuffer;
    typeQString errorstr;
	int errorLine;
	int errorCol;

    typeQDomDocument doc;
	if(!doc.setContent(buffe, false, &errorstr, &errorLine, &errorCol))
	{
		return false;
	}

    typeQDomElement root = doc.documentElement();

    if (root.tagName() == XML_KEY_CONTENT)
    {
		ParserContentElement(root);
		return true;
	} 
    else if (root.tagName() == XML_KEY_CONF_BAND)
	{
		ParserNVBandElement(root);
		return true;
	}
    else if(root.tagName() == PLIST_KEY)
    {
        ParserListElement(root);
        return true;
    }
	else
	{
		return false;
	}

	return false;
}

bool DomParser::readFile(typeQString fileName)
{
    typeQFile file(fileName);
    if(!file.open(typeQFile::ReadOnly|typeQFile::Text))
    {
        return false;
    }
    const typeQByteArray & buffe = file.readAll();
    typeQString errorstr;
    int errorLine;
    int errorCol;

    typeQDomDocument doc;
    if(!doc.setContent(buffe, false, &errorstr, &errorLine, &errorCol))
    {
        return false;
    }

    typeQDomElement root = doc.documentElement();
//     if(root.tagName() != XML_KEY_CONTENT)
//     {
//         return false;
//     }    

    if (root.tagName() == XML_KEY_CONTENT)
	{        
		ParserContentElement(root);
		return true;
	} 
    else if (root.tagName() == XML_KEY_BAND)
	{
		ParserNVBandElement(root);
		return true;
	}
    else if (root.tagName() == PLIST_KEY)
    {
        ParserListElement(root);
        return true;
    }
	else
	{
		return false;
	}

	return false;
    
}

void DomParser::ParserContentElement(const typeQDomElement &element)
{    
    //add by jie.li 2011-06-10
    m_CfgXMLVer = element.attribute(XML_ATTRIBUTE_VERSION);   //get the version of the config.xml
    //end add

    typeQDomNode child = element.firstChild();    
    while(!child.isNull())
    {
        if(child.toElement().tagName() == XML_KEY_CUSTOMERID)
        {
            ParserCustomerIDElement(child.toElement(),treeWidget->invisibleRootItem());
        }
        else if (child.toElement().tagName() == XML_KEY_PACKAGE)
		{
			ParserPackageElement(child.toElement(),treeWidget->invisibleRootItem());
		}
        else if(child.toElement().tagName() == XML_KEY_HARDWARE)
        {
            ParserHardwareElement(child.toElement(),treeWidget->invisibleRootItem());
        }
        else if (child.toElement().tagName() == XML_KEY_SOFTWARE)
		{
			ParserSoftwareElement(child.toElement(),treeWidget->invisibleRootItem());
		}
		else if (child.toElement().tagName() == "SIMLOCK")
		{
			ParserSimLockElement(child.toElement(),treeWidget->invisibleRootItem());
		}
        else if (child.toElement().tagName() == XML_KEY_SWCONFIG)
		{
			ParserSW_configElement(child.toElement(),treeWidget->invisibleRootItem());
		}
        child = child.nextSibling();
    }
}

void DomParser::ParserPackageElement(const typeQDomElement &element,typeQTreeWidgetItem *parent)
{
	m_strPackage = element.text();
}

void DomParser::ParserCustomerIDElement(const typeQDomElement &element,typeQTreeWidgetItem *parent)
{
    typeQTreeWidgetItem *item = new typeQTreeWidgetItem(parent);
    item->setText(0,element.attribute(XML_ATTRIBUTE_MANDATORY));

    m_strCustMandatory = element.attribute(XML_ATTRIBUTE_MANDATORY); //read the value of Mandatory£¬suach as:yes
	m_strCustomer = element.text(); //read the value of CustomerID£¬such as:General

        if(m_strCustMandatory == "yes")
        {
         CompareList << element.tagName();
        }

        xmlData.key = element.tagName();
        xmlData.value = element.text();
        m_XMLMap.push_back(xmlData);

}

void DomParser::ParserHardwareElement(const typeQDomElement &element,typeQTreeWidgetItem *parent)
{
    typeQTreeWidgetItem *item = new typeQTreeWidgetItem(parent);
    item->setText(0,element.attribute(XML_ATTRIBUTE_MANDATORY));
        m_strFrmMandatory = element.attribute(XML_ATTRIBUTE_MANDATORY); //read the value of Mandatory£¬suach as:yes

    typeQDomNode child = element.firstChild();
	while(!child.isNull())
	{
        if(child.toElement().tagName() == XML_KEY_BAND)
		{
            if(m_strFrmMandatory == "yes")
            {
              CompareList << child.toElement().tagName();
            }
            ParserBandElement(child.toElement(),treeWidget->invisibleRootItem());
		}
		//......
        else if (child.toElement().tagName() == XML_KEY_PCB)
		{
            if(m_strFrmMandatory == "yes")
            {
              CompareList << child.toElement().tagName();
            }
            ParserPCBElement(child.toElement(),treeWidget->invisibleRootItem());
		}
        else if(child.toElement().tagName() == XML_KEY_FLASH)
		{
            if(m_strFrmMandatory == "yes")
            {
              CompareList << child.toElement().tagName();
            }
            ParserFlashElement(child.toElement(),treeWidget->invisibleRootItem());
		}
        else if (child.toElement().tagName() == XML_KEY_RF_SWITCHER)
		{
            if(m_strFrmMandatory == "yes")
            {
              CompareList << child.toElement().tagName();
            }
            ParserRF_switcherElement(child.toElement(),treeWidget->invisibleRootItem());
		}
        else if (child.toElement().tagName() == XML_KEY_DIV_CALIBRATION)
        {
            ParserDivCalibrationElement(child.toElement(),treeWidget->invisibleRootItem());
        }
		child = child.nextSibling();
	}
}

void DomParser::ParserPCBElement(const typeQDomElement &element,typeQTreeWidgetItem *parent)
{
	m_strPCB = element.text();
    xmlData.key = element.tagName();
    xmlData.value = element.text();
    m_XMLMap.push_back(xmlData);
}

void DomParser::ParserBandElement(const typeQDomElement &element, typeQTreeWidgetItem *parent)
{
    typeQTreeWidgetItem *item = new typeQTreeWidgetItem(parent);
    //changed by jie.li 2011-06-10
    /*if(m_CfgXMLVer == CFG_XML_VER_THREE)
    {
        item->setText(0,element.attribute(XML_ATTRIBUTE_UMTS2100));
        item->setText(1,element.attribute(XML_ATTRIBUTE_UMTS1700));
        item->setText(2,element.attribute(XML_ATTRIBUTE_UMTS1900));
        item->setText(3,element.attribute(XML_ATTRIBUTE_UMTS900));
        item->setText(4,element.attribute(XML_ATTRIBUTE_UMTS850));
        item->setText(5,element.attribute(XML_ATTRIBUTE_UMTS1800));
        item->setText(6,element.attribute(XML_ATTRIBUTE_EGSM900));
        item->setText(7,element.attribute(XML_ATTRIBUTE_DCS1800));
        item->setText(8,element.attribute(XML_ATTRIBUTE_GSM850));
        item->setText(8,element.attribute(XML_ATTRIBUTE_PCS1900));
    }
    else
    {
        item->setText(0,element.attribute(XML_ATTRIBUTE_UMTS2100));
        item->setText(1,element.attribute(XML_ATTRIBUTE_UMTS1700));
        item->setText(2,element.attribute(XML_ATTRIBUTE_UMTS1900));
        item->setText(3,element.attribute(XML_ATTRIBUTE_UMTS900));
        item->setText(4,element.attribute(XML_ATTRIBUTE_UMTS850));
        item->setText(5,element.attribute(XML_ATTRIBUTE_EGSM900));
        item->setText(6,element.attribute(XML_ATTRIBUTE_DCS1800));
        item->setText(7,element.attribute(XML_ATTRIBUTE_GSM850));
        item->setText(7,element.attribute(XML_ATTRIBUTE_PCS1900));
    }*/
    //end changed

        typeQString strValue;
    strIgUMTS2100 = element.attribute(XML_ATTRIBUTE_UMTS2100);
        if(strIgUMTS2100 == "yes")
            strValue = XML_ATTRIBUTE_UMTS2100;
    strIgUMTS1700 = element.attribute(XML_ATTRIBUTE_UMTS1700);
        if(strIgUMTS1700 == "yes")
            strValue += XML_ATTRIBUTE_UMTS1700;
    strIgUMTS1900 = element.attribute(XML_ATTRIBUTE_UMTS1900);
        if(strIgUMTS1900 == "yes")
            strValue += XML_ATTRIBUTE_UMTS1900;
    strIgUMTS900 = element.attribute(XML_ATTRIBUTE_UMTS900);
        if(strIgUMTS900 == "yes")
            strValue += XML_ATTRIBUTE_UMTS900;
    strIgUMTS850 = element.attribute(XML_ATTRIBUTE_UMTS850);
        if(strIgUMTS850 == "yes")
            strValue += XML_ATTRIBUTE_UMTS850;
    //add by jie.li 2011-11-29 for Y580
    strIgUMTS800 = element.attribute(XML_ATTRIBUTE_UMTS800);
    if (!strIgUMTS800.isNull())
    {
        if(strIgUMTS800 == "yes")
            strValue += XML_ATTRIBUTE_UMTS800;
    }
    //end add
    //add by jie.li 2011-06-10
    /*if(m_CfgXMLVer == CFG_XML_VER_THREE)
    {
        strIgUMTS1800 = element.attribute(XML_ATTRIBUTE_UMTS1800);
        if(strIgUMTS1800 == "yes")
            strValue += XML_ATTRIBUTE_UMTS1800;
    }*/
    //end add
    //changed by jie.li 2011-11-29
    strIgUMTS1800 = element.attribute(XML_ATTRIBUTE_UMTS1800);
    if (!strIgUMTS1800.isNull())
    {
        if(strIgUMTS1800 == "yes")
            strValue += XML_ATTRIBUTE_UMTS1800;
    }
    //end changed
    strIgEGSM900 = element.attribute(XML_ATTRIBUTE_EGSM900);
        if(strIgEGSM900 == "yes")
            strValue += XML_ATTRIBUTE_EGSM900;
    strIgDCS1800 = element.attribute(XML_ATTRIBUTE_DCS1800);
        if(strIgDCS1800 == "yes")
            strValue += XML_ATTRIBUTE_DCS1800;
    strIgGSM850 = element.attribute(XML_ATTRIBUTE_GSM850);
        if(strIgGSM850 == "yes")
            strValue += XML_ATTRIBUTE_GSM850;
    strIgPCS1900  = element.attribute(XML_ATTRIBUTE_PCS1900);
        if(strIgPCS1900 == "yes")
            strValue += XML_ATTRIBUTE_PCS1900;
    //add by jie.li 2011-11-29
    strIgLTE2100  = element.attribute(XML_ATTRIBUTE_LTE2100);
    if (!strIgLTE2100.isNull())
    {
        if (strIgLTE2100 == "yes")
            strValue += XML_ATTRIBUTE_LTE2100;
    }

    strIgLTE1800  = element.attribute(XML_ATTRIBUTE_LTE1800);
    if (!strIgLTE1800.isNull())
    {
        if (strIgLTE1800 == "yes")
            strValue += XML_ATTRIBUTE_LTE1800;
    }

    strIgLTE2600  = element.attribute(XML_ATTRIBUTE_LTE2600);
    if (!strIgLTE2600.isNull())
    {
        if (strIgLTE2600 == "yes")
            strValue += XML_ATTRIBUTE_LTE2600;
    }

    strIgLTE900  = element.attribute(XML_ATTRIBUTE_LTE900);
    if (!strIgLTE900.isNull())
    {
        if (strIgLTE900 == "yes")
            strValue += XML_ATTRIBUTE_LTE900;
    }

    strIgLTE800  = element.attribute(XML_ATTRIBUTE_LTE800);
    if (!strIgLTE800.isNull())
    {
        if (strIgLTE800 == "yes")
            strValue += XML_ATTRIBUTE_LTE800;
    }
    //end add

    //add by jie.li 2012-10-16 for L100G
    strIgLTE700  = element.attribute(XML_ATTRIBUTE_LTE700);
    if (!strIgLTE700.isNull())
    {
        if (strIgLTE700 == "yes")
            strValue += XML_ATTRIBUTE_LTE700;
    }

    strIgLTE1700  = element.attribute(XML_ATTRIBUTE_LTE1700);
    if (!strIgLTE1700.isNull())
    {
        if (strIgLTE1700 == "yes")
            strValue += XML_ATTRIBUTE_LTE1700;
    }
    //end add

    //add by yanbin.wan 2013-01-25 for LTE band (O)
    strIgLTE850 = element.attribute(XML_ATTRIBUTE_LTE850);
    if (!strIgLTE850.isNull())
    {
        if (strIgLTE850 == "yes")
        {
            strValue += XML_ATTRIBUTE_LTE850;
        }
    }

    strIgLTE1900 = element.attribute(XML_ATTRIBUTE_LTE1900);
    if (!strIgLTE1900.isNull())
    {
        if (strIgLTE1900 == "yes")
        {
            strValue += XML_ATTRIBUTE_LTE1900;
        }
    }
    //end add

    typeQString  strBandValue = element.text();

    xmlData.key = element.tagName();
    xmlData.value = strValue;   //all the band
    m_XMLMap.push_back(xmlData);
}

void DomParser::ParserFlashElement(const typeQDomElement &element,typeQTreeWidgetItem *parent)
{
    typeQTreeWidgetItem *item = new typeQTreeWidgetItem(parent);
	item->setText(0,element.attribute("vendor"));
	item->setText(1,element.attribute("nand"));
	item->setText(2,element.attribute("sdram"));

	strvendor = element.attribute("vendor");
	strnand = element.attribute("nand");
	strsdram = element.attribute("sdram");
	
	m_strFlashValue = element.text();
    xmlData.key = element.tagName();
    xmlData.value = element.text();
    m_XMLMap.push_back(xmlData);
}

void DomParser::ParserRF_switcherElement(const typeQDomElement &element,typeQTreeWidgetItem *parent)
{
	m_RFswitch = element.text();
    xmlData.key = element.tagName();
    xmlData.value = element.text();
    m_XMLMap.push_back(xmlData);
}

//add by jie.li
void DomParser::ParserDivCalibrationElement(const QDomElement &element, QTreeWidgetItem *parent)
{
    m_strDiversity = element.text();
}
//end add

void DomParser::ParserSoftwareElement(const typeQDomElement &element,typeQTreeWidgetItem *parent)
{
    typeQDomNode child = element.firstChild();
	while(!child.isNull())
	{
        if(child.toElement().tagName() == XML_KEY_DASHBOARD)
		{
			ParserDashboardElement(child.toElement(),treeWidget->invisibleRootItem());
		}
		//......
		else if(child.toElement().tagName() == "Partition")
		{
            ParserPartitionElement(child.toElement(),treeWidget->invisibleRootItem());
		}
        else if (child.toElement().tagName() == XML_KEY_FIRMWARE_VER)
		{
			ParserFirInterVerElement(child.toElement(),treeWidget->invisibleRootItem());
		}
        else if (child.toElement().tagName() == XML_KEY_FIRMWARE_EX_VER)
		{
			ParserFirExterVerElement(child.toElement(),treeWidget->invisibleRootItem());
		}
		else if (child.toElement().tagName() == "USB_Driver")
		{
			ParserUSB_DriverElement(child.toElement(),treeWidget->invisibleRootItem());
		}
        else if (child.toElement().tagName() == XML_KEY_QCN_VER)
        {
            ParserQCNElement(child.toElement(),treeWidget->invisibleRootItem());
        }
        //add by jie.li 2011-09-28
        else if (child.toElement().tagName() == XML_KEY_LTEBANDCFG)
        {
            ParserLTEBANDCFGElement(child.toElement(),treeWidget->invisibleRootItem());
        }
        //end add
        //add by yanbin.wan 2013-01-28
        else if(child.toElement().tagName() == XML_KEY_Q6_RESOURCE_VER)
        {
            ParserQ6ResourceElement(child.toElement(),treeWidget->invisibleRootItem());
        }
        else if(child.toElement().tagName() == XML_KEY_LINUX_KERNEL_VER)
        {
            ParserLinuxKernelElement(child.toElement(),treeWidget->invisibleRootItem());
        }
        else if(child.toElement().tagName() == XML_KEY_LINUX_APP_VER)
        {
            ParserLinuxAppElement(child.toElement(),treeWidget->invisibleRootItem());
        }
        else if(child.toElement().tagName() == XML_KEY_LINUX_SYS_VER)
        {
            ParserLinuxSysElement(child.toElement(),treeWidget->invisibleRootItem());
        }
        //end add
		child = child.nextSibling();
	}
}

void DomParser::ParserPartitionElement(const typeQDomElement &element,typeQTreeWidgetItem *parent)
{
    typeQTreeWidgetItem *item = new typeQTreeWidgetItem(parent);
    item->setText(0,element.attribute(XML_ATTRIBUTE_MANDATORY));

    m_strPartMandatory = element.attribute(XML_ATTRIBUTE_MANDATORY);
	m_strPartition = element.text();

    if(m_strPartMandatory == "yes")
    {
     CompareList << element.tagName();
    }
    xmlData.key = element.tagName();
    xmlData.value = element.text();
    m_XMLMap.push_back(xmlData);
}

void DomParser::ParserFirInterVerElement(const typeQDomElement &element,typeQTreeWidgetItem *parent)
{
    typeQTreeWidgetItem *item = new typeQTreeWidgetItem(parent);
    item->setText(0,element.attribute(XML_ATTRIBUTE_MANDATORY));

    m_strFirInterMandatory = element.attribute(XML_ATTRIBUTE_MANDATORY);
	m_strFirInterVer = element.text();
    if(m_strFirInterMandatory == "yes")
    {
     CompareList << element.tagName();
    }
    xmlData.key = element.tagName();
    xmlData.value = element.text();
    m_XMLMap.push_back(xmlData);
}

void DomParser::ParserFirExterVerElement(const typeQDomElement &element,typeQTreeWidgetItem *parent)
{
    typeQTreeWidgetItem *item = new typeQTreeWidgetItem(parent);
    item->setText(0,element.attribute(XML_ATTRIBUTE_MANDATORY));

    m_strFirExterMandatory = element.attribute(XML_ATTRIBUTE_MANDATORY);
	m_strFirExterVer = element.text();
    if(m_strFirExterMandatory == "yes")
    {
     CompareList << element.tagName();
    }
    xmlData.key = element.tagName();
    xmlData.value = element.text();
    m_XMLMap.push_back(xmlData);
}

void DomParser::ParserQCNElement(const typeQDomElement &element,typeQTreeWidgetItem *parent)
{
    typeQTreeWidgetItem *item = new typeQTreeWidgetItem(parent);
    item->setText(0,element.attribute(XML_ATTRIBUTE_MANDATORY));

    m_strQCNMandatory = element.attribute(XML_ATTRIBUTE_MANDATORY);
	m_strQCN= element.text();
    if(m_strQCNMandatory == "yes")
    {
     CompareList << element.tagName();
    }
    xmlData.key = element.tagName();
    xmlData.value = element.text();
    m_XMLMap.push_back(xmlData);
}

//add by jie.li 2011-09-28
void DomParser::ParserLTEBANDCFGElement(const typeQDomElement &element, typeQTreeWidgetItem *parent)
{
    typeQTreeWidgetItem *item = new typeQTreeWidgetItem(parent);
    item->setText(0,element.attribute(XML_ATTRIBUTE_MANDATORY));

    m_strLTEMandatory = element.attribute(XML_ATTRIBUTE_MANDATORY);
    m_strLTE= element.text();
    if(m_strLTEMandatory == "yes")
    {
     CompareList << element.tagName();
    }
    xmlData.key = element.tagName();
    xmlData.value = element.text();
    m_XMLMap.push_back(xmlData);
}
//end add

void DomParser::ParserDashboardElement(const typeQDomElement &element, typeQTreeWidgetItem *parent)
{
    typeQTreeWidgetItem *item = new typeQTreeWidgetItem(parent);
    item->setText(0,element.attribute(XML_ATTRIBUTE_MANDATORY));
    m_strDashboardMandatory = element.attribute(XML_ATTRIBUTE_MANDATORY);


    typeQDomNode child = element.firstChild();
	while(!child.isNull())
	{
        if(child.toElement().tagName() == XML_KEY_WINDOWS_VER)
		{
            if(m_strDashboardMandatory == "yes")
            {
              CompareList << child.toElement().tagName();
            }
            ParserWinVerElement(child.toElement(),treeWidget->invisibleRootItem());
		}
        else if(child.toElement().tagName() == XML_KEY_MAC_VER)
		{
            if(m_strDashboardMandatory == "yes")
            {
              CompareList << child.toElement().tagName();
            }
			ParserMacVerElement(child.toElement(),treeWidget->invisibleRootItem());
		}
        else if(child.toElement().tagName() == XML_KEY_LINUX_VER)
		{
            if(m_strDashboardMandatory == "yes")
            {
              CompareList << child.toElement().tagName();
            }
            ParserLinuxVerVerElement(child.toElement(),treeWidget->invisibleRootItem());
        }
        //add by jie.li 2011-11-29 for Y580
        else if(child.toElement().tagName() == XML_KEY_WEBUI_VER)
        {
            if(m_strDashboardMandatory == "yes")
            {
              CompareList << child.toElement().tagName();
            }
            ParserWEBUIVerVerElement(child.toElement(),treeWidget->invisibleRootItem());
        }
        //end add
		child = child.nextSibling();
	}
}

void DomParser::ParserWinVerElement(const typeQDomElement &element, typeQTreeWidgetItem *parent)
{
    typeQString  strWinVer = element.text();
	m_strWinVer = strWinVer; 
    xmlData.key = element.tagName();
    xmlData.value = element.text();
    m_XMLMap.push_back(xmlData);
}

void DomParser::ParserMacVerElement(const typeQDomElement &element, typeQTreeWidgetItem *parent)
{
    typeQString  strMacVer = element.text();
	m_strMacVer = strMacVer;
    xmlData.key = element.tagName();
    xmlData.value = element.text();
    m_XMLMap.push_back(xmlData);
}

void DomParser::ParserLinuxVerVerElement(const typeQDomElement &element, typeQTreeWidgetItem *parent)
{
    typeQString  strLinuxVer = element.text();
	m_strLinuxVer = strLinuxVer;
    xmlData.key = element.tagName();
    xmlData.value = element.text();
    m_XMLMap.push_back(xmlData);
}

//add by jie.li 2011-11-29 for y580
void DomParser::ParserWEBUIVerVerElement(const typeQDomElement &element, typeQTreeWidgetItem *parent)
{
    typeQString  strWEBUIVer = element.text();
    m_strWEBUIVer = strWEBUIVer;
    xmlData.key = element.tagName();
    xmlData.value = element.text();
    m_XMLMap.push_back(xmlData);
}
//end add

void DomParser::ParserUSB_DriverElement(const typeQDomElement &element, typeQTreeWidgetItem *parent)
{
    typeQTreeWidgetItem *item = new typeQTreeWidgetItem(parent);
    item->setText(0,element.attribute(XML_ATTRIBUTE_MANDATORY));
    m_strUSBMandatory = element.attribute(XML_ATTRIBUTE_MANDATORY);

    if(m_strUSBMandatory == "yes")
    {
     CompareList << element.tagName();
    }

    typeQDomNode child = element.firstChild();
	while(!child.isNull())
	{
        if(child.toElement().tagName() == XML_KEY_WINDOWS_VER)
		{
			ParserUSBWinVerElement(child.toElement(),treeWidget->invisibleRootItem());
		}
        else if(child.toElement().tagName() == XML_KEY_MAC_VER)
		{
			ParserUSBMacVerElement(child.toElement(),treeWidget->invisibleRootItem());
		}
        else if(child.toElement().tagName() == XML_KEY_LINUX_VER)
		{
			ParserUSBLinuxVerVerElement(child.toElement(),treeWidget->invisibleRootItem());
		}
		child = child.nextSibling();
	}
}

void DomParser::ParserUSBWinVerElement(const typeQDomElement &element, typeQTreeWidgetItem *parent)
{
	m_strUSBWinVer = element.text();
}

void DomParser::ParserUSBMacVerElement(const typeQDomElement &element, typeQTreeWidgetItem *parent)
{
	m_strUSBMacVer = element.text();
}

void DomParser::ParserUSBLinuxVerVerElement(const typeQDomElement &element, typeQTreeWidgetItem *parent)
{
	m_strUSBLinuxVer = element.text();
}

void DomParser::ParserSimLockElement(const typeQDomElement &element,typeQTreeWidgetItem *parent)
{
    typeQDomNode child = element.firstChild();
	while(!child.isNull())
	{
		if(child.toElement().tagName() == "Code")
		{
			ParserCodeElement(child.toElement(),treeWidget->invisibleRootItem());
		}
		else if(child.toElement().tagName() == "Action")
		{
			ParserActionElement(child.toElement(),treeWidget->invisibleRootItem());
		}
		child = child.nextSibling();
	}
}

void DomParser::ParserActionElement(const typeQDomElement &element, typeQTreeWidgetItem *parent)
{
    typeQDomNode child = element.firstChild();
	while(!child.isNull())
	{
		if(child.toElement().tagName() == "PN")
		{
			//ParserActPNElement(child.toElement(),treeWidget->invisibleRootItem());
			m_strActPN = child.toElement().text();
		}
		else if(child.toElement().tagName() == "PU")
		{
			m_strActPU = child.toElement().text();
		}
		else if(child.toElement().tagName() == "PP")
		{
			m_strActPP = child.toElement().text();
		}
		else if(child.toElement().tagName() == "PC")
		{
			m_strActPC = child.toElement().text();
		}
		else if(child.toElement().tagName() == "PF")
		{
			m_strActPF = child.toElement().text();
		}
		child = child.nextSibling();
	}
}

void DomParser::ParserCodeElement(const typeQDomElement &element, typeQTreeWidgetItem *parent)
{
    typeQDomNode child = element.firstChild();
	while(!child.isNull())
	{
		if(child.toElement().tagName() == "PN")
		{
			//ParserCodePNElement(child.toElement(),treeWidget->invisibleRootItem());
			m_strCodePN = child.toElement().text();
		}
		else if(child.toElement().tagName() == "PU")
		{
			m_strCodePU = child.toElement().text();
		}
		else if(child.toElement().tagName() == "PP")
		{
			m_strCodePP = child.toElement().text();
		}
		else if(child.toElement().tagName() == "PC")
		{
			m_strCodePC = child.toElement().text();
		}
		else if(child.toElement().tagName() == "PF")
		{
			m_strCodePF = child.toElement().text();
		}
		child = child.nextSibling();
	}
}

void DomParser::ParserSW_configElement(const typeQDomElement &element, typeQTreeWidgetItem *parent)
{
    typeQDomNode child = element.firstChild();
	while(!child.isNull())
	{
        if(child.toElement().tagName() == XML_KEY_HSU_COMPID)
		{
			m_strHSU_Comp_id = child.toElement().text();
			xmlData.key = child.toElement().tagName();
			xmlData.value = child.toElement().text();
			m_XMLMap.push_back(xmlData);
		}
        else if(child.toElement().tagName() == XML_KEY_VOICE_ENABLE)
		{
			m_strVoice = child.toElement().text();
			xmlData.key = child.toElement().tagName();
			xmlData.value = child.toElement().text();
			m_XMLMap.push_back(xmlData);
		}
        else if(child.toElement().tagName() == XML_KEY_SNIMEI_ENABLE)
		{
			m_strIMEI = child.toElement().text();
			xmlData.key = child.toElement().tagName();
			xmlData.value = child.toElement().text();
			m_XMLMap.push_back(xmlData);
		}
        else if(child.toElement().tagName() == XML_KEY_SPECIAL_AT)
		{
			m_strSpecial_AT = child.toElement().text();
			xmlData.key = child.toElement().tagName();
			xmlData.value = child.toElement().text();
			m_XMLMap.push_back(xmlData);
		}
        else if(child.toElement().tagName() == XML_KEY_DISK_ATTR)
		{
			m_strDisk_attrI = child.toElement().text();
			xmlData.key = child.toElement().tagName();
			xmlData.value = child.toElement().text();
			m_XMLMap.push_back(xmlData);
		}
        else if(child.toElement().tagName() == XML_KEY_SETUP_MOPDE)
		{
			m_strDevmopde = child.toElement().text();
			xmlData.key = child.toElement().tagName();
			xmlData.value = child.toElement().text();
			m_XMLMap.push_back(xmlData);
		}
        else if(child.toElement().tagName() == XML_KEY_LED_MODE)
		{
			m_strLed = child.toElement().text();
			xmlData.key = child.toElement().tagName();
			xmlData.value = child.toElement().text();
			m_XMLMap.push_back(xmlData);
		}
        else if(child.toElement().tagName() == XML_KEY_EFS_DIR_ENABLE)
		{
			m_strEFS = child.toElement().text();
			xmlData.key = child.toElement().tagName();
			xmlData.value = child.toElement().text();
			m_XMLMap.push_back(xmlData);
		}
        else if(child.toElement().tagName() == XML_KEY_DIAG_ENABLE)
		{
			m_strDigPort = child.toElement().text();
			xmlData.key = child.toElement().tagName();
			xmlData.value = child.toElement().text();
			m_XMLMap.push_back(xmlData);
		}
        else if(child.toElement().tagName() == XML_KEY_BANDCONF)
		{
			ParserBand_confElement(child.toElement(),treeWidget->invisibleRootItem());
		}
		child = child.nextSibling();
	}
}

void DomParser::ParserBand_confElement(const typeQDomElement &element,typeQTreeWidgetItem *parent)
{
    typeQDomNode child = element.firstChild();
	while(!child.isNull())
	{
        if(child.toElement().tagName() == XML_KEY_BAND)
		{
			m_strBand = child.toElement().text();
		}
        else if(child.toElement().tagName() == XML_KEY_MODE)
		{
			m_strMode = child.toElement().text();
		}
        else if(child.toElement().tagName() == XML_KEY_ORDER)
		{
			m_strOrder = child.toElement().text();
		}
        else if(child.toElement().tagName() == XML_KEY_DOMAIN)
		{
			m_strDomain = child.toElement().text();
		}

            if(child.toElement().tagName() == XML_KEY_BAND)
                xmlData.key = XML_KEY_BANDCONF;
            else
                xmlData.key = child.toElement().tagName();

		xmlData.value = child.toElement().text();
		m_XMLMap.push_back(xmlData);
		
		child = child.nextSibling();
	}

}

/************************************************************************/
/*  for dynamic_nv.xml  Begin                                                                  */
/************************************************************************/
void DomParser::ParserNVBandElement(const typeQDomElement &element)
{
    m_strBandVersion = element.attribute(XML_ATTRIBUTE_VERSION);
    typeQDomNode child = element.firstChild();
	while(!child.isNull())
	{
        //add by jie.li 2011-06-10
        /*if(m_strBandVersion == DYNAMIC_XML_VER_THREE)
        {
            if (child.toElement().tagName() == XML_ATTRIBUTE_UMTS1800)
            {
                ParserUMTS1800Element(child.toElement(),treeWidget->invisibleRootItem());
            }
        }*/
        //end add
        //changed by jie.li 2011-11-29
        if (child.toElement().tagName() == XML_ATTRIBUTE_UMTS1800)
        {
            ParserUMTS1800Element(child.toElement(),treeWidget->invisibleRootItem());
        }
        //end changed
        if(child.toElement().tagName() == XML_ATTRIBUTE_UMTS2100)
		{
			ParserUMTS2100Element(child.toElement(),treeWidget->invisibleRootItem());
		}
        else if (child.toElement().tagName() == XML_ATTRIBUTE_UMTS1700)
		{
			ParserUMTS1700Element(child.toElement(),treeWidget->invisibleRootItem());
		}
        else if(child.toElement().tagName() == XML_ATTRIBUTE_UMTS1900)
		{
			ParserUMTS1900Element(child.toElement(),treeWidget->invisibleRootItem());
		}
        else if (child.toElement().tagName() == XML_ATTRIBUTE_UMTS900)
		{
			ParserUMTS900Element(child.toElement(),treeWidget->invisibleRootItem());
		}
        else if (child.toElement().tagName() == XML_ATTRIBUTE_UMTS850)
		{
			ParserUMTS850Element(child.toElement(),treeWidget->invisibleRootItem());
		}        
        else if (child.toElement().tagName() == XML_ATTRIBUTE_EGSM900)
		{
			ParserEGSM900Element(child.toElement(),treeWidget->invisibleRootItem());
		}
        else if (child.toElement().tagName() == XML_ATTRIBUTE_DCS1800)
		{
			ParserDCS1800Element(child.toElement(),treeWidget->invisibleRootItem());
		}
        else if (child.toElement().tagName() == XML_ATTRIBUTE_GSM850)
		{
			ParserGSM850Element(child.toElement(),treeWidget->invisibleRootItem());
		}
        else if (child.toElement().tagName() == XML_ATTRIBUTE_PCS1900)
		{
			ParserPCS1900Element(child.toElement(),treeWidget->invisibleRootItem());
		}
		else if (child.toElement().tagName() == "Default")
		{
			ParserDefaultElement(child.toElement(),treeWidget->invisibleRootItem());
		}

        //add by jie.li 2011-11-29
        if (child.toElement().tagName() == XML_ATTRIBUTE_LTE2100)
        {
            ParserLTE2100Element(child.toElement(),treeWidget->invisibleRootItem());
        }

        if (child.toElement().tagName() == XML_ATTRIBUTE_LTE1800)
        {
            ParserLTE1800Element(child.toElement(),treeWidget->invisibleRootItem());
        }

        if (child.toElement().tagName() == XML_ATTRIBUTE_LTE2600)
        {
            ParserLTE2600Element(child.toElement(),treeWidget->invisibleRootItem());
        }

        if (child.toElement().tagName() == XML_ATTRIBUTE_LTE900)
        {
            ParserLTE900Element(child.toElement(),treeWidget->invisibleRootItem());
        }

        if (child.toElement().tagName() == XML_ATTRIBUTE_LTE800)
        {
            ParserLTE800Element(child.toElement(),treeWidget->invisibleRootItem());
        }

        if (child.toElement().tagName() == XML_ATTRIBUTE_LTE850)
        {
            ParserLTE850Element(child.toElement(),treeWidget->invisibleRootItem());
        }

        if (child.toElement().tagName() == XML_ATTRIBUTE_LTE1900)
        {
            ParserLTE1900Element(child.toElement(),treeWidget->invisibleRootItem());
        }

        //end add

        //add by jie.li 2012-04-06 for Diversity NV
        if (child.toElement().tagName() == XML_ATTRIBUTE_DIVERSITY)
        {
            ParserDiversityElement(child.toElement(),treeWidget->invisibleRootItem());
        }
        //end add

		child = child.nextSibling();
	}

}

void DomParser::ParserUMTS2100Element(const typeQDomElement &element,typeQTreeWidgetItem *parent)
{
	m_UMTS2100Band.clear();
    typeQDomNode child = element.firstChild();
	while(!child.isNull())
	{	
        typeQString strValue = child.toElement().text();
		m_UMTS2100Band.push_back(strValue);
		child = child.nextSibling();
	}
}

void DomParser::ParserUMTS1700Element(const typeQDomElement &element,typeQTreeWidgetItem *parent)
{
	m_UMTS1700Band.clear();
    typeQDomNode child = element.firstChild();
	while(!child.isNull())
	{	
        typeQString strValue = child.toElement().text();
		m_UMTS1700Band.push_back(strValue);
		child = child.nextSibling();
	}
}

void DomParser::ParserUMTS1900Element(const typeQDomElement &element,typeQTreeWidgetItem *parent)
{
	m_UMTS1900Band.clear();
    typeQDomNode child = element.firstChild();
	while(!child.isNull())
	{	
        typeQString strValue = child.toElement().text();
		m_UMTS1900Band.push_back(strValue);
		child = child.nextSibling();
	}
}


void DomParser::ParserUMTS900Element(const typeQDomElement &element,typeQTreeWidgetItem *parent)
{
	m_UMTS900Band.clear();
    typeQDomNode child = element.firstChild();
	while(!child.isNull())
	{	
        typeQString strValue = child.toElement().text();
		m_UMTS900Band.push_back(strValue);
		child = child.nextSibling();
	}
}

void DomParser::ParserUMTS850Element(const typeQDomElement &element,typeQTreeWidgetItem *parent)
{
	m_UMTS850Band.clear();
    typeQDomNode child = element.firstChild();
	while(!child.isNull())
	{	
        typeQString strValue = child.toElement().text();
		m_UMTS850Band.push_back(strValue);
		child = child.nextSibling();
	}
}

//add by jie.li 2011-06-10
void DomParser::ParserUMTS1800Element(const typeQDomElement &element,typeQTreeWidgetItem *parent)
{
    m_UMTS1800Band.clear();
    typeQDomNode child = element.firstChild();
    while(!child.isNull())
    {
        typeQString strValue = child.toElement().text();
        m_UMTS1800Band.push_back(strValue);
        child = child.nextSibling();
    }
}
//end add

void DomParser::ParserEGSM900Element(const typeQDomElement &element,typeQTreeWidgetItem *parent)
{
	m_EGSM900Band.clear();
    typeQDomNode child = element.firstChild();
	while(!child.isNull())
	{	
        typeQString strValue = child.toElement().text();
		m_EGSM900Band.push_back(strValue);
		child = child.nextSibling();
	}
}
void DomParser::ParserDCS1800Element(const typeQDomElement &element,typeQTreeWidgetItem *parent)
{
	m_DCS1800Band.clear();
    typeQDomNode child = element.firstChild();
	while(!child.isNull())
	{	
        typeQString strValue = child.toElement().text();
		m_DCS1800Band.push_back(strValue);
		child = child.nextSibling();
	}
}
void DomParser::ParserGSM850Element(const typeQDomElement &element,typeQTreeWidgetItem *parent)
{
	m_GSM850Band.clear();
    typeQDomNode child = element.firstChild();
	while(!child.isNull())
	{	
        typeQString strValue = child.toElement().text();
		m_GSM850Band.push_back(strValue);
		child = child.nextSibling();
	}
}
void DomParser::ParserPCS1900Element(const typeQDomElement &element,typeQTreeWidgetItem *parent)
{
	m_PCS1900Band.clear();
    typeQDomNode child = element.firstChild();
	while(!child.isNull())
	{	
        typeQString strValue = child.toElement().text();
		m_PCS1900Band.push_back(strValue);
		child = child.nextSibling();
	}
}
void DomParser::ParserDefaultElement(const typeQDomElement &element,typeQTreeWidgetItem *parent)
{
	m_DefaultBand.clear();
    typeQDomNode child = element.firstChild();
	while(!child.isNull())
	{	
        typeQString strValue = child.toElement().text();
		m_DefaultBand.push_back(strValue);
		child = child.nextSibling();
	}
}

//add by jie.li 2011-11-29 for LTE
void DomParser::ParserLTE2100Element(const typeQDomElement &element,typeQTreeWidgetItem *parent)
{
    m_LTE2100Band.clear();
    typeQDomNode child = element.firstChild();
    while(!child.isNull())
    {
        typeQString strValue = child.toElement().text();
        m_LTE2100Band.push_back(strValue);
        child = child.nextSibling();
    }
}

void DomParser::ParserLTE1800Element(const typeQDomElement &element,typeQTreeWidgetItem *parent)
{
    m_LTE1800Band.clear();
    typeQDomNode child = element.firstChild();
    while(!child.isNull())
    {
        typeQString strValue = child.toElement().text();
        m_LTE1800Band.push_back(strValue);
        child = child.nextSibling();
    }
}

void DomParser::ParserLTE2600Element(const typeQDomElement &element,typeQTreeWidgetItem *parent)
{
    m_LTE2600Band.clear();
    typeQDomNode child = element.firstChild();
    while(!child.isNull())
    {
        typeQString strValue = child.toElement().text();
        m_LTE2600Band.push_back(strValue);
        child = child.nextSibling();
    }
}

void DomParser::ParserLTE900Element(const typeQDomElement &element,typeQTreeWidgetItem *parent)
{
    m_LTE900Band.clear();
    typeQDomNode child = element.firstChild();
    while(!child.isNull())
    {
        typeQString strValue = child.toElement().text();
        m_LTE900Band.push_back(strValue);
        child = child.nextSibling();
    }
}

void DomParser::ParserLTE800Element(const typeQDomElement &element,typeQTreeWidgetItem *parent)
{
    m_LTE800Band.clear();
    typeQDomNode child = element.firstChild();
    while(!child.isNull())
    {
        typeQString strValue = child.toElement().text();
        m_LTE800Band.push_back(strValue);
        child = child.nextSibling();
    }
}
//end add

void DomParser::ParserDiversityElement(const QDomElement &element, QTreeWidgetItem *parent)
{
    m_DiversityBand.clear();
    typeQDomNode child = element.firstChild();
    while(!child.isNull())
    {
        typeQString strValue = child.toElement().text();
        m_DiversityBand.push_back(strValue);
        child = child.nextSibling();
    }
}

/************************************************************************/
/*  for Mac info.plist  Begin                                                                  */
/************************************************************************/
void DomParser::ParserListElement(const typeQDomElement &element)
{
    typeQDomNode child = element.firstChild();
    while(!child.isNull())
    {
        if(child.toElement().tagName() == PLIST_KEY_DICT)
        {
            ParserDictElement(child.toElement(),treeWidget->invisibleRootItem());
        }
        child = child.nextSibling();
    }
}

void DomParser::ParserDictElement(const typeQDomElement &element, typeQTreeWidgetItem *parent)
{
    typeQTreeWidgetItem *item = new typeQTreeWidgetItem(parent);

    typeQDomNode child = element.firstChild();
    while(!child.isNull())
    {
        if(child.toElement().tagName() == PLIST_KEY_KEY)
        {
            if(child.toElement().text() == "CFBundleVersion")
            {
                child = child.nextSibling();
                pcVersion = child.toElement().text();
                break;
            }
            else if(child.toElement().text() == "Installation Path")
            {
                child = child.nextSibling();
                pcInstallPath = child.toElement().text();
                break;
            }
        }

        child = child.nextSibling();
    }
}

bool DomParser::ParseSsidFromBuf(typeQByteArray& xmlBuffer, WebsXMLInfo* pxmlinfo)
{
    typeQByteArray& buffe = xmlBuffer;
    qDebug()<<QString(xmlBuffer);
    typeQString errorstr = "";
    int errorLine = 0;
    int errorCol = 0;

    typeQDomDocument doc;
    if(!doc.setContent(buffe, false, &errorstr, &errorLine, &errorCol))
    {
        return false;
    }

    typeQDomElement root = doc.documentElement();

    if (root.tagName() == "webs_config")
    {
        ParserWebsCfgElement(root, pxmlinfo);
        return true;
    }  

    return false;
}

void DomParser::ParserWebsCfgElement(const typeQDomElement &element, WebsXMLInfo* pxmlinfo)
{
    typeQDomNode child = element.firstChild();
    while(!child.isNull())
    {
        if(child.toElement().tagName() == "device")
        {
            ParserDeviceElement(child.toElement(),treeWidget->invisibleRootItem(), pxmlinfo);
        }

        child = child.nextSibling();
    }
}

void DomParser::ParserDeviceElement(const typeQDomElement &element,typeQTreeWidgetItem *parent, WebsXMLInfo* pxmlinfo)
{
    typeQDomNode child = element.firstChild();
    while(!child.isNull())
    {
        if(child.toElement().tagName() == "dev_name")
        {
            ParserDevNameElement(child.toElement(),treeWidget->invisibleRootItem(), pxmlinfo);
        }
        else if(child.toElement().tagName() == "cust_model_name")
        {
            ParserCustModelNameElement(child.toElement(),treeWidget->invisibleRootItem(), pxmlinfo);
        }

        child = child.nextSibling();
    }
}

void DomParser::ParserDevNameElement(const typeQDomElement &element,typeQTreeWidgetItem *parent, WebsXMLInfo* pxmlinfo)
{
    typeQTreeWidgetItem *item = new typeQTreeWidgetItem(parent);
    QString strText = element.text();

    strText.toWCharArray(pxmlinfo->webs_dev_name);
}

void DomParser::ParserCustModelNameElement(const typeQDomElement &element,typeQTreeWidgetItem *parent, WebsXMLInfo* pxmlinfo)
{
    typeQTreeWidgetItem *item = new typeQTreeWidgetItem(parent);
    item->setText(0,element.attribute("Ext_SSID_Rule"));

    QString strAtt = element.attribute("Ext_SSID_Rule");
    QString strText = element.text();

    strAtt.toWCharArray(pxmlinfo->webs_Ext_SSID);
    strText.toWCharArray(pxmlinfo->webs_cust_model_name);
}

//add by yanbin.wan 2013-01-25
void DomParser::ParserLTE850Element(const typeQDomElement &element,typeQTreeWidgetItem *parent)
{
    m_LTE850Band.clear();
    typeQDomNode child = element.firstChild();
    while(!child.isNull())
    {
        typeQString strValue = child.toElement().text();
        m_LTE850Band.push_back(strValue);
        child = child.nextSibling();
    }
}
void DomParser::ParserLTE1900Element(const typeQDomElement &element,typeQTreeWidgetItem *parent)
{
    m_LTE1900Band.clear();
    typeQDomNode child = element.firstChild();
    while(!child.isNull())
    {
        typeQString strValue = child.toElement().text();
        m_LTE1900Band.push_back(strValue);
        child = child.nextSibling();
    }
}
//end add
void DomParser::ParserQ6ResourceElement(const typeQDomElement &element, typeQTreeWidgetItem *parent)
{
    typeQTreeWidgetItem *item = new typeQTreeWidgetItem(parent);
    item->setText(0,element.attribute(XML_ATTRIBUTE_MANDATORY));

    m_strQ6ResourceMandatory = element.attribute(XML_ATTRIBUTE_MANDATORY);
    m_strQ6Resource= element.text();
    if(m_strQ6ResourceMandatory == "yes")
    {
     CompareList << element.tagName();
    }
    xmlData.key = element.tagName();
    xmlData.value = element.text();
    m_XMLMap.push_back(xmlData);
}

void DomParser::ParserLinuxKernelElement(const typeQDomElement &element, typeQTreeWidgetItem *parent)
{
    typeQTreeWidgetItem *item = new typeQTreeWidgetItem(parent);
    item->setText(0,element.attribute(XML_ATTRIBUTE_MANDATORY));

    m_strLinuxKernelMandatory = element.attribute(XML_ATTRIBUTE_MANDATORY);
    m_strLinuxKernel= element.text();
    if(m_strLinuxKernelMandatory == "yes")
    {
     CompareList << element.tagName();
    }
    xmlData.key = element.tagName();
    xmlData.value = element.text();
    m_XMLMap.push_back(xmlData);
}

void DomParser::ParserLinuxAppElement(const typeQDomElement &element, typeQTreeWidgetItem *parent)
{
    typeQTreeWidgetItem *item = new typeQTreeWidgetItem(parent);
    item->setText(0,element.attribute(XML_ATTRIBUTE_MANDATORY));

    m_strLinuxAppMandatory = element.attribute(XML_ATTRIBUTE_MANDATORY);
    m_strLinuxApp= element.text();
    if(m_strLinuxAppMandatory == "yes")
    {
     CompareList << element.tagName();
    }
    xmlData.key = element.tagName();
    xmlData.value = element.text();
    m_XMLMap.push_back(xmlData);
}

void DomParser::ParserLinuxSysElement(const typeQDomElement &element, typeQTreeWidgetItem *parent)
{
    typeQTreeWidgetItem *item = new typeQTreeWidgetItem(parent);
    item->setText(0,element.attribute(XML_ATTRIBUTE_MANDATORY));

    m_strLiunxSysMandatory = element.attribute(XML_ATTRIBUTE_MANDATORY);
    m_strLinuxSys= element.text();
    if(m_strLiunxSysMandatory == "yes")
    {
     CompareList << element.tagName();
    }
    xmlData.key = element.tagName();
    xmlData.value = element.text();
    m_XMLMap.push_back(xmlData);
}
