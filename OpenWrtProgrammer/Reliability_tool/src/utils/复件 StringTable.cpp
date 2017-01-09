//#include "../define/stdafx.h"
#include "StringTable.h"
#include <fstream>


QStringTable& QStringTable::Instance()
{
    static QStringTable s_Instance;
    return s_Instance;
}

QStringTable::~QStringTable()
{

}

#ifndef T_MOBILE
bool QStringTable::GetString(int id, QString& str) const
{
    const_iterator it = m_Map.find(id);
    if (it != m_Map.end())
    {
        str = it.value();
        return true;
    }
    return false;
}
#else
bool QStringTable::GetString(QString strLan, int id, QString& str) const
{
    LanguageType    m_lan;
    if("ENGLISH" == strLan.toUpper())
    {
        m_lan = EN;
    }
    else if("GERMAN" == strLan.toUpper())
    {
        m_lan = GR;
    }
    else if("DUTCH" == strLan.toUpper())
    {
        m_lan = DU;
    }
    else if("POLISH" == strLan.toUpper())
    {
        m_lan = PL;
    }
    else if("CZECH" == strLan.toUpper())
    {
        m_lan = CZ;
    }
    else if("SLOVAKIAN" == strLan.toUpper())
    {
        m_lan = SL;
    }
    else if("HUNGARIAN" == strLan.toUpper())
    {
        m_lan = HU;
    }
    else if("CROATIAN" == strLan.toUpper())
    {
        m_lan = CR;
    }
    else if("MACEDONIAN" == strLan.toUpper())
    {
        m_lan = MA;
    }
    else if("TURKISH" == strLan.toUpper())
    {
        m_lan = TR;
    }
    else if("MONTENEGRIN" == strLan.toUpper())
    {
        m_lan = MO;
    }
    else if("BULGARIAN" == strLan.toUpper())
    {
        m_lan = BU;
    }
    else if("GREEK" == strLan.toUpper())
    {
        m_lan = GE;
    }
    else if("ROMANIAN" == strLan.toUpper())
    {
        m_lan = RO;
    }
    else if("ALBANIAN" == strLan.toUpper())
    {
        m_lan = AL;
    }
    else
    {
        m_lan = EN;
    }

    switch(m_lan)
    {
    case EN:
        id += 100;
        break;
    case GR:
        id += 200;
        break;
    case DU:
        id += 300;
        break;
    case PL:
        id += 400;
        break;
    case CZ:
        id += 500;
        break;
    case SL:
        id += 600;
        break;
    case HU:
        id += 700;
        break;
    case CR:
        id += 800;
        break;
    case MA:
        id += 900;
        break;
    case TR:
        id += 1000;
        break;
    case MO:
        id += 1100;
        break;
    case BU:
        id += 1200;
        break;
    case GE:
        id += 1300;
        break;
    case RO:
        id += 1400;
        break;
    case AL:
        id += 1500;
        break;
    default:
        id += 100;
        break;
    }

    const_iterator it = m_Map.find(id);
    if (it != m_Map.end())
    {
        str = it.value();
        return true;
    }

    return false;
}
#endif

QStringTable::QStringTable()
{

}

#ifndef T_MOBILE
bool QStringTable::ParseTxtString(QChar* txtDataBuf)
{	
	if (txtDataBuf != NULL)
    {
        QChar txtBuf[512] = {0};
        int i=0;
        int nID = 0;

        while (txtDataBuf[0] != '\0')
        {
            if (txtDataBuf[0] == '\r')
            {
                QString strTxtTemp(txtBuf, 512);
                //int iPos = strTxtTemp.Find("###");
                int iPos = strTxtTemp.indexOf("###",0);
                if (iPos > 0)
                {
                    i = 0;
                    QString strNum = strTxtTemp.left(iPos);
                    nID = strNum.toInt();
                    QString strV = strTxtTemp.right(strTxtTemp.size()-iPos-3);
                    m_Map[nID] = strV;
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
#else
bool QStringTable::ParseTxtString(QChar* txtDataBuf, QString strLan)
{	
    m_strLan = strLan;
    if (txtDataBuf != NULL)
    {
        QChar txtBuf[512] = {0};
        int i=0;
        int nID = -1;

        while (txtDataBuf[0] != '\0')
        {
            if (txtDataBuf[0] == '\r')
            {
                QString strTxtTemp(txtBuf, 512);
                //int iPos = strTxtTemp.Find("###");
                int iPos = strTxtTemp.indexOf("###",0);
                if (iPos > 0)
                {
                    i = 0;
                    QString strNum = strTxtTemp.left(iPos);
                    nID = strNum.toInt();
                    QString strV = strTxtTemp.right(strTxtTemp.size()-iPos-3);
                    char strEnd = '\0';
                    iPos = strV.indexOf(strEnd,0);
                    m_Map[nID] = strV.mid(0, iPos);
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
#endif
