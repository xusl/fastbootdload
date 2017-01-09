#ifndef TESTITEMDEFINE_H
#define TESTITEMDEFINE_H



#define 0   DIAG_ReadSSID(char *ssid)
#define 1   DIAG_WriteSSID(char *ssid)
#define 2   DIAG_RestartWiFi(void);
#define 3   DIAG_DisableSecurity(void);
#define 4   DIAG_CheckStaConnect(void);
#define 5   DIAG_Charger_TEST(void);
#define 6   DIAG_EnterLCDScreenTEST(void);
#define 7   DIAG_LCDScreen_Turn_On(void);
#define 8   DIAG_LCDScreen_Turn_Off(void);
#define 9   DIAG_EXITLCDScreenTEST(void);
#define 10   DIAG_EnterLEDTEST(void);
#define 11   DIAG_EXITLEDTEST(void);
#define 12   DIAG_REDLEDTEST(void);    //LED
#define 13   DIAG_GREENLEDTEST(void);  //OLED
#define 14   DIAG_BLUELEDTEST(void);
#define 15   DIAG_ENTRYKEYTEST(void);
#define 16   DIAG_KEYTEST(void);
#define 17   DIAG_EXITKEYTEST(void);
#define 18   DIAG_ReadFIRMWAREVersion(char* FirmVer);
#define 19   DIAG_CheckSD_Card(void);
#define 20   DIAG_CheckSIM_Card(void);
#define 21   DIAG_CheckNetWork(void);
#define 22   DIAG_TraceAbilityRead(TraceInfo *pTrace);
#define 23   DIAG_TraceAbilityWrite(char *traceInfo);
#define 24   DIAG_TraceAbilityLocal(TraceInfo *pTrace);


#endif // TESTITEMDEFINE_
