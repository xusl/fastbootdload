#ifndef __WORKINFORMATION_H__
#define __WORKINFORMATION_H__
class WorkInformation {
public:
BOOL AddDevInfo(CString name, CString value) = 0;
BOOL  SetProgress(int progress) = 0;
};
#endif