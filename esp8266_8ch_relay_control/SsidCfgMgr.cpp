#include "SsidCfgMgr.h"
#include "Arduino.h"
#include "FS.h"

#define DBG_OUTPUT_PORT Serial

const char* cfgSsidMapFilename PROGMEM = "/cfgssidmap.t";

const char* keySsid PROGMEM = "ssid=";
const char* keySsidpw PROGMEM = "ssidpw=";
const char* keySsidChannel PROGMEM = "ssidch=";
const char* keyUseDevMac = "usedevmac=";
const char* keyMac PROGMEM = "mac=";
const char* keyDhcp PROGMEM = "dhcp=";
const char* keyIpadr PROGMEM = "ipadr=";
const char* keyIpgw PROGMEM = "ipgw=";
const char* keyIpsubnet PROGMEM = "ipsubnet=";
const char* keyHtpport PROGMEM = "htpport=";
const char* keyTcpport PROGMEM = "tcpport=";

String SsidCfgMgr::ssidnamArr[maxSsids];
String SsidCfgMgr::ssidNnnnFile[maxSsids];
//
SsidCfgMgr SsidCfgMgr::ssidObject[maxSsids];

SsidCfgMgr SsidCfgMgr::nullObj;

boolean SsidCfgMgr::emptyClassObjectAssigned = false;
boolean emptyClassObject = false;

SsidCfgMgr::SsidCfgMgr() {
  //
}

char* SsidCfgMgr::getName() {
  this->ssidName.toCharArray(ssidName2ConnBuf, 99);
  return ssidName2ConnBuf;
}
char* SsidCfgMgr::getPwd() {
  this->ssidPw.toCharArray(ssidPwd2ConnBuf, 299);
  return ssidPwd2ConnBuf;
}

boolean SsidCfgMgr::initSsidCfgMgr() {
  // create a null object
  if (emptyClassObjectAssigned) {
    return false;
  }
  // create the null element
  nullObj = SsidCfgMgr();
  emptyClassObjectAssigned = true;

  nullObj.emptyClassObject = true;

  int  i;
  for (i = 0; i < maxSsids; i++) {
    nullIndex(i);
  }
  readSsidmap();

  for (i = 0; i < maxSsids; i++) {
    if (ssidnamArr[i] != "??") {
      return true;
    }
  }
  return false;
}

void SsidCfgMgr::nullIndex(int idx) {
  ssidnamArr[idx] = "??";
  ssidNnnnFile[idx] = "??";
  ssidObject[idx] = nullObj;
}

boolean SsidCfgMgr::isNullObj() {
  return !this->emptyClassObject;
}

/**
   get the index for the SSID-name in the table

   return 1 to n, or -1 if not found
*/
int SsidCfgMgr::getSsidNameIndex(String ssidName) {
  int i;
  for (i = 0; i < maxSsids; i++) {
    if (ssidnamArr[i] == ssidName) {
      return i;
    }
  }
  return -1;
}

SsidCfgMgr SsidCfgMgr::getSsidNameObject(String ssidName) {
  int idx = SsidCfgMgr::getSsidNameIndex(ssidName);
  if (idx == -1) {
    return nullObj;
  }
  loadSsid(ssidName);
  return ssidObject[idx];
}

void SsidCfgMgr::reWriteLoad() {
  // re-write the order now
  writeSsidmap();
  readSsidmap();
}

String SsidCfgMgr::getFreeName() {
  int i;
  for (i = 0; i < maxSsids; i++) {
    char buffer[30];
    sprintf(buffer, "/cfgssid%d.t", i);

    if (SPIFFS.exists(buffer)) {
      continue;
    }
    return buffer; // String(buffer);
  }
  return "dummy.txt";
}

/**
   Add a new SSID object

   return true if successful
*/
boolean SsidCfgMgr::addSsid(String ssidName, String ssidPwd, String ssidChannel,
                            String useDevMacStr, String macAddr, String useDhcpStr,
                            String ipAddr, String ipGateway, String ipSubnet,
                            String portHttp, String portTcp) {  //?? add the other data fields
  int  i;
  for (i = 0; i < maxSsids; i++) {
    if (ssidnamArr[i] == "??") {
      SsidCfgMgr nuObj = SsidCfgMgr();

      nuObj.ssidName = ssidName;
      nuObj.ssidPw = ssidPwd;

      nuObj.ssidChStr = ssidChannel;

      nuObj.useDevMacStr = useDevMacStr;
      nuObj.macAddrStr = macAddr;     // 17

      nuObj.useDhcpStr = useDhcpStr;

      nuObj.ipAddrStr = ipAddr;       // 15
      nuObj.ipGatewayStr = ipGateway; // 15
      nuObj.ipSubnetStr = ipSubnet;   // 15

      nuObj.httpPortStr = portHttp;   // 5
      nuObj.httpTcpStr = portTcp;     // 5

      // store in the map
      ssidnamArr[i] = ssidName;
      ssidNnnnFile[i] = getFreeName();

      ssidObject[i] = nuObj;

      reWriteLoad();

      nuObj.save();

      return true;
    }
  }
  return false;
}

void SsidCfgMgr::deleteSsid(String name) {
  int idx = getSsidNameIndex(name);

  if (idx != -1) {
    // remove the file associated with the object
    String file2Delete = ssidNnnnFile[idx];

    if (SPIFFS.exists(file2Delete)) {
      SPIFFS.remove(file2Delete);
    }
    nullIndex(idx);
    // remove idx and then movie up the idx+ items
    int  i;
    for (i = idx; i < maxSsids - 1; i++) {
      ssidnamArr[i] = ssidnamArr[i + 1];
      ssidNnnnFile[i] = ssidNnnnFile[i + 1];
      ssidObject[i] = ssidObject[i + 1];
    }
    // the last N idx will be just nulled as it becomes a free space
    nullIndex(maxSsids - 1);
  }
  reWriteLoad();
}

void SsidCfgMgr::reOrder(String name, bool up) {
  int  i;
  for (i = 0; i < maxSsids; i++) {
    if (ssidnamArr[i] == name) {
      changeOrderInternal(i, up);
      return;
    }
  }
}

void SsidCfgMgr::changeOrderInternal(int index, bool up) {
  int swapN;
  int swapN1;

  if (up) {
    swapN = index;
    swapN1 = index - 1;
  } else {
    // down
    swapN = index + 1; // move N+1 up
    swapN1 = index;
  }
  String ssidNameTmp = ssidnamArr[swapN];
  ssidnamArr[swapN] = ssidnamArr[swapN1];
  ssidnamArr[swapN1] = ssidNameTmp;

  String ssidFileTmp =  ssidNnnnFile[swapN];
  ssidNnnnFile[swapN] = ssidNnnnFile[swapN1];
  ssidNnnnFile[swapN1] = ssidFileTmp;

  // re-write the order now
  reWriteLoad();
}

String readDataOnly(File f) {
  String inp = f.readStringUntil('\r');

  if (inp.startsWith("\n")) {
    return inp.substring(1);
  }
  return inp;
}

void SsidCfgMgr::readSsidmap() {
  int i;
  // null the table
  for ( i = 0; i < maxSsids; i++) {
    SsidCfgMgr::nullIndex(i);
  }
  // reload the table
  File f = SPIFFS.open(cfgSsidMapFilename, "r");
  for ( i = 0; i < maxSsids; i++) {
    String inp = readDataOnly(f);
    if (inp == "") {
      continue;
    }
    int plusIdx = inp.indexOf("+");
    ssidnamArr[i] = inp.substring(0, plusIdx);
    ssidNnnnFile[i] = inp.substring(plusIdx + 1);
    ssidObject[i] = nullObj;
  }
  f.close();
}
void SsidCfgMgr::writeSsidmap() {
  int i;
  File f = SPIFFS.open(cfgSsidMapFilename, "w");
  for ( i = 0; i < maxSsids; i++) {
    String ssidnam = ssidnamArr[i];
    if (ssidnam == "??") {
      continue;
    }
    ssidnam +=  "+";
    ssidnam +=  ssidNnnnFile[i];

    f.println(ssidnam);
  }
  f.close();
}

boolean SsidCfgMgr::loadSsid(String ssidNameP) {
  int idx = getSsidNameIndex(ssidNameP);

  if (idx == -1) {
    return false;
  }
  SsidCfgMgr loadObject = SsidCfgMgr();
  return loadObject.load(ssidNameP);
}

boolean SsidCfgMgr::load(String ssidNameP) {
  // the SSID data is separate from it order and as such need to
  // ensure we get the current relationship/mapping
  int idx = getSsidNameIndex(ssidNameP);

  if (idx == -1) {
    return false;
  }
  // process the file and load the data for the object
  File f1 = SPIFFS.open(ssidNnnnFile[idx], "r");

  if (!f1) {
    return false;  //?? how to handle errors
  }
  while (f1.available()) {
    String inpLine = readDataOnly(f1);

    int equalIdx = inpLine.indexOf("=");
    if (equalIdx == -1) {
      continue;
    }
    equalIdx++;

    if (inpLine.startsWith(keySsid)) {
      this->ssidName = inpLine.substring(equalIdx);
    }
    if (inpLine.startsWith(keySsidpw)) {
      this->ssidPw = inpLine.substring(equalIdx);
    }
    if (inpLine.startsWith(keySsidChannel)) {
      this->ssidChStr = inpLine.substring(equalIdx);
    }

    if (inpLine.startsWith(keyUseDevMac)) {
      this->useDevMacStr = inpLine.substring(equalIdx);
    }
    if (inpLine.startsWith(keyMac)) {
      this->macAddrStr = inpLine.substring(equalIdx);
    }
    if (inpLine.startsWith(keyHtpport)) {
      this->httpPortStr = inpLine.substring(equalIdx);
    }
    if (inpLine.startsWith(keyTcpport)) {
      this->httpTcpStr = inpLine.substring(equalIdx);
    }

    if (inpLine.startsWith(keyDhcp)) {
      this->useDhcpStr = inpLine.substring(equalIdx);
    }
    if (inpLine.startsWith(keyIpadr)) {
      this->ipAddrStr = inpLine.substring(equalIdx);
    }
    if (inpLine.startsWith(keyIpgw)) {
      this->ipGatewayStr = inpLine.substring(equalIdx);
    }
    if (inpLine.startsWith(keyIpsubnet)) {
      this->ipSubnetStr = inpLine.substring(equalIdx);
    }
  }
  f1.close();

  // for some reason the code below may only be done at
  // this position in the method, at the start of the method
  // does not work and causes exceptions
  ssidObject[idx] = *this;
  return true;
}
void SsidCfgMgr::save() {
  // the SSID data is separate from it order and as such need to
  // ensure we get the current relationship/mapping
  int idx = getSsidNameIndex(this->ssidName);

  String ssidDataFN = ssidNnnnFile[idx];

  File f1 = SPIFFS.open(ssidDataFN, "w");

  if (!f1) {
    return;  //?? how to handle errors
  }
  f1.print(keySsid);        f1.println(this->ssidName);
  f1.print(keySsidpw);      f1.println(this->ssidPw);
  f1.print(keySsidChannel); f1.println(this->ssidChStr);

  f1.print(keyUseDevMac);   f1.println(this->useDevMacStr);
  if (this->useDevMacStr == "false") {
    f1.print(keyMac);         f1.println(this->macAddrStr);
  }

  f1.print(keyDhcp);        f1.println(this->useDhcpStr);
  if (this->useDhcpStr == "false") {
    f1.print(keyIpadr);       f1.println(this->ipAddrStr);
    f1.print(keyIpgw);        f1.println(this->ipGatewayStr);
    f1.print(keyIpsubnet);    f1.println(this->ipSubnetStr);
  }

  f1.print(keyHtpport);     f1.println(this->httpPortStr);
  f1.print(keyTcpport);     f1.println(this->httpTcpStr);

  f1.close();
}
