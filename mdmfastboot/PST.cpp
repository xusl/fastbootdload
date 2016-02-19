#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>

#include "resource.h"
#include "qcnlib/QcnParser.h"
#include <msxml.h>
#include <atlstr.h>
#import "msxml6.dll" raw_interfaces_only
#include "PST.h"

#include "PortStateUI.h"

flash_image::flash_image(const wchar_t* config):
  image_list(NULL),
  image_last(NULL),
  a5sw_kern_ver("Unknown"),
  a5sw_sys_ver("Unknown"),
  a5sw_usr_ver("Unknown"),
  fw_ver("Unknown"),
  qcn_ver("Unknown"),
  nv_buffer(NULL),
  nv_num(0),
  nv_cmd(NULL)
{
  CString path;
  int data_len;

  if (config == NULL) {
    ERROR("not specified config file name");
    return ;
  }

  data_len = GetPrivateProfileString(PKG_SECTION,
                                     PKG_PATH,
                                     GetAppPath(path).GetString(),
                                     pkg_dir,
                                     MAX_PATH,
                                     config);

  if (pkg_dir[data_len - 1] != L'\\' ) {
    if ( data_len > MAX_PATH - 2) {
    ERROR("bad package directory in the section path.");
        return ;
    }
    pkg_dir[data_len] = L'\\';
    pkg_dir[data_len + 1] = L'\0';
    data_len++;
    }

  wcsncpy(pkg_conf_file, pkg_dir, data_len+1);
  wcsncat(pkg_conf_file, PKG_CONFIG_XML, sizeof(pkg_conf_file) / sizeof(pkg_conf_file[0]) - data_len);

  wcsncpy(pkg_qcn_file, pkg_dir, data_len+1);
  wcsncat(pkg_qcn_file, PKG_STATIC_QCN, sizeof(pkg_qcn_file) / sizeof(pkg_qcn_file[0]) - data_len);

  read_fastboot_config(config);
  read_diagpst_config(config);
  read_package_version(pkg_conf_file);
}

flash_image::~flash_image() {
  reset(TRUE);
}


int flash_image::read_diagpst_config(const wchar_t* config) {
  wchar_t partition_tbl[PARTITION_TBL_LEN] = {0};
  wchar_t filename[MAX_PATH];
  wchar_t *partition;
  size_t partition_len;
  CString path;
  int data_len;

  if (config == NULL) {
    ERROR("not specified config file name");
    return -1;
  }

  data_len = GetPrivateProfileString(DIAGPST_SECTION,
                                     NULL,
                                     NULL,
                                     partition_tbl,
                                     PARTITION_TBL_LEN,
                                     config);

  if (data_len == 0) {
    LOGW("no DIAG PST in%S .", config);
    return 0;
  }

  partition = partition_tbl;
  partition_len = wcslen(partition);

  while (partition_len > 0) {
    data_len = GetPrivateProfileString(DIAGPST_SECTION,
                                       partition,
                                       NULL,
                                       filename,
                                       MAX_PATH,
                                       config);
    if (data_len > 0) {
//        path.Empty();
        path = pkg_dir;
        //path += L'\\';
        path += filename;
      AddFileBuffer(partition, path.GetBuffer(), filename);
      path.ReleaseBuffer();
    }

    partition = partition + partition_len + 1;
    partition_len = wcslen(partition);
  }

  return 0;
}

int flash_image::read_fastboot_config(const wchar_t* config) {
  wchar_t partition_tbl[PARTITION_TBL_LEN] = {0};
  wchar_t filename[MAX_PATH];
  wchar_t *partition;
  size_t partition_len;
  CString path;
  int data_len;

  if (config == NULL) {
    ERROR("not specified config file name");
    return -1;
  }

  data_len = GetPrivateProfileString(PARTITIONTBL_SECTION,
                                     NULL,
                                     NULL,
                                     partition_tbl,
                                     PARTITION_TBL_LEN,
                                     config);

  if (data_len == 0) {
    WARN("no %S exist, load default partition table.", config);
    wchar_t *imgs[] = {
      L"mibib", L"sbl1.mbn",
      L"sbl2", L"sbl2.mbn",
      L"rpm", L"rpm.mbn",
      L"dsp1", L"dsp1.mbn",
      L"dsp3", L"dsp3.mbn",
      L"dsp2", L"dsp2.mbn",
      L"aboot", L"appsboot.mbn",
      L"boot", L"boot-oe-msm9615.img",
      L"system", L"9615-cdp-image-9615-cdp.yaffs2",
      L"userdata", L"9615-cdp-usr-image.usrfs.yaffs2",
    };

    for (int i = 0; i < sizeof(imgs)/ sizeof(imgs[0]); i += 2) {
        //parameter push stack from right to left
        add_image(imgs[i], imgs[i+1], TRUE, config);
    }

    set_package_dir(GetAppPath(path).GetString(), config);
    return 0;
  }

  partition = partition_tbl;
  partition_len = wcslen(partition);

  while (partition_len > 0) {
    data_len = GetPrivateProfileString(PARTITIONTBL_SECTION,
                                       partition,
                                       NULL,
                                       filename,
                                       MAX_PATH,
                                       config);
    if (data_len > 0) {
//        path.Empty();
        path = pkg_dir;
        //path += L'\\';
        path += filename;
      add_image(partition, path.GetBuffer(), 0, config);
      path.ReleaseBuffer();
    }

    partition = partition + partition_len + 1;
    partition_len = wcslen(partition);
  }

  return 0;
}



int flash_image::add_image( wchar_t *partition, const wchar_t *lpath, BOOL write, const wchar_t* config)
{

  FlashImageInfo* img = NULL;

  if (partition == NULL || lpath == NULL) {
    ERROR("Bad parameter");
    return -1;
  }

  img = (FlashImageInfo *)calloc(1, sizeof(FlashImageInfo));
  //img = (FlashImageInfo *)malloc(sizeof(FlashImageInfo));
  if (img == NULL) ERROR("out of memory");
  //memset(img, 0, sizeof(FlashImageInfo));
  img->data = load_file(lpath, &img->size);

  if (img->data == NULL) {
    ERROR("can not load data from file %S for  partition %S", lpath, partition);
    free(img);
    return -1;
  }
  int iDl = GetPrivateProfileInt(PARTITIONTBL_DL,
									  partition,
									  1,
									  config);

  img->need_download = (1==iDl)?true:false;
  img->partition = wcsdup(partition);
  img->partition_str = WideStrToMultiStr(partition);
  img->lpath = wcsdup(lpath);

  if (image_last != NULL)
    image_last->next = img;
  else
    image_list = img;

  image_last = img;

  DEBUG("Load data from file %S for partition %S", lpath, partition);

  if (write && config != NULL) {
    WritePrivateProfileString(PARTITIONTBL_SECTION,partition,lpath,config);
  }

  return 0;
}

bool flash_image::AddFileBuffer(const wchar_t *partition, const wchar_t *lpath, const wchar_t *fileName) {
    char * part = WideStrToMultiStr(partition);
    char * fn = WideStrToMultiStr(fileName);
    //int bytes = sizeof (wchar_t) * (2+ wcslen(pkgPath)+ wcslen(filName));
    //wchar_t *lpath = (wchar_t *)malloc(bytes);
    if (part != NULL && fn != NULL && lpath != NULL){
        //memset(lpath, 0, bytes);
        FileBufStruct afBuf;
        afBuf.strFileBuf = (uint8*)load_file(lpath, &afBuf.uFileLens);
        if(afBuf.strFileBuf != NULL) {
        afBuf.strFileName = strdup(fn);
        afBuf.isDownload= true;
        strcpy((char *)(afBuf.partition + 2), part);
        m_dlFileBuffer.insert(std::pair<string,FileBufStruct>(fn,afBuf));
        LOGE("Insert diag file %s , partition %s", fn, part);
        }
    }
    DELETE_IF(part);
    DELETE_IF(fn);
    //FREE_IF(lpath);
    return true;
}


const wchar_t * flash_image::get_package_dir(void) {
    return pkg_dir;
}

const wchar_t * flash_image::get_package_config(void) {
    return pkg_conf_file;
}


const wchar_t * flash_image::get_package_qcn_path(void) {
  return pkg_qcn_file;
}

BOOL flash_image::set_package_dir(const wchar_t * dir, const wchar_t* config, BOOL release) {
    if(dir == NULL) {
        ERROR("Bad Parameter.");
        return FALSE;
    }

    if (wcscmp(dir, pkg_dir) == 0) {
        INFO("Package directory is not changed.");
        return FALSE;
    }

    wcscpy(pkg_dir, dir);

    if (config != NULL)
    WritePrivateProfileString(PKG_SECTION, PKG_PATH, dir ,config);

#if 0
    if(release) {
        reset(FALSE);
        read_config(config);
        read_package_version(PKG_CONFIG_XML);
    }
#endif

    return TRUE;
}

const FlashImageInfo* flash_image::get_partition_info(wchar_t *partition, void **ppdata, unsigned *psize) {
  FlashImageInfo* img;

  // ASSERT( ppdata == NULL || psize == NULL );

  for (img = image_list; img; img = img->next) {
    if (wcscmp(partition, img->partition) == 0) {
      if(ppdata != NULL)
        *ppdata = img->data;
      if(psize != NULL)
        *psize = img->size;
      return img;
    }
  }
  return NULL;
}

const FlashImageInfo* flash_image::image_enum_init (void) {
    return image_list;
}

const FlashImageInfo* flash_image::image_enum_next (const FlashImageInfo* img) {
    if (img == NULL)
        return NULL;

    return img->next;
}

BOOL flash_image::qcn_cmds_enum_init (char *cmd) {
  if (cmd == NULL)
    cmd = "nv write";

  if (nv_cmd == NULL || strcmp(nv_cmd, cmd) != 0)  {
     FREE_IF(nv_cmd);
     nv_cmd = strdup(cmd);
     if(nv_cmd == NULL) {
      ERROR("OUT OF MEMORY");
      return FALSE;
     }

     if (nv_buffer != NULL) {
      QcnParser::PutNVWriteCommands(nv_buffer, nv_num);
      nv_buffer = NULL;
      nv_num = 0;
      }
  }

  if (nv_buffer == NULL)
   {
  QcnParser chQcn;

  if (chQcn.OpenDocument(pkg_qcn_file) == FALSE)
  {
    return FALSE;
  }
  return chQcn.GetNVWriteCommands(nv_cmd, &nv_buffer, &nv_num);
 // chQcn.PutNVWriteCommands(nvBuf, dwLens);
  }

    return TRUE;
}

const char* flash_image::qcn_cmds_enum_next (unsigned int index) {
  if (nv_buffer == NULL) {
    ERROR("No nv cmds buffer, may be should call qcn_cmds_enum_init first!");
    return NULL;
  }

  if (index >= nv_num) {
    ERROR("index (%d) is exceed nv number (%d)!", index, nv_num);
    return NULL;
  }

  return *(nv_buffer + index);
}

void flash_image::read_package_version(const wchar_t * package_conf){
  CComPtr<MSXML2::IXMLDOMDocument> spDoc;
  CComPtr<MSXML2::IXMLDOMNodeList> spNodeList;
  CComPtr<MSXML2::IXMLDOMElement> spElement;
  CComBSTR strTagName;
  VARIANT_BOOL bFlag;
  long lCount;
  HRESULT hr;

  ::CoInitialize(NULL);
  hr = spDoc.CoCreateInstance(__uuidof(MSXML2::DOMDocument));    //创建文档对象
  hr = spDoc->load(CComVariant(package_conf), &bFlag);       //load xml文件
  hr = spDoc->get_documentElement(&spElement);   //获取根结点
  if (spElement == NULL) {
    ERROR("No %S exist", package_conf);
    return;
    }
  hr = spElement->get_tagName(&strTagName);

  //cout << "------TagName------" << CString(strTagName) << endl;

  hr = spElement->get_childNodes(&spNodeList);   //获取子结点列表
  hr = spNodeList->get_length(&lCount);

  for (long i=0; i<lCount; ++i) {
    CComVariant varNodeValue;
    CComPtr<MSXML2::IXMLDOMNode> spNode;
    MSXML2::DOMNodeType NodeType;
    CComPtr<MSXML2::IXMLDOMNodeList> spChildNodeList;

    hr = spNodeList->get_item(i, &spNode);         //获取结点
    hr = spNode->get_nodeType(&NodeType);     //获取结点信息的类型

    if (NODE_ELEMENT == NodeType) {
      long childLen;
      hr = spNode->get_childNodes(&spChildNodeList);
      hr = spChildNodeList->get_length(&childLen);

      //cout << "------NodeList------" << endl;

      for (int j=0; j<childLen; ++j) {
        CComPtr<MSXML2::IXMLDOMNode> spChildNode;
        CComBSTR value;
        CComBSTR name;

        hr = spChildNodeList->get_item(j, &spChildNode);
        hr = spChildNode->get_nodeName(&name);            //获取结点名字
        hr = spChildNode->get_text(&value);                //获取结点的值
        //cout << CString(name) << endl;
        //cout << CString(value) << endl << endl;

        parse_pkg_sw(CString(name), CString(value));

        spChildNode.Release();
      }
    }

    spNode.Release();
    spChildNodeList.Release();
  }

  spNodeList.Release();
  spElement.Release();
  spDoc.Release();
  ::CoUninitialize();
}


 int flash_image::parse_pkg_sw(CString & node, CString & text) {
    if (node == L"Linux_Kernel_Ver")
        a5sw_kern_ver = text;

    else if (node == L"Linux_SYS_Ver")
        a5sw_sys_ver = text;

    else if (node == L"Linux_UserData_Ver")
        a5sw_usr_ver = text;

    else if (node == L"Q6_Resource_Ver")
        fw_ver = text;

    else if (node == L"QCN")
        qcn_ver = text;

    return 0;
}
 int flash_image::parse_pkg_hw(CString & node, CString & text) {
    return 0;
}

BOOL flash_image::get_pkg_a5sw_sys_ver(CString &version) {
    version = a5sw_sys_ver;
    return TRUE;
}
BOOL flash_image::get_pkg_a5sw_usr_ver(CString &version) {
    version = a5sw_usr_ver;
    return TRUE;
}
BOOL flash_image::get_pkg_a5sw_kern_ver(CString &version) {
    version = a5sw_kern_ver;
    return TRUE;
}
BOOL flash_image::get_pkg_qcn_ver(CString &version) {
    version = qcn_ver;
    return TRUE;
}
BOOL flash_image::get_pkg_fw_ver(CString &version) {
    version = fw_ver;
    return TRUE;
}

BOOL flash_image::set_download_flag(CString strPartitionName, bool bDownload) {
	BOOL bRet = 0;
	FlashImageInfo* img = image_list;
	for(;img != NULL; ) {
		if (0 == wcscmp(img->partition, strPartitionName.GetBuffer()))
		{
			img->need_download = bDownload;
			bRet = 1;
			break;
		}
		img = img->next;
	}
	return bRet;
}

BOOL flash_image::reset(BOOL free_only) {
    FlashImageInfo *img;
    for (img = image_list; img; img = image_list) {
      image_list = img->next;
      if (img->partition != NULL) {
        free(img->partition);
        img->partition = NULL;
      }

      if (img->partition_str != NULL) {
        delete img->partition_str;
        img->partition_str = NULL;
      }

      if (img->lpath != NULL) {
        free(img->lpath);
        img->lpath = NULL;
      }

      if (img->data != NULL) {
        free(img->data);
        img->data = NULL;
      }

      free(img);
      img = NULL;
    }

    image_list = NULL;

    if (!free_only) {
      a5sw_kern_ver=("Unknown"),
      a5sw_sys_ver=("Unknown"),
      a5sw_usr_ver=("Unknown"),
      fw_ver=("Unknown"),
      qcn_ver=("Unknown");
      if (nv_buffer != NULL) {
      QcnParser::PutNVWriteCommands(nv_buffer, nv_num);
      nv_buffer = NULL;
      nv_num = 0;
      }
      FREE_IF (nv_cmd);
    }

          std::map<string,FileBufStruct>::iterator it;
    for (it = m_dlFileBuffer.begin(); it != m_dlFileBuffer.end(); it++)
    {
        FREE_IF(it->second.strFileBuf);
 FREE_IF(it->second.strFileName);
    }

	return TRUE;
}

