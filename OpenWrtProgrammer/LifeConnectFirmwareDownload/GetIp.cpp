#include "StdAfx.h"
#include "GetIp.h"

GetIp::GetIp(void)
{
}

GetIp::~GetIp(void)
{
}

bool GetIp::GetAdapter()
{
    PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO();
    unsigned long stSize = sizeof(IP_ADAPTER_INFO);
    int nRel = GetAdaptersInfo(pIpAdapterInfo,&stSize);
    int netCardNum = 0;
    int IPnumPerNetCard = 0;
    if (ERROR_BUFFER_OVERFLOW == nRel)
    {
        delete pIpAdapterInfo;
        pIpAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[stSize];
        nRel=GetAdaptersInfo(pIpAdapterInfo,&stSize);
    }
    if (ERROR_SUCCESS == nRel)
    {
        PIP_ADAPTER_INFO pIpAdapterInfo_Temp=pIpAdapterInfo;
        while (pIpAdapterInfo_Temp)
        {
            IP_ADDR_STRING *pIpAddrString =&(pIpAdapterInfo_Temp->IpAddressList);
            do
            {
                device_ip =pIpAddrString->IpAddress.String;
                if(device_ip.find("172")!=-1)
                {
                    delete pIpAdapterInfo;
                    return true;
                }
                pIpAddrString=pIpAddrString->Next;
            } while (pIpAddrString);
            pIpAdapterInfo_Temp = pIpAdapterInfo_Temp->Next;
        }
    }

    if (pIpAdapterInfo)
    {
        delete pIpAdapterInfo;
    }
    return false;
}
