#include "StdAfx.h"
#include "GetIp.h"
#include <stdio.h>
#include "log.h"
#include <Icmpapi.h>
#pragma comment(lib,"Iphlpapi.lib")
//#pragma comment(lib,"icmp.lib")

GetIp::GetIp(string network):
    device_ip(""),
    segment(network)
{
    device_ip.clear();
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
                if(device_ip.find(segment)!=-1)
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

#define MAX_BUF_SIZE   300
bool Ping(const char *ip_addr) {
    HANDLE hIcmpFile = NULL;
    DWORD dwRetVal = 0;
    IPAddr ipaddr = INADDR_NONE;
    char SendData[] = "IP Monitor";
    DWORD ReplySize  = MAX_BUF_SIZE + sizeof(ICMP_ECHO_REPLY) + 8;
    char ReplyBuffer[MAX_BUF_SIZE + sizeof(ICMP_ECHO_REPLY) + 8];
    IP_OPTION_INFORMATION ipOptions;
    memset(&ipOptions, 0, sizeof(IP_OPTION_INFORMATION));
    ipOptions.Ttl = 255;
    ipOptions.Flags = NULL;

    ipaddr = inet_addr(ip_addr);
    if( ipaddr == INADDR_NONE ) {
        hostent* hp = gethostbyname(ip_addr);
        if( hp ) {
            memcpy(&ipaddr,hp->h_addr,hp->h_length);
        } else {
            return false;
        }
    }
    hIcmpFile = IcmpCreateFile();
    if( hIcmpFile == INVALID_HANDLE_VALUE ) {
        LOGE("IcmpCreateFile failed");
        return false;
    }

    dwRetVal = IcmpSendEcho( hIcmpFile,
                            ipaddr,
                            (LPVOID) SendData,
                            sizeof(SendData),
                            NULL,
                            ReplyBuffer,
                            ReplySize,
                            2000);
    IcmpCloseHandle(hIcmpFile);

    if( dwRetVal > 0 ) {
        PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)ReplyBuffer;
        struct in_addr ReplyAddr;
        ReplyAddr.S_un.S_addr = pEchoReply->Address;
        if( pEchoReply->Status != IP_SUCCESS ) {
            return false;
        }
        if( pEchoReply->Address != ipaddr ) {
        return false;
        }
        if( strcmp( SendData, (CHAR*)pEchoReply->Data ) != 0 ) {
            return false;
        }
        return true;
    } else {
        return false;
    }
}


int ResolveIpMac(const char *DestIpString, string & mac)
{
    #define BUFFER_LEN  6
    DWORD dwRetVal;
    IPAddr DestIp = 0;
    IPAddr SrcIp = 0;       /* default for src ip , means INADDR_ANY IPv4 address*/
    ULONG MacAddr[2];       /* for 6-byte hardware addresses */
    ULONG PhysAddrLen = 6;  /* default to length of six bytes */
    CHAR  Buffer[BUFFER_LEN];

    //char *DestIpString = NULL;
    //char *SrcIpString = NULL;

    BYTE *bPhysAddr;
    unsigned int i;

    if (DestIpString == NULL || DestIpString[0] == '\0') {
        LOGE("invalid destination IP address.");
        return 1;
    }
    mac.clear();

    DestIp = inet_addr(DestIpString);

    memset(&MacAddr, 0xff, sizeof (MacAddr));

    //LOGE("Sending ARP request for IP address: %s", DestIpString);

    dwRetVal = SendARP(DestIp, SrcIp, &MacAddr, &PhysAddrLen);

    if (dwRetVal == NO_ERROR) {
        bPhysAddr = (BYTE *) & MacAddr;
        if (PhysAddrLen) {
            for (i = 0; i < (int) PhysAddrLen; i++) {
                memset(Buffer, 0, BUFFER_LEN);
                if (i == (PhysAddrLen - 1))
                    _snprintf(Buffer, BUFFER_LEN, "%.2X", (int) bPhysAddr[i]);
                else
                    _snprintf(Buffer, BUFFER_LEN, "%.2X-", (int) bPhysAddr[i]);
                mac += Buffer;
            }
            return 0;
        } else {
            LOGE("Warning: SendArp completed successfully, but returned length=0\n");
        }
    } else {
        const char * errstr = "";
        switch (dwRetVal) {
        case ERROR_GEN_FAILURE:
            errstr = _T(" (ERROR_GEN_FAILURE)");
            break;
        case ERROR_INVALID_PARAMETER:
            errstr = _T(" (ERROR_INVALID_PARAMETER)");
            break;
        case ERROR_INVALID_USER_BUFFER:
            errstr = _T(" (ERROR_INVALID_USER_BUFFER)");
            break;
        case ERROR_BAD_NET_NAME:
            errstr = _T(" (ERROR_GEN_FAILURE)");
            break;
        case ERROR_BUFFER_OVERFLOW:
            errstr = _T(" (ERROR_BUFFER_OVERFLOW)");
            break;
        case ERROR_NOT_FOUND:
            errstr = _T(" (ERROR_NOT_FOUND)");
            break;
        default:
            break;
        }
        LOGE("Error: SendArp failed with error: %d(%s)", dwRetVal, errstr);
    }

    return 1;
}