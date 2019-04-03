#ifndef SsidCfgMgr_h
#define SsidCfgMgr_h

#include "Arduino.h"
#include "FS.h"

// the maximum number of SSIDs to support
#define maxSsids 5

/**  Each SSID is stored as a piece of data */

class SsidCfgMgr {
  public:


    boolean isNullObj();

    static boolean initSsidCfgMgr();

    static boolean addSsid( String ssidName, String ssidPwd,  String ssidChannel,
                            String useDevMacStr, String macAddr, String useDhcpStr,
                            String ipAddr, String ipGateway, String ipSubnet,
                            String portHttp, String portTcp);
    static void deleteSsid(String ssidName);
    static void reOrder(String name, bool up);

    static SsidCfgMgr getSsidNameObject(String ssidName);

    // object data
    char* getName();
    char* getPwd();

    // change able, public to provide easy access
    String ssidChStr;     // 2
    String useDevMacStr;     // 5
    String macAddrStr;    // 17
    String httpPortStr;   // 5
    String httpTcpStr;    // 5
    String useDhcpStr;       // 5
    String ipAddrStr;     // 15
    String ipGatewayStr;  // 15
    String ipSubnetStr;   // 15

    void save();

    static String ssidnamArr[maxSsids];
    static SsidCfgMgr ssidObject[maxSsids];

    boolean load(String ssidNameP);

  private:
    SsidCfgMgr();


    static String ssidNnnnFile[maxSsids];


    // these may never change
    String ssidName;      // 32 chars
    String ssidPw;        // 64 chars

    static void nullIndex(int idx);

    static SsidCfgMgr nullObj;

    static boolean emptyClassObjectAssigned;
    boolean emptyClassObject;

    String fileName;
    String delimiter;

    // ssid map software
    static void changeOrderInternal(int index, bool up);

    static void readSsidmap();
    static void writeSsidmap();
    static void reWriteLoad();
    static String getFreeName();

    static int getSsidNameIndex(String ssidName);
    static boolean loadSsid(String ssidNameP);

    char ssidName2ConnBuf[100];
    char ssidPwd2ConnBuf[300];
};

#endif
