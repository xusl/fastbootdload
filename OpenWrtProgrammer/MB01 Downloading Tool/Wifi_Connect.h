#pragma once
#include "stdafx.h"
#include<Windows.h>
#include<wlanapi.h>
#include<ObjBase.h>
#include<WTypes.h>
#include<string>
#include<stdio.h>
#include<stdlib.h>
#include "device.h"
#pragma comment(lib,"wlanapi.lib")
#pragma comment(lib,"ole32.lib")
#define BUFFER_HEADER_LEN (64)
#define MAX_RETRY_NUM	(100)
using namespace std;
class Wifi_Connect
{
public:
	Wifi_Connect(void);
	~Wifi_Connect(void);
	bool Get_WIFI_Name(WCHAR * wifi_name,string &Port_number,int iflag = 0);
	int listenStatus();
	bool get_Trace(char s_Trace[]);
	void Get_WIFI_MAC(char *wifi_mac);
	bool set_Trace(char s_Trace[]);	
private:
	int Wifi_Connect::Get_ComPort();
	bool Wifi_Connect::get_Mac(uint16 port,string &MAC, int iflag);		
	
	char 	s_WIFIAddr[13];
};

