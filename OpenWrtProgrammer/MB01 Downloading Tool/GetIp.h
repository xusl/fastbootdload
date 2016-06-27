#pragma once
#include <string>
#include<iostream>
#include <WinSock2.h>
#include <Iphlpapi.h>

using namespace std;
#pragma comment(lib,"Iphlpapi.lib") 

class GetIp
{
public:
	GetIp(void);
	~GetIp(void);
	bool GetAdapter();
    string IP()
    {
        return device_ip;
    }
private:
	string device_ip;
};
