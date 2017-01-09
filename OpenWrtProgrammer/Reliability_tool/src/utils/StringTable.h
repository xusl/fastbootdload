//
//////////////////////////////////////////////////////////////////////

#ifndef STIRNGSADE_TABLE
#define STIRNGSADE_TABLE

#include "../define/stdafx.h"

typedef enum {
    EN = 0,
    GR,
    DU,
    PL,
    CZ,
    SL,
    HU,
    CR,
    MA,
    TR,
    MO,
    BU,
    GE,
    RO,
    AL
}LanguageType;

class typeQStringTable
{
public:
        static typeQStringTable& Instance();
        ~typeQStringTable();

        //bool GetString(int id, typeQString& str)const;
        bool GetString(QString strLan, int id, QString& str)const;
        bool ParseTxtString(QChar* txtDataBuf, QString strPathLan);
        QString m_strLan;
        void setLanguage(QString strLan);
        void GetLanguage(QMap<int, QString> &LanMap);
        void GetLanguageKind(QMap<int, QString> &LanMap);
private:
        typeQStringTable();
	
private:
        typeQStringTable& operator=(const typeQStringTable&);
        typeQStringTable(const typeQStringTable&);
private:
        QMap<int, QString> m_Map;

        typedef QMap<int, QString>::iterator iterator;
        typedef QMap<int, QString>::const_iterator const_iterator;
        QMap<int, QString> m_LanMap;
        QMap<int, QString> m_LanguageMap;
};

#endif
