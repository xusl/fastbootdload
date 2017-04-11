#!/bin/python
#coding=utf-8
# -*- coding: utf-8 -*-
import os, sys, stat
import shutil
import time
import shlex, subprocess
import ConfigParser
import logging
from optparse import OptionParser
from xml.dom import minidom
from struct import *
import platform

logger = None
cwdname = os.path.dirname(os.getcwd())
adsuFileName = 'ADSU.exe'
ModemImgDir = 'ModemImage'
PkgDir = 'package'

g_modemimagePath = os.path.join(cwdname, ModemImgDir)
g_configxmlPath = os.path.join(cwdname, ModemImgDir, 'config.xml')
g_efsPath = os.path.join(cwdname, ModemImgDir, 'efs.mbn')
g_kernelADSUPath = os.path.join(cwdname, PkgDir, 'kernelADSU', adsuFileName)
g_downloadimgPath = os.path.join(cwdname, 'DownloadImage')
g_fotaPath = os.path.join(cwdname, 'FotaImage')
g_downloadimgfile = os.path.join(cwdname, 'DownloadImage', 'Download.img')
g_downloadimgTempPath = os.path.join(cwdname, PkgDir, 'Download.img')
g_winADSUPath = os.path.join(cwdname, 'ADSU', 'WIN')
g_winADSUfile = os.path.join(cwdname, 'ADSU','WIN', adsuFileName)
g_tempADSUPath = os.path.join(cwdname, PkgDir, adsuFileName)
g_helpPath = os.path.join(cwdname, PkgDir, 'resource', 'Help.chm')
g_dynamicNVPath = os.path.join(cwdname, PkgDir, 'resource', 'dynamic_nv_281.xml')
g_iconPath = os.path.join(cwdname, PkgDir, 'resource', 'HspaUsbModem.ico')
g_strPath = os.path.join(cwdname, PkgDir, 'resource', 'string_multiLanguage.txt')
g_bmpPath = os.path.join(cwdname, PkgDir, 'resource', 'Upgrader.bmp')
nReleaseNoteTableItemCount = 13
g_customer_info_Path = os.path.join(cwdname, ModemImgDir, 'custom_info.xml')
Version = cwdname
PackageVersion = Version.split(os.sep)[-1]

sTime = time.strftime('%Y-%m-%d',time.localtime(time.time()))

class FileItem(object):
    def __init__(self, name, path, position, size):
        self.name = name
        self.path = path
        self.position = position
        self.size = size

class Platform(object):
    def __init__(self, platform, headerLen = 200, filesLen = 150000):
        self.platfrom = platform
        #image header struture data
        # according VERSION_HEAD_LEN
        self.headerTotalLength = headerLen
        # according FILEINFO_HEAD_LEN_NEW
        self.filesInfoTotalLength = filesLen
        self.appVersionSize = 32
        # according char      fileName[72]; in FilePosInfoNewS
        self.filenameBufferLen = 72

    def buildFileItems(self, sinkpath, names, offset):
        '''
        因为是Heander 和 Data是分开的，要保持一致，用列表。
        '''
        items = []
        pos = offset
        for name in names:
            if sinkpath is not None:
                path = os.path.join(sinkpath, name)
            else:
                path = name

            if name == "NPRG9x25.mbn" or name == "hostdl.mbn" or \
               name == "nandprg.mbn":
                itemname = "nandprgcombined.mbn"
            else:
                itemname = name
            try:
                size = os.path.getsize(path)
                items.append(FileItem(name, path, pos, size))
                pos += size
            except OSError as oe:
                print oe
        return items

class PlatformMDM9x25(Platform):
    pass

class PlatformMDM9x30(Platform):
    pass

class Project(object):
    def __init__(self, code="", name="", externalVersion="", internalVersion=None, curef=None):
        self.code = code
        self.name = name
        self.externalVersion = externalVersion
        self.internalVersion = internalVersion
        self.curef = curef
        self.platform = None

    def setProjectCode(self, code):
        if not self.__checkValue__(code):
            return
        self.code = code
        self.setPlatform(code)

    def setProjectName(self, name):
        if not self.__checkValue__(name):
            return
        self.name = name

    def setExternalVersion(self, version):
        if not self.__checkValue__(version):
            return
        self.externalVersion = version

    def setInternalVersion(self, version):
        if not self.__checkValue__(version):
            return
        self.internalVersion = version

    def setCURef(self, curef):
        if not self.__checkValue__(curef):
            return
        self.curef = curef

    def __checkValue__(self, value, msg=None):
        error = False
        if value is None or len(value) == 0:
            error = True
        elif not isinstance(value, str) and not isinstance(value, unicode):
            error = True
        if error:
            if msg is not None:
                print msg + " do not have valid value"
            else:
                print "Occur bad value " + value
            return False
        return True

    def setPlatform(self, name):
        self.platform = Platform(name)

    def getPlatform(self):
        return self.platform

    def getImageVersion(self, isString = True):
        project_code = self.code
        version = "0.0"
        if self.__checkValue__(project_code, "The project code"):
            if  (project_code == "Y850") or (project_code == "L850") or \
                (project_code == "Y854") or (project_code == "Y853") or \
                (project_code == "Y855") or (project_code == "Y856") or \
                (project_code == "Y858") or (project_code == "Y860") or \
                (project_code == "Y859"):
                version = '6.0'
            elif(project_code == 'M850'):
                version = '7.0'
            elif(project_code == 'Y900') or (project_code == "Y901"):
                version = '8.0'
            else:
                print "Unspport project " + code
        if isString:
            return version
        else:
            return int(float(version))

    def getConfigFilesSection(self):
        version = self.getImageVersion()
        if version == '6.0':
            return "WriteFiles_IMG5"
        elif version == '7.0':
            return "WriteFiles_IMG6"
        elif version == '8.0':
            return "WriteFiles_IMG7"

    def getPidVidText(self, indent='\n'):
        project_code = self.code
        if not self.__checkValue__(project_code, "The project code"):
            return ''

        if (project_code == "Y850") or (project_code == "L850") or \
           (project_code == "Y855") or (project_code == "Y856") or \
           (project_code == "Y858") or (project_code == "Y900") or \
           (project_code == "Y901"):
            return 'PID: PID_0195' + indent + 'VID: VID_1BBB'
        elif (project_code == "Y860") or (project_code == "Y859"):
            return 'PID: PID_0197' + indent + 'VID: VID_1BBB'
        print "Unknow project code " + code + ". Retrun empty PID/VID string"
        return ''

def ConfigxmlValue():
    project = Project()
    doc = minidom.parse(g_configxmlPath)
    root = doc.documentElement
    for Base_Info in root.getElementsByTagName("Base_Info"):
        print("-------------------------------------------")
        nameNode1 = Base_Info.getElementsByTagName("Project_Code")[0]
        project.setProjectCode(nameNode1.childNodes[0].nodeValue)
        print (nameNode1.nodeName + ":" + nameNode1.childNodes[0].nodeValue)

        nameNode2 = Base_Info.getElementsByTagName("Project_Name")[0]
        print (nameNode2.nodeName + ":" + nameNode2.childNodes[0].nodeValue)
        #global project_name
        #project_name = nameNode2.childNodes[0].nodeValue
        project.setProjectName(nameNode2.childNodes[0].nodeValue)

    for Software in root.getElementsByTagName("Software"):
        print("-------------------------------------------")
        nameNode1 = Software.getElementsByTagName("External_Ver")[0]
        print (nameNode1.nodeName + ":" + nameNode1.childNodes[0].nodeValue)
        project.setExternalVersion(nameNode1.childNodes[0].nodeValue)

        project_code = project.code
        if(project_code == "Y850") or (project_code == "L850") or \
          (project_code == "Y855") or (project_code == "Y854") or \
          (project_code == "Y853"):
            nameNode2 = Software.getElementsByTagName("Internal_Ver")[0]
            print (nameNode2.nodeName + ":" + nameNode2.childNodes[0].nodeValue)
            project.setInternalVersion(nameNode2.childNodes[0].nodeValue)

    contents3 = root.getElementsByTagName("CURef")[0]
    for contents3 in contents3.childNodes:
        if contents3.nodeType in (contents3.TEXT_NODE,contents3.CDATA_SECTION_NODE):
            cu_ref = contents3.data
            print 'CURef:' + cu_ref
            project.setCURef(cu_ref)

    return project

def customer_info():
    doc = minidom.parse(g_customer_info_Path)
    root = doc.documentElement
    contents = root.getElementsByTagName("security_mode_value")[0]
    for contents in contents.childNodes:
        print("-------------------------------------------")
        if contents.nodeType in (contents.TEXT_NODE,contents.CDATA_SECTION_NODE):
            global sec_value
            sec_value = contents.data
            print 'Security code:' + sec_value

    contents2 = root.getElementsByTagName("Action")
    for Action in contents2:
        global PN_value,PU_value,PP_value,PC_value,PF_value
        nameNode1 = Action.getElementsByTagName("PN")[0]
        PN_value = nameNode1.childNodes[0].nodeValue
        nameNode2 = Action.getElementsByTagName("PU")[0]
        PU_value = nameNode2.childNodes[0].nodeValue
        nameNode3 = Action.getElementsByTagName("PP")[0]
        PP_value = nameNode3.childNodes[0].nodeValue
        nameNode4 = Action.getElementsByTagName("PC")[0]
        PC_value = nameNode4.childNodes[0].nodeValue
        nameNode5 = Action.getElementsByTagName("PF")[0]
        PF_value = nameNode5.childNodes[0].nodeValue
        print PN_value,PU_value,PP_value,PC_value,PF_value

def getSIMLockStatus():
    if (PN_value == '0') and (PU_value == '0') and (PP_value == '0') and \
        (PC_value == '0') and (PF_value == '0'):
        return 'No simlock'
    elif (PN_value == '2') and (PU_value == '0') and (PP_value == '0') and \
        (PC_value == '0') and (PF_value == '0'):
        return 'Normal simlock'
    else:
        return 'Has simlock'

def getVersionText(project, indent='\n'):
    external_ver = project.externalVersion
    internal_ver = project.internalVersion
    if internal_ver is None:
    #if (project_code == "Y900") or (project_code == "Y860") or \
    #   (project_code == "Y858") or (project_code == "Y901") or \
    #   (project_code == "Y859"):
        internal_ver = external_ver
    return 'External:' + external_ver + indent + 'Internal:' + internal_ver

def modifyReleaseNote(docPath, PackageVersion, project):
    import win32com.client
    w = win32com.client.Dispatch('word.Application')
    w.Visible = 0
    w.DisplayAlerts = 0

    doc = w.Documents.Open(docPath)
    table = doc.Tables[0]
    nIndex = 0
    while nIndex < nReleaseNoteTableItemCount:
        row = table.Rows[nIndex]
        nameCellText = row.Cells[0].Range.Text
        if nameCellText.find(u"Package Version") == 0:
            row.Cells[1].Range.Text = unicode(PackageVersion)
        elif nameCellText.find(u"Firmware Version") == 0:
            row.Cells[1].Range.Text = unicode(getVersionText(project))
        elif nameCellText.find(u"TPST Version") == 0:
            row.Cells[1].Range.Text = unicode('TPW0060000')
        elif nameCellText.find(u"ADSU Version") == 0:
            row.Cells[1].Range.Text = unicode('ADSU0052000')
        elif nameCellText.find(u"Online index") == 0:
            row.Cells[1].Range.Text = unicode(project.curef)
        elif nameCellText.find(u"Online update") == 0:
            row.Cells[1].Range.Text = unicode('Support')
        elif nameCellText.find(u"WIFI security") == 0:
            if (sec_value == '0'):
                row.Cells[1].Range.Text = unicode('Disabled')
            else:
                row.Cells[1].Range.Text = unicode('Enabled')
        elif nameCellText.find(u"PID/VID Information") == 0:
            row.Cells[1].Range.Text = unicode(project.getPidVidText())
        elif nameCellText.find(u"Simlock") == 0:
            row.Cells[1].Range.Text = unicode(getSIMLockStatus())
        elif nameCellText.find(u"Release Date") == 0:
            row.Cells[1].Range.Text = unicode(sTime)

        w.Selection.Find.ClearFormatting()
        w.Selection.Find.Replacement.ClearFormatting()
        w.Selection.Find.Execute('<Proj>', False, False, False, False, False, True, 1, True, project.name, 2)
        w.Selection.Find.Execute('<release_date>', False, False, False, False, False, True, 1, True, sTime, 2)

        nIndex +=1
    #doc.SaveAs(os.path.join(os.path.dirname(os.getcwd()),'Release Notes.doc')
    doc.Close()
    w.Quit()

def writeReleaseNote(path, PackageVersion, project):
    fmt = "{0:>32}: {1:<}\n"
    indent = '\n' + ' ' * 34
    with open(path, 'w') as fd:
        fd.write(fmt.format("Package Version", PackageVersion))
        fd.write(fmt.format("Firmware Version", getVersionText(project, indent)))
        fd.write(fmt.format("TPST Version", 'TPW0060000'))
        fd.write(fmt.format("ADSU Version", 'ADSU0052000'))
        fd.write(fmt.format("Online index", project.curef))
        fd.write(fmt.format("Online update", 'Support'))
        fd.write(fmt.format("WIFI security", \
                'Disabled' if sec_value == '0' else 'Enabled'))
        fd.write(fmt.format("PID/VID Information", project.getPidVidText(indent)))
        fd.write(fmt.format("Simlock", getSIMLockStatus()))
        fd.write(fmt.format("Release Date", sTime))

def updateConfigIni(conf_ini = "config.ini", imgver=None):
    config = ConfigParser.ConfigParser()
    pkgSection = 'Packet'
    if not os.path.exists(conf_ini):
        config.add_section(pkgSection)
    else:
        config.read(conf_ini)
    config.set(pkgSection, 'ExeImgPath', g_tempADSUPath)
    config.set(pkgSection, 'SavePacketPath', g_downloadimgTempPath)
    config.set(pkgSection, 'HelpPath', g_helpPath)
    config.set(pkgSection, 'IconPath', g_iconPath)
    config.set(pkgSection, 'StringPath', g_strPath)
    config.set(pkgSection, 'PicturePath', g_bmpPath)
    config.set(pkgSection, 'DynamicNVPath', g_dynamicNVPath)
    config.set(pkgSection, 'ExePath', g_kernelADSUPath)
    config.set(pkgSection, 'SourceImagePath', g_modemimagePath)
    if imgver is not None:
        config.set(pkgSection, 'ImgVer', imgver)
    with open(conf_ini, 'wb') as out:
        config.write(out)
    return config

def writeSpacePad(ofd, num):
    if num <= 0:
        return
    ofd.write('\x00' * num)

def writeBufferString(ofd, string, bufferlen):
    bytes = pack('{0}s'.format(len(string)), string)
    ofd.write(bytes)
    padsize = bufferlen - len(bytes)
    writeSpacePad(ofd, padsize)

def writeAppVersion(ofd, config, type, name, size):
    #refer PacketHeadInfoS struct and APP_TYPE enum in src/comm/define.h
    #in Win32 enum hold 4 bytes.
    section = 'AppVersion'
    ofd.write(pack('I', type)) # such as firmware version, APP_TYPE_FIRMWARE
    try:
        version = config.get(section, name)
    except ConfigParser.NoOptionError as noop:
        print "writeAppVersio:" , nnoop
        version = 'V1.0.0'
    print version
    writeBufferString(ofd, version, size)
    return 4 + size

def wirteHeader(ofd, config, platform, version):
    buflen = platform.appVersionSize
    # image version 2 bytes
    ofd.write(pack('H', version))
    padSize = platform.headerTotalLength - 2
    # Every appversion hold 4 + 32 bytes
    padSize -= writeAppVersion(ofd, config, 1, "FirmwareVersion", buflen)
    padSize -= writeAppVersion(ofd, config, 2, "DashBoardVersion", buflen)
    padSize -= writeAppVersion(ofd, config, 3, "QcnVersion", buflen)
    padSize -= writeAppVersion(ofd, config, 4, "SIMLockVersion", buflen)
    #the header totally occupy 200 bytes
    writeSpacePad(ofd, padSize)
    logger.info("write versions to file header")

def findFilesOption():
    return []

def getFilesInConf(config, section):
    '''
config.ini
[WriteFiles_IMG6]
filenum = 15
file0 = NPRG9x25.mbn
file1 = partition.mbn
file2 = appsboot.mbn
file3 = mba.mbn
file4 = rpm.mbn
file5 = sbl1.mbn
.ini 设计缺陷：
1. filenum可以去掉
2. NPRG9x25.mbn 在写入img时改名nandprgcombined.mbn,
nandprgcombined.mbn = NPRG9x25.mbn
partition.mbn = partition.mbn
    '''
    try:
        files = config.items(section)
        #print files
        return list(zip(*files[1:])[1])
    except ConfigParser.NoOptionError as noop:
        print "writeFilesInformation:", noop
        return []

def writeFilesInformation(ofd, config, section, platform, sinkpath):
    totalLen = platform.filesInfoTotalLength
    files = getFilesInConf(config, section)
    files += findFilesOption()
    offset = ofd.tell() + platform.filesInfoTotalLength
    items = platform.buildFileItems(sinkpath, files, offset)
    fblen = platform.filenameBufferLen
    logger.info("write file list information")
    for item in items:
        writeBufferString(ofd, item.name, fblen)
        ofd.write(pack('I', item.position))
        ofd.write(pack('I', item.size))
    writeSpacePad(ofd, platform.filesInfoTotalLength - len(items) * (fblen + 4 + 4))
    return items

def writeFilesData(ofd, items):
    for item in items:
        logger.info("write file %s" % item.path)
        with open(item.path, 'rb') as ifd:
            while True:
                buffer = ifd.read(2048 * 1024)
                if len(buffer) == 0:
                        break
                ofd.write(buffer)

def writeCrcData(ofd):
    table = [                                                              \
		0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,    \
		0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,    \
		0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,    \
		0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,    \
		0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,    \
		0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,    \
		0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,    \
		0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,    \
		0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,    \
		0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,    \
		0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,    \
		0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,    \
		0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,    \
		0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,    \
		0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,    \
		0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,    \
		0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,    \
		0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,    \
		0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,    \
		0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,    \
		0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,    \
		0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,    \
		0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,    \
		0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,    \
		0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,    \
		0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,    \
		0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,    \
		0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,    \
		0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,    \
		0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,    \
		0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,    \
		0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78]

    total = ofd.tell()
    i = 0.0
    buflen = 8*1024*1024

    #reset file position
    ofd.seek(0)

    logger.info("Do CRC calcuate")
    crc = 0xFFFF
    while True:
        buf = ofd.read(buflen)
        if len(buf) == 0:
            break
        #print '.',
        i += len(buf)
        sys.stdout.write("%4d%%" % ((i/total)*100) + "\b" * 5)
        sys.stdout.flush()
        #if not 0<=data<256:
        #    print "data must between 0-255"
        #    continue
        for data in buf:
            value = table[(crc ^ ord(data)) & 0x00ff]
            crc = (crc >> 8) ^ value
    crc ^= 0xFFFF;
    ofd.write(pack('H', crc))

def rewriteDosHeader(fd, res2):
    """
typedef uint32_t DWORD;
typedef uint16_t WORD;
typedef int32_t  LONG;
typedef int      BOOL;
typedef struct _IMAGE_DOS_HEADER {      // DOS .EXE header
    WORD   e_magic;                     // Magic number
    WORD   e_cblp;                      // Bytes on last page of file
    WORD   e_cp;                        // Pages in file
    WORD   e_crlc;                      // Relocations
    WORD   e_cparhdr;                   // Size of header in paragraphs
    WORD   e_minalloc;                  // Minimum extra paragraphs needed
    WORD   e_maxalloc;                  // Maximum extra paragraphs needed
    WORD   e_ss;                        // Initial (relative) SS value
    WORD   e_sp;                        // Initial SP value
    WORD   e_csum;                      // Checksum
    WORD   e_ip;                        // Initial IP value
    WORD   e_cs;                        // Initial (relative) CS value
    WORD   e_lfarlc;                    // File address of relocation table
    WORD   e_ovno;                      // Overlay number
    WORD   e_res[4];                    // Reserved words
    WORD   e_oemid;                     // OEM identifier (for e_oeminfo)
    WORD   e_oeminfo;                   // OEM information; e_oemid specific
    WORD   e_res2[10];                  // Reserved words
    LONG   e_lfanew;                    // File address of new exe header
} IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;
    """
    fd.seek(2 * (14 + 4 + 2))
    # posDataStart is DWORD, which is 32 bits.
    fd.write(pack('I', res2))

def packageFiles(outfile, sinkpath, project, config):
    platform = project.getPlatform()
    imgversion = project.getImageVersion(False)
    with open(outfile, 'w+b') as ofd:
        wirteHeader(ofd, config, platform, imgversion)
        sec = project.getConfigFilesSection()
        items = writeFilesInformation(ofd, config, sec, platform, sinkpath)
        writeFilesData(ofd, items)
        writeCrcData(ofd)
    logger.info("Do packageFiles %s" % outfile)

def packageExe(project, exePath, sinkfiles):
    platform = project.getPlatform()
    items = platform.buildFileItems(None, sinkfiles, 0)
    if os.path.exists(exePath):
        os.remove(exePath)

    with open(exePath, 'wb') as ef:
        writeFilesData(ef, items)
        res2 = ef.tell()
        for item in items:
            ef.write(pack('I', item.position))
            ef.write(pack('I', item.size))
        rewriteDosHeader(ef, res2)
    logger.info("Do packageExe %s" % exePath)

def executeCommand(cmd):
    try:
        output = subprocess.check_output(shlex.split(cmd))
        logger.debug(output)
    except OSError as error:
        logger.debug("command {0} with error {1}".format(cmd,str(error)))

def main(winExec):
    project = ConfigxmlValue()
    config = updateConfigIni("config.ini", project.getImageVersion())

    if winExec:
        cmd=(os.path.join(cwdname, PkgDir, 'PKIMG.exe'))
        os.system(cmd)
    else:
        packageFiles(g_downloadimgTempPath, g_modemimagePath, project, config)
        # g_kernelADSUPath must be first,
        sinkfiles = [g_kernelADSUPath, g_downloadimgTempPath, g_bmpPath, \
                g_strPath, g_iconPath, g_dynamicNVPath, g_helpPath]
        packageExe(project, g_tempADSUPath, sinkfiles)

    if platform.system() == "Windows":
        basepath = os.path.join(cwdname, PkgDir)
        signtool = os.path.join(basepath, 'signtool.exe')
        key = os.path.join(basepath, 'JRDJRD.pfx')
        outfile = os.path.join(basepath, adsuFileName)
        cmd = [signtool, 'sign', '/f', key, '/p', 'mobile', '/t', \
                'http://timestamp.verisign.com/scripts/timstamp.dll', outfile]
        logger.info("sign {0}".format(adsuFileName))
        logger.info("execute command " + " ".join(cmd))
        try:
            subprocess.call(cmd)
        except OSError as error:
            logger.debug("command {0} with error {1}".format(cmd,str(error)))

    if(os.path.exists(g_downloadimgPath)):
        if os.path.exists(g_downloadimgfile):
            os.remove(g_downloadimgfile)
            print "Remove old one ok! "
        shutil.move(g_downloadimgTempPath, g_downloadimgPath)
        print "Packing and move Download.img OK!"
        shutil.copy2(g_configxmlPath,g_fotaPath)
        shutil.copy2(g_efsPath,g_fotaPath)
    else:
        print "Packing Download.img OK!"

    if(os.path.exists(g_winADSUPath)):
        if os.path.exists(g_winADSUfile):
            os.remove(g_winADSUfile)
            logger.debug("Remove {0}".format(g_winADSUfile))
        shutil.move(g_tempADSUPath, g_winADSUPath)
        logger.debug("Packing and move ADSU.exe OK!")
    else:
        print "Packing ADSU.exe OK!"

    if platform.system() == "Windows":
        docPath = os.path.join(cwdname, PkgDir, 'Release Notes.doc')
        if os.path.exists(docPath):
            customer_info()
            modifyReleaseNote(docPath, PackageVersion, project)
            shutil.copy2(docPath,Version)
            logger.info("Write Release notes!")
    else:
        docPath = os.path.join(cwdname, PkgDir, 'Release Notes.txt')
        customer_info()
        writeReleaseNote(docPath, PackageVersion, project)
        shutil.copy2(docPath,Version)
        logger.info("Write Release notes!")

def log_init(tag, debug, logname):
    #if not os.path.exists(os.path.dirname(logname)):
    #    os.mkdir(os.path.dirname(logname), 0777)

    if debug:
        file_level = logging.DEBUG
        consoel_level = logging.DEBUG
    else:
        file_level = logging.DEBUG
        consoel_level = logging.INFO

    logging.basicConfig(level=file_level,#logging.DEBUG,
                        format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                        datefmt='%m-%d %H:%M',
                        filename=logname,
                        filemode='a')  #使用追加模式，这也是默认的模式。
    console = logging.StreamHandler()
    console.setLevel(consoel_level)
    formatter = logging.Formatter('%(name)-12s: %(levelname)-8s %(message)s')
    console.setFormatter(formatter)
    logging.getLogger('').addHandler(console)
    logger = logging.getLogger(tag)
    return logger

if __name__ == '__main__':
    parser = OptionParser(usage=u"\n\tpython %prog [opts] [args]\n",
                description="JRD SCD MBB image package tools")
    parser.add_option("-c", "--config", dest="conf",
                action="store", type="string", help=u"configuration file",
                metavar=u"<config.ini>", default="config.ini")
    parser.add_option("-d", "--debug", action="store_const", const=1,
                help=u"develop debug")
    parser.add_option("-o", "--output", action="store", type="string",
                help=u"output path")
    parser.add_option("-w", dest="winExec", action="store_true",
                default=False, help=u"Use Windows Execute PKIMG.exe."\
                "By default, use Python code to package images.")
    (opts, args) = parser.parse_args()
    parser.add_option("-v", "--verbose", action="store_true",
                default=False, help=u"verbose mode, output more log")
    (opts, args) = parser.parse_args()
    logger = log_init("Package", opts.verbose, 'package-{0}.log'.format(sTime))
    main(opts.winExec)
