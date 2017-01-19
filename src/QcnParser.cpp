#include "StdAfx.h"
#include <string>
#include <iostream>
#include <afxwin.h>
#include <Objidl.h>
#include <comutil.h>
#include "QcnParser.h"
#include "log.h"
#include "utils.h"


QcnParser::QcnParser() {
  m_NVNumberedItems = 0;
  m_NVItemArray = NULL;
  m_StorageStat = STAT_UNKNOWN;
}

QcnParser::~QcnParser()
{
  Clear();
}

BOOL QcnParser::OpenDocument(LPCWSTR pDocName) {
  IStorage* spRoot = NULL;
  HRESULT hr;

  if (pDocName == NULL ) {
    ERROR("Invalid parameter.");
    return FALSE;
  }

  hr= StgOpenStorage(pDocName,
                    NULL,
                    STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE,
                    NULL,
                    0,
                    &spRoot);

  if (FAILED(hr)) {
    ERROR( "Failed to open <%S>, hr = 0x%08X", pDocName, hr);
    return FALSE;
  }

  Clear();

  IterateStorage(spRoot, 0);
  //MergeEFSBackup();

  spRoot->Release();

  return TRUE;
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
  //DEBUG("Depth %d, pItemName:%S, TYPE : %d, size :%d",depth,  ItemName, stat.type, size);

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

      if (depth == 2) {
        if (wcscmp(ItemName, L"NV_Items") == 0) {
          m_StorageStat |= STAT_NV_Items;
        } else if (wcscmp(ItemName, L"EFS_Backup") == 0) {
          m_StorageStat |= STAT_EFS_Backup;
        //} else if (wcscmp(ItemName, L"Provisioning_Item_Files") == 0) {
        //  m_StorageStat |= STAT_Provisioning_Item_Files;
        } else if (wcscmp(ItemName, L"NV_NUMBERED_ITEMS") == 0) {
          m_StorageStat |= STAT_NV_NUMBERED_ITEMS;
        } else {
          //m_StorageStat &= ~STAT_LEVEL_2_MASK;
          spRoot ->Release();
          break;
        }
      } else if (depth == 3) {
        if (wcscmp(ItemName, L"EFS_Dir") == 0) {
          m_StorageStat |= STAT_EFS_Dir;
        } else if (wcscmp(ItemName, L"EFS_Data") == 0) {
          m_StorageStat |= STAT_EFS_Data;
        } else {
          m_StorageStat &= ~STAT_LEVEL_3_MASK;
        }
      }

      IterateStorage(spRoot, depth + 1);
      spRoot ->Release();
      if (depth == 2 ) {
        m_StorageStat &= ~STAT_LEVEL_2_MASK;
      } else if (depth == 3){
        m_StorageStat &= ~STAT_LEVEL_3_MASK;
      }
    }
    break ;

  case STGTY_STREAM:
    if (m_StorageStat & STAT_EFS_Dir) {
      char *path;
      /*size do not contain the NULL termination char*/
      char*data = (char *)calloc(1, size + 1);
      if (!data) {
        ERROR("out of memory");
        break;
      }
      DumpStreamContents(spStorage, ItemName, size, data);
      if (m_StorageStat & STAT_EFS_Backup) {
        if (sizeof(EFS_Dir_Data) < size) {
          ERROR("Size %d exceed EFS_Dir_Data size", size);
          free(data);
          break;
        }
        path = ((EFS_Dir_Data *)data)->stream;
      } else {
        if (rest_of_stream < size) {
          size = rest_of_stream;
          WARN("size %d exceed rest_of_stream %d",size, rest_of_stream);
        }
        path = data;
      }

      if (((strncmp(path, "/nv/", 4) == 0) && (m_StorageStat & STAT_NV_Items))
        ||( m_StorageStat & STAT_EFS_Backup)) {
        UpdateEFSNV(ItemName, path, NULL, 0);
      } else {
        WARN("%s NOT START WITH /nv/", path);
      }
      free(data);
    }else if (m_StorageStat & STAT_EFS_Data) { //read file contents
      char* data = (char *)calloc(1, size);

      if (data == NULL) {
        ERROR("out of memory");
        return;
      }
      DumpStreamContents(spStorage, ItemName, size, data);
      UpdateEFSNV(ItemName, NULL, data, size);
    }else if (m_StorageStat & STAT_NV_NUMBERED_ITEMS
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
      WARN("Do not handle stat %d, item %S", m_StorageStat, ItemName);
    }
    break;

  case STGTY_LOCKBYTES:
    // ILockBytes
    break;

  case STGTY_PROPERTY:
    // IEnumSTATSTG,
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

BOOL QcnParser::UpdateEFSNV(LPOLESTR item, PCCH path, PCHAR data, int size) {
  int item_index;
  map<int, EFS_NV> :: iterator iter;
  EFS_NV bknv;

  if (item == NULL) {
    return FALSE;
  }

  item_index = wcstol(item, NULL, 16);

  iter = m_EFSNVs.find(item_index);
  if (iter == m_EFSNVs.end()) {
    bknv.NVdata = data;
    bknv.dataLens = size;
    bknv.fileName = (path != NULL ? path : "");
    m_EFSNVs.insert(INT_EFSNV_PAIR(item_index, bknv));
  } else {
    if (NULL != data && size > 0) {
      iter->second.NVdata = data;
      iter->second.dataLens = size;
    }
    if (path != NULL) {
      iter->second.fileName = path;
    }
  }
  return TRUE;
}

BOOL QcnParser::Clear(void) {
  map<int, EFS_NV> :: const_iterator iter;
  for ( iter = m_EFSNVs.begin( ) ; iter != m_EFSNVs.end( ) ; iter++ ){
    if (iter->second.NVdata) {
      free(iter->second.NVdata);
    }
  }
  m_EFSNVs.clear();

  if (m_NVItemArray != NULL) {
    free(m_NVItemArray);
    m_NVItemArray = NULL;
  }
  return TRUE;
}

#define HEX_UP(val) (((val) > 9) ? ((val) + 0x40 - 9): ((val) + 0x30))
#define HEX_LOW(val) (((val) > 9) ? ((val) + 0x60 - 9): ((val) + 0x30))
BOOL QcnParser::GetNVWriteCommands(char *nv_cmd, char *** cmds_ptr,unsigned int *nr_ptr) {
  map <int, EFS_NV> :: const_iterator iter;
  NV_ITEM_PACKET_INFO *nv = m_NVItemArray;
  unsigned int cmd_len;
  unsigned int nv_cmd_len;
  unsigned int size;
  unsigned int used;
  unsigned int offset;
  char ** cmds;
  if (nv_cmd == NULL || cmds_ptr == NULL || nr_ptr == NULL) {
    ERROR("Bad parameters!");
    return FALSE;
  }

  size = m_NVNumberedItems + m_EFSNVs.size();
  cmds = (char **)calloc(sizeof(char*), size);
  if (cmds == NULL) {
    ERROR("Out of memory");
    return FALSE;
  }

  nv_cmd_len = strlen(nv_cmd);

  for (size = 0; size < m_NVNumberedItems && nv != NULL; size++, nv++) {
    cmd_len = nv_cmd_len + 12 + sizeof(nv->itemData) * 2;
    *(cmds + size) = (char*)calloc(sizeof(char), cmd_len);
    if (*(cmds + size) == NULL) {
      ERROR("Out of memory");
      break;
    }
    used = _snprintf(*(cmds + size), cmd_len,"%s %8d ", nv_cmd, nv->nvItemID);

    for (offset = 0;offset < sizeof(nv->itemData); used++, offset++) {
      if (used > cmd_len - 3) {
        ERROR("Buffer length is shorter than data.");
        break;
      }
    *(*(cmds + size) + used) = HEX_UP(((*(nv->itemData + offset))>> 4) & 0x0f);
    used++;
    *(*(cmds + size) + used) = HEX_UP((*(nv->itemData + offset)) & 0x0f);
    }
    *(*(cmds + size) + used) = '\0';

    //DEBUG("nvItemID %d, nvItemIndex %d, packetLen %d", nv->nvItemID, nv->nvItemIndex, nv->packetLen);
    //DEBUG("COMMMAND IS %s", *(cmds + size));
  }

  for ( iter = m_EFSNVs.begin( ) ; iter != m_EFSNVs.end( ) ; iter++, size++){
    cmd_len = nv_cmd_len + wcslen(iter->second.fileName.GetString()) + 4;
   cmd_len += iter->second.dataLens * 2;
    *(cmds + size) = (char*)calloc(sizeof(char), cmd_len);
    if (*(cmds + size) == NULL) {
      ERROR("Out of memory");
      break;
    }
    used = _snprintf(*(cmds + size), cmd_len,"%s %S ", nv_cmd, iter->second.fileName.GetString());

    for (offset = 0;offset < iter->second.dataLens; offset++,used++) {
      if (used > cmd_len - 3) {
        ERROR("Buffer length is shorter than data.");
        break;
      }
    *(*(cmds + size) + used) = HEX_UP((*(iter->second.NVdata + offset)>> 4) & 0x0f);
    used++;
    *(*(cmds + size) + used) = HEX_UP(*(iter->second.NVdata + offset) & 0x0f);
    }
    *(*(cmds + size) + used) = '\0';

     //DEBUG("ITEM %08X, PATH %S, datalen %d",iter->first, iter->second.fileName, iter->second.dataLens);
     //DEBUG("COMMMAND IS %s", *(cmds + size));
  }

  *nr_ptr = size;
  *cmds_ptr = cmds;

  return TRUE;
}

BOOL QcnParser::PutNVWriteCommands(char **cmds,unsigned int nr) {
  unsigned int i = 0;
  char* cmd;
  if (cmds == NULL || nr == NULL) {
    ERROR("Bad parameters!");
    return FALSE;
  }

  for ( i = 0;i < nr; i++) {
    cmd = *(cmds +i);
    if (cmd != NULL) {
      free(cmd);
      *cmd = NULL;
    }
  }

  free(cmds);

  return TRUE;
}
