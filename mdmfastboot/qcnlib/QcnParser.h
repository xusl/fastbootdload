#ifndef __CHANGE_QCN_H
#define __CHANGE_QCN_H

#include <vector>

using namespace std;
typedef unsigned char      uint8;

//add by jie.li 2012-02-21 for LTE NV>20000
#define rest_of_stream		128
typedef struct _EFS_Dir_Data{
	USHORT	mask;						//Attribute mask
	BYTE	bufferOpr;					//Buffering option
	BYTE	cleanOpr;					//Cleanup option
	DWORD	date;						//Creation date
	char	stream[rest_of_stream];		//Unterminated filename for this sequence number
}EFS_Dir_Data;

typedef struct _EFS_Backups{
	CString	  fileName; 			//file name
	DWORD	  dataLens;				//EFS_Data lens
	char*	  NVdata;				//the data of NV
}EFS_Backups;

typedef struct _EFS_Dir{
	CString	  fileName; 			//file name
	DWORD	  dirLens;				//EFS_Dir lens
	CString	  pathFileName;;
}EFS_Dir;

typedef struct _EFS_Data{
	CString	  fileName; 			//file name
	DWORD	  dataLens;				//EFS_Data lens
	char*	  NVdata;
}EFS_Data;
//end add

class QcnParser
{
public:
	QcnParser();
	~QcnParser();
private:
	uint8* pdata;
	DWORD  m_readLens;

	//add by jie.li 2012-02-21 for LTE NV>20000
	CString strName1;
	bool bBackup;
	bool bEfsDir;
	bool bEfsData;
	bool bProvisioning;
	char* NVdata;
	DWORD  m_NVreadLens;
	std::vector<EFS_Dir>		vecEFS_Dir;
	std::vector<EFS_Data>		vecEFS_Data;
	//end add
	
public:
	/*********************************************************
	OpenDocument:
	This function opens the compound file and passes a 
	pointer to the root IStorage interface to IterateStorage. 
	The function releases the IStorage object by setting 
	the smart pointer spRoot to NULL before exiting.
	**********************************************************/
    uint8* OpenDocument(LPCTSTR pDocName, DWORD* lens);

	//add by jie.li 2012-02-21 for LTE NV>20000
	std::vector<EFS_Backups>	vecEFS_Backup;	
	//end add

private:
	/*********************************************************
	IterateStorage
	This function displays the name of the storage pointed 
	to by spStorage and calls EnumBranch to enumerate the 
	storage¡¯s elements.
	**********************************************************/
    void IterateStorage(IStorage* spStorage, int indentCount, bool binDump);

	/*********************************************************
	EnumBranch
	EnumBranch enumerates all the elements for which spStorage
	is a parent. It calls ExamineBranch for each element.
	**********************************************************/
	void EnumBranch(IStorage* spStorage, int indentCount, bool binDump);

	/*********************************************************
	ExamineBranch
	This function checks the element type and takes appropriate
	action to display its contents. If the element is a stream, 
	the function prints the stream name and size, and calls 
	DumpStreamContents if the ¨Cd command line option was set.
	For a storage, the function calls ExamineStorage.
	**********************************************************/
	void ExamineBranch(IStorage* spStorage, int indentCount,STATSTG& stat, bool binDump);

	/********************************************************
	ExamineStorage
	ExamineStorage takes a pointer to a parent storage object 
	(spStorage) and the name of a child storage object 
	(stat.pwcsName), opens the child storage, and recursively 
	calls IterateStorage for the child object. IterateStorage 
	then displays the name of the storage and calls EnumBranch
	to enumerate the storage¡¯s elements. 	This can lead to
	EnumBranch calling ExamineBranch, and finally calling 
	ExamineStorage again.
	*********************************************************/
	void ExamineStorage(IStorage* spStorage, int indentCount,STATSTG& stat, bool binDump);

	/********************************************************
	DumpStreamContents
	This function prints the contents of a stream in hexadecimal.
	*********************************************************/
	void  DumpStreamContents(IStorage* spStorage, int indentCount,LPWSTR pStreamName,DWORD readSize);

	//add by jie.li for LTE NV>20000
	void  DumpStreamEfsBackupContents(IStorage* spStorage, int indentCount,LPWSTR pStreamName,DWORD readSize);
	void  MergeEFSBackup();
	//end add

};

#endif  //CHANGE_QCN_H