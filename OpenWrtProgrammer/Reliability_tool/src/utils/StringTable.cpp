
#include "StringTable.h"
#include <fstream>


typeQStringTable& typeQStringTable::Instance()
{
    static typeQStringTable s_Instance;
    return s_Instance;
}

typeQStringTable::~typeQStringTable()
{

}

bool typeQStringTable::GetString(typeQString strLan, int id, typeQString& str) const
{
    int sizeMap = m_LanMap.size();
    for(int i=1; i<=sizeMap; i++)
    {
        QString str = m_LanMap[i];
        if(str == strLan)
        {
            id += i*100;
            break; //add by jie.li 2012-8-22
        }
    }

    const_iterator it = m_Map.find(id);
    if (it != m_Map.end())
    {
        str = it.value();
        return true;
    }

    return false;
}
//#endif

typeQStringTable::typeQStringTable()
{

}

bool typeQStringTable::ParseTxtString(typeQChar* txtDataBuf, typeQString strPathLan)
{
    if (txtDataBuf != NULL)
    {
        typeQChar txtBuf[512] = {0};
        int i=0;
        int nID = 0;
        int iLan = 1;
        QString strLan = "";

        while (txtDataBuf[0] != '\0')
        {
            if (txtDataBuf[0] == '\r')
            {
                typeQString strTxtTemp(txtBuf, 512);
                //int iPos = strTxtTemp.Find("###");
                int iPos = strTxtTemp.indexOf("###",0);
                if (iPos > 0)
                {
                    i = 0;
                    typeQString strNum = strTxtTemp.left(iPos);
                    nID = strNum.toInt();                   
                    int iLan1 = nID % 100;

                    typeQString strValue = strTxtTemp.right(strTxtTemp.size()-iPos-3);
                    char strValueEnd = '\0';
                    int iPos2 = strValue.indexOf(strValueEnd,0);
                    strLan = strValue.mid(0, iPos2);
                    iPos2 = strLan.indexOf("=", 0);
                    QString strLanEnable = strLan.right(strLan.size() - iPos2 -1);
                    QString strDefualt = strLanEnable;                 
                    strLanEnable.toUpper();
                    bool lanEnable;
                    if(strLanEnable.contains("YES", Qt::CaseInsensitive))   //judge whether show the language or not
                        lanEnable = true;
                    else
                        lanEnable = false;
                    strLan = strLan.left(iPos2);
                    if(0 == iLan1) //100整除
                    {
                        int iLan2 = nID/100;  //根据结果确定是哪种语言
                        if (0 == iLan2) //set default language
                        {
                            if (strPathLan == "")
                                m_strLan = strDefualt;
                            else
                                m_strLan = strPathLan;
                        }
                        if (1 == iLan2)
                        {
                            if (lanEnable)
                                m_LanMap[1] = strLan;
                            else
                                m_LanMap[1] = "Unable";    //Unable is just a sign
                        }
                        if (2 == iLan2)
                        {
                            if (lanEnable)
                                m_LanMap[2] = strLan;
                            else
                                m_LanMap[2] = "Unable";
                        }
                        if (3 == iLan2)
                        {
                            if (lanEnable)
                                m_LanMap[3] = strLan;
                            else
                                m_LanMap[3] = "Unable";
                        }
                        if (4 == iLan2)
                        {
                            if (lanEnable)
                                m_LanMap[4] = strLan;
                            else
                                m_LanMap[4] = "Unable";
                        }
                        if (5 == iLan2)
                        {
                            if (lanEnable)
                                m_LanMap[5] = strLan;
                            else
                                m_LanMap[5] = "Unable";
                        }
                        if (6 == iLan2)
                        {
                            if (lanEnable)
                                m_LanMap[6] = strLan;
                            else
                                m_LanMap[6] = "Unable";
                        }
                        if (7 == iLan2)
                        {
                            if (lanEnable)
                                m_LanMap[7] = strLan;
                            else
                                m_LanMap[7] = "Unable";
                        }
                        if (8 == iLan2)
                        {
                            if (lanEnable)
                                m_LanMap[8] = strLan;
                            else
                                m_LanMap[8] = "Unable";
                        }
                        if (9 == iLan2)
                        {
                            if (lanEnable)
                                m_LanMap[9] = strLan;
                            else
                                m_LanMap[9] = "Unable";
                        }
                        if (10 == iLan2)
                        {
                            if (lanEnable)
                                m_LanMap[10] = strLan;
                            else
                                m_LanMap[10] = "Unable";
                        }
                        if (11 == iLan2)
                        {
                            if (lanEnable)
                                m_LanMap[11] = strLan;
                            else
                                m_LanMap[11] = "Unable";
                        }
                        if (12 == iLan2)
                        {
                            if (lanEnable)
                                m_LanMap[12] = strLan;
                            else
                                m_LanMap[12] = "Unable";
                        }
                        if (13 == iLan2)
                        {
                            if (lanEnable)
                                m_LanMap[13] = strLan;
                            else
                                m_LanMap[13] = "Unable";
                        }
                        if (14 == iLan2)
                        {
                            if (lanEnable)
                                m_LanMap[14] = strLan;
                            else
                                m_LanMap[14] = "Unable";
                        }
                        if (15 == iLan2)
                        {
                            if (lanEnable)
                                m_LanMap[15] = strLan;
                            else
                                m_LanMap[15] = "Unable";
                        }
                    }
                    typeQString strV = strTxtTemp.right(strTxtTemp.size()-iPos-3);
                    //m_Map[nID] = strV;
                    char strEnd = '\0';
                    iPos = strV.indexOf(strEnd,0);
                    m_Map[nID] = strV.mid(0, iPos);

                    if (nID%100 == 72)
                    {
                        m_LanguageMap[iLan] = m_Map[nID];
                        iLan++;
                    }

                    memset(txtBuf, 0, 512);                                     
                }
            }

            if (txtDataBuf[0] == '\\' && txtDataBuf[1] == 'n')
            {
                 txtBuf[i] = '\n';
                 i++;
                 txtDataBuf++;
                 txtDataBuf++;
                 continue;
            }

            txtBuf[i] = txtDataBuf[0];
            i++;
            txtDataBuf++;
        }
        return true;
    }
    return false;
}
void typeQStringTable::setLanguage(typeQString strLan)
{
    m_strLan = strLan;
}

void typeQStringTable::GetLanguage(typeQMap<int, typeQString> &LanMap)
{
    LanMap = m_LanMap;
}

void typeQStringTable::GetLanguageKind(typeQMap<int, typeQString> &LanMap)
{
    LanMap = m_LanguageMap;
}
