#pragma once
#include <string>
#include<iostream>
#include <WinSock2.h>
#include <Iphlpapi.h>

using namespace std;


bool Ping(const char *ip_addr);

int ResolveIpMac(const char *DestIpString, string & mac);

class GetIp
{
public:
    GetIp(string network="192.168.1");
    ~GetIp(void);
    bool GetAdapter();
    bool GetHostIP(string& ip, string& gw) {
        int refresh = 1;
        do {
            if (!device_ip.empty() &&(device_ip !="")&&(device_ip !="0.0.0.0")) {
                ip = device_ip;
                gw = gateway_ip;
                return true;
            }
            if (refresh >= 1) {
                if(!GetAdapter())
                    return false;
            }
        } while(refresh-- >= 0);
        return false;
    }
private:
    string segment;
    string device_ip;
    string gateway_ip;
};
