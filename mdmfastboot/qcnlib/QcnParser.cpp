#include "StdAfx.h"
#include <string>
#include <iostream>
#include <afxwin.h>
#include <Objidl.h>
#include <comutil.h>
#include "QcnParser.h"
//#include "memmgr.h"
#include "log.h"
#include "..\utils.h"


QcnParser::QcnParser() {
  m_NVNumberedItems = 0;
  m_NVItemArray = NULL;
  m_StatStorageName = STAT_UNKNOWN;
}

QcnParser::~QcnParser()
{
  free(m_NVItemArray);
}

uint8* QcnParser::OpenDocument(LPCWSTR pDocName, DWORD* lens) {
  IStorage* spRoot = NULL;
  HRESULT hr;

  if (pDocName == NULL || lens == NULL) {
    ERROR("Invalid parameter.");
    return NULL;
  }

  hr= StgOpenStorage(pDocName,
                    NULL,
                    STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE,
                    NULL,
                    0,
                    &spRoot);

  if (FAILED(hr)) {
    ERROR( "Failed to open <%S>, hr = 0x%08X", pDocName, hr);
    return NULL;
  }

  IterateStorage(spRoot, 0);
  MergeEFSBackup();

  spRoot->Release();

 // *lens = m_readLens;
  return NULL;
}

void QcnParser::IterateStorage(IStorage* spStorage, int depth) {
  STATSTG stat;
  IEnumSTATSTG* spEnum=NULL;
  HRESULT hr = spStorage->Stat(&stat, STATFLAG_DEFAULT);

  if (FAILED(hr)){
    INFO("Failed to Stat storage, hr = 0x%08X", hr);
    return;
  }

  //INFO("stg <%ls>", stat.pwcsName);
  CoTaskMemFree(stat.pwcsName);

  hr = spStorage->EnumElements(0, NULL, 0, &spEnum);

  if (FAILED(hr)) {
    ERROR("Failed to EnumElements, hr = 0x%08X", hr);
    return;
  }

  while (spEnum->Next(1, &stat, NULL) == S_OK) {
    ExamineBranch(spStorage, stat, depth);
    // pls refer STATSTG Structure in MSDN.
    //pwcsName
    // A pointer to a NULL-terminated Unicode string that contains the name.
    //Space for this string is allocated by the method called and freed by the caller
    //(for more information, see CoTaskMemFree). To not return this member,
    //specify the STATFLAG_NONAME value when you call a method that returns a
    //STATSTG structure, except for calls to IEnumSTATSTG::Next, which provides
    //no way to specify this value.
    CoTaskMemFree(stat.pwcsName);
  }

  spEnum->Release();
}

void QcnParser::ExamineBranch(IStorage* spStorage, STATSTG& stat, int depth)
{
  LPOLESTR ItemName = stat.pwcsName;
  DWORD size = stat.cbSize.LowPart; /* equal to  stat.cbSize.LowPart */
  DEBUG("Depth %d, pItemName:%S, TYPE : %d, size :%d",depth,  ItemName, stat.type, size);

  switch(stat.type) {
  case STGTY_STORAGE:
    {
      IStorage* spRoot=NULL;
      HRESULT hr;

      hr= spStorage->OpenStorage(stat.pwcsName,
                                 NULL,
                                 STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE,
                                 NULL,
                                 0,
                                 &spRoot);

      if (FAILED(hr)) {
        ERROR("Failed to OpenStorage <%ls>, hr = 0x%08X.", stat.pwcsName, hr);
        return;
      }

      // m_StatStorageName = STAT_UNKNOWN;
      if (depth == 2) {
        if (wcscmp(ItemName, L"NV_Items") == 0) {
          m_StatStorageName |= STAT_NV_Items;
        } else if (wcscmp(ItemName, L"EFS_Backup") == 0) {
          m_StatStorageName |= STAT_EFS_Backup;
        //} else if (wcscmp(ItemName, L"Provisioning_Item_Files") == 0) {
        //  m_StatStorageName |= STAT_Provisioning_Item_Files;
        } else if (wcscmp(ItemName, L"NV_NUMBERED_ITEMS") == 0) {
          m_StatStorageName |= STAT_NV_NUMBERED_ITEMS;
        } else {
          //m_StatStorageName &= ~STAT_LEVEL_2_MASK;
          spRoot ->Release();
          break;
        }
      } else if (depth == 3) {
        if (wcscmp(ItemName, L"EFS_Dir") == 0) {
          m_StatStorageName |= STAT_EFS_Dir;
        } else if (wcscmp(ItemName, L"EFS_Data") == 0) {
          m_StatStorageName |= STAT_EFS_Data;
        } else {
          m_StatStorageName &= ~STAT_LEVEL_3_MASK;
        }
      }

      IterateStorage(spRoot, depth + 1);
      spRoot ->Release();
      if (depth == 2 ) {
        m_StatStorageName &= ~STAT_LEVEL_2_MASK;
      } else if (depth == 3){
        m_StatStorageName &= ~STAT_LEVEL_3_MASK;
      }
    }
    break ;

  case STGTY_STREAM:
    //DEBUG("*****strName = %S stat.cbSize = %d", ItemName, size);
    if (m_StatStorageName & STAT_EFS_Dir) {
      //EFS_Dir_Data *efsDirData ;
      char *path;
      EFS_Backups bknv;
      map<int, EFS_Backups> :: iterator iter;
      int item_index = wcstol(ItemName, NULL, 16);
      /*size do not contain the NULL termination char*/
      char*data = (char *)calloc(1, size + 1);
      if (!data) {
        ERROR("out of memory");
        break;
      }
      //ZeroMemory(&efsDirData, sizeof(efsDirData));
      DumpStreamContents(spStorage, ItemName, size, data);
      if (m_StatStorageName & STAT_EFS_Backup) {
        if (sizeof(EFS_Dir_Data) < size) {
          ERROR("Size %d exceed EFS_Dir_Data size", size);
          free(data);
          break;
        }
        path = ((EFS_Dir_Data *)data)->stream;
        //memcpy(&efsDirData ,data, size);
      } else {
        if (rest_of_stream < size) {
          size = rest_of_stream;
          WARN("size %d exceed rest_of_stream %d",size, rest_of_stream);
        }
        //strncpy(efsDirData.stream, data, size);
        path = data;
      }

      if (strncmp(path, "/nv/", strlen("/nv/")) == 0) {
        iter = m_BackupNV.find(item_index);
        if (iter == m_BackupNV.end()) {
          bknv.fileName = path;
          m_BackupNV.insert(BackupNV(item_index, bknv));
        } else {
          iter->second.fileName = path;
        }
      } else {
        WARN("%s NOT START WITH /nv/", path);
      }
      free(data);
    }else if (m_StatStorageName & STAT_EFS_Data) { //read file contents
      char* data = (char *)calloc(1, size);
      int item_index = wcstol(ItemName, NULL, 16);
      map<int, EFS_Backups> :: iterator iter;
      EFS_Backups bknv;

      if (data == NULL) {
        ERROR("out of memory");
        return;
      }
      DumpStreamContents(spStorage, ItemName, size, data);

      iter = m_BackupNV.find(item_index);
      if (iter == m_BackupNV.end()) {
        bknv.NVdata = data;
        bknv.dataLens = size;
        m_BackupNV.insert(BackupNV(item_index, bknv));
      } else {
        iter->second.NVdata = data;
        iter->second.dataLens = size;
      }
    }else if (m_StatStorageName & STAT_NV_NUMBERED_ITEMS
              && (wcscmp(ItemName, L"NV_ITEM_ARRAY") == 0)) {
      m_NVNumberedItems = size / sizeof(NV_ITEM_PACKET_INFO);
      if (m_NVNumberedItems * sizeof(NV_ITEM_PACKET_INFO) != size) {
        ERROR("ERROR SIZE %d", size);
        break;
      }
      if (m_NVItemArray != NULL) {
        WARN("FREE m_NVItemArray");
        free(m_NVItemArray);
      }

      m_NVItemArray = (NV_ITEM_PACKET_INFO *)calloc(1, size);
      if (m_NVItemArray == NULL) {
        ERROR("out of memory.");
        break;
      }
      DumpStreamContents(spStorage, ItemName, size, m_NVItemArray);
    } else {
      WARN("Do not handle stat %d, item %S", m_StatStorageName, ItemName);
    }
    break;

  case STGTY_LOCKBYTES:
    // ILockBytes
    break;

  case STGTY_PROPERTY:
    //    IEnumSTATSTG,
    break;
  }
}

BOOL QcnParser::DumpStreamContents(IStorage* spStorage,
								   LPWSTR pStreamName,DWORD buffer_len, PVOID buffer)
{
  IStream* spData=NULL;
  ULONG streamRead = 0;
  BOOL result = FALSE;
  HRESULT hr;

  if (buffer == NULL) {
    ERROR("Invalid paramater.");
    return result;
  }

  hr = spStorage->OpenStream(pStreamName,
                             NULL,
                             STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE,
                             0,
                             &spData);
  if (FAILED(hr)) {
    ERROR("Failed to OpenStream <%ls>, hr = 0x%08X.", pStreamName, hr);
    return result;
  }

  if (spData->Read(buffer, buffer_len, &streamRead) == S_OK) {
    if (streamRead == buffer_len) {
      result = TRUE;
    } else {
      ERROR("Read QCN length incorrect!");
    }
  }
  spData->Release();
  return result;
}

void QcnParser::MergeEFSBackup() {
  map<int, EFS_Backups> :: const_iterator iter;
  for ( iter = m_BackupNV.begin( ) ; iter != m_BackupNV.end( ) ; iter++ ){
    DEBUG("ITEM %X, PATH %S, datalen %d",iter->first, iter->second.fileName, iter->second.dataLens);
  }

}
