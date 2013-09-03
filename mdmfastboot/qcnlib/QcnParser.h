#ifndef __CHANGE_QCN_H
#define __CHANGE_QCN_H

#include <vector>
#include <map>

using namespace std;
typedef unsigned char      uint8;

/*
 *############## .qcn item hierarchy#############
 * Generally, there are four level from root item to the
 * leaf data item.
 * Level 0: two item, one is numbered folder, such as "0000 0000"
 *          or "0000 0008". the other is "File_Version", it has only
 *          data, do not contain any folder.
 * Level 1: only a "default" folder for numberd folder "0000 0000".
 * Level 2: there are more items in this level.
 *                (1)"NV_Items"
 *                (2)"EFS_Backup"
 *                (3)"Provisioning_Item_Files"
 *                (4)"NV_NUMBERED_ITEMS"
 *                (5)"Feature_Mask"
 *                (6)"Mobile_Property_Info"
 *                (1),(2),(3) have subfolder "EFS_Dir" and "EFS_Data"
 *                seperately.
 *                (5),(6) just have data.
 *                (4) have a subfolder "NV_ITEM_ARRAY". This folder contain
 *                normal NV item, and (1) store extend NV item which store NV
 *                data in EFS files.
 * Level 3: "EFS_Dir", "EFS_Data", "NV_ITEM_ARRAY"
 * Items in "EFS_Dir" folder contains the EFS file path, and items in
 * "EFS_Data" contains the EFS file data, the item combine in the two
 * folder with index, that is with the index, item in one folder can match
 * item in the other folder. The data of item in "EFS_DIR" in "NV_Items" begin
 * with "/nv".
 * "NV_ITEM_ARRAY" contain all NV items that numbered less than 20000, all item
 * have the same size, just like array, so "NV_ITEM_ARRAY" store they in its
 * data directly, it contains none folder.
*/


enum {
  STAT_UNKNOWN = 0x00000000,
  STAT_GENERAL = 0x80000000,
  STAT_NV_Items = 0x00010000,
  STAT_EFS_Backup = 0x00020000,
  STAT_Provisioning_Item_Files = 0x00040000,
  STAT_NV_NUMBERED_ITEMS =   0x00080000,
  STAT_LEVEL_2_MASK = 0x00ff0000,
  STAT_EFS_Dir = 0x00000001,
  STAT_EFS_Data = 0x00000002,
  STAT_LEVEL_3_MASK = 0x0000ffff,
};

//NV item packet struct
typedef struct _NV_ITEM_PACKET_INFO{
	USHORT packetLen;			//Packet length
	USHORT packetReserve;		//reserve
	USHORT nvItemID;			//NV ID
	USHORT nvItemIndex;			//NV Index
	BYTE   itemData[128];		//NV data
}NV_ITEM_PACKET_INFO;


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

//end add


typedef pair <int, EFS_Backups> BackupNV;
class QcnParser
{
public:
	QcnParser();
	~QcnParser();
private:
  ULONG m_NVNumberedItems;
  NV_ITEM_PACKET_INFO *m_NVItemArray;
  UINT m_StatStorageName;
  map<int, EFS_Backups> m_BackupNV;

public:
	/*********************************************************
	OpenDocument:
	This function opens the compound file and passes a
	pointer to the root IStorage interface to IterateStorage.
	The function releases the IStorage object by setting
	the smart pointer spRoot to NULL before exiting.
	**********************************************************/
    uint8* OpenDocument(LPCWSTR pDocName, DWORD* lens);

private:
	/*********************************************************
	IterateStorage
	This function displays the name of the storage pointed
	to by spStorage and calls EnumBranch to enumerate the
	storage¡¯s elements.
	**********************************************************/
    void IterateStorage(IStorage* spStorage, int depth);


	/*********************************************************
	ExamineBranch
	This function checks the element type and takes appropriate
	action to display its contents. If the element is a stream,
	the function prints the stream name and size, and calls
	DumpStreamContents if the ¨Cd command line option was set.
	For a storage, the function calls ExamineStorage.
	**********************************************************/
	void ExamineBranch(IStorage* spStorage, STATSTG& stat, int depth);


	/********************************************************
	DumpStreamContents
	This function prints the contents of a stream in hexadecimal.
	*********************************************************/
	BOOL DumpStreamContents(IStorage* spStorage, LPWSTR pStreamName,DWORD   buffer_len, PVOID buffer);


	void  MergeEFSBackup();
};

#endif  //CHANGE_QCN_H
