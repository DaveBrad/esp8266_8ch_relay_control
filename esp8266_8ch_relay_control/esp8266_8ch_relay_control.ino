//#include "ArduinoBearSSL.h"
#include "Arduino.h"
#include "ESP8266WiFi.h"
#include "ESP8266WebServer.h"
#include "WiFiClient.h"
#include "Wire.h"   // for I2C
#include "FS.h"
#include "AES.h"
#include "Mcp23017.h"
#include "HttpCtrl.h"
#include "SsidCfgMgr.h"
#include "AesPrc.h"
#include "DoubleResetDetector.h"

// The following include and define lines allow for
// development of the code and provides:
// 1) an aid to upload files to the SPIFFS system on the ESP8266
//     (DevFsUpload.h)
//
// 2) the MCP23017 uses I2C communication, the emulate retains
//    the state of the GPIO pins in the ESP8266 and does not
//    allow I2C failure conditions
//
// 3) set debug message needs for DEBUG or more permanent MARK points
//    causes messages to the monitor serial port to be output
//
// for production these need to be commented out
#include "DevFsUpload.h" // NOTE: comment out for production releases
#define MCP23017_emulate

#define DEBUG
//#define DEBUGMARK // debug mark message point in code

// macro's are typically UPPERCASE, however for readability of code
// mixed case is used

// Serial.print/println macro's for Arduino debug messaging
// when DEBUG is not defined
//
#define Dprint(...)      // DEBUG print
#define Dprintln(...)    // DEBUG println
#define Dmark(...)       // DEBUGMARK is a mark -> println at marked point in code

// re-dfine macro's if the DEBUG condition is set
//
#ifdef DEBUG
#define Dprint(...)    Serial.print(__VA_ARGS__)
#define Dprintln(...)  Serial.println(__VA_ARGS__)
#endif

#ifdef DEBUGMARK
#define Dmark(...)  Serial.println(__VA_ARGS__)
#endif

// - - - - debug: other specific debug conditions
#define development
#ifdef development

//#define dbgPort81
#define dbgpwd       // debug for password control testing

#endif



// Capability: allows double reset to drop device to SoftAP mode vs station mode
//             in the event a device recovery is required
//             (rather than a factory reset)
#define DRD_TIMEOUT 10
#define DRD_ADDRESS 0

DoubleResetDetector drd(DRD_TIMEOUT, DRD_ADDRESS);

AES aes;   // not implemented yet

// factory reset password SOftAP mode only
const String softApPwdDefault  = "password1";

// device-access password as set from initial/factory configuration
char softApPwd[20];   // password for device is 16 chars max
boolean passwordSet = false;

// when 'config' dialog device-access password is correct only,
// device-access configure is permitted to be entered while
// all other requests cause a deny to device-access
//
// needs to be set for each handle page being processed
//
boolean accessPermit = false;

// file names are 8.3 format
//
// labels storage
const char* cfglabelsFilename PROGMEM = "/cfgLabel.txt";

// device-access password
const char* cfgPwdFilename PROGMEM = "/cfgPwd.txt";

// MCP23017 I2C address to use
const char* cfgMcpFilename PROGMEM = "/cfgMcpAd.txt";

// force AP restart recovery if file present
const char* cfgaccAP PROGMEM = "/cfgaccAP.txt";

// constants for processing as SoftAP
//
const char* apIpAddr PROGMEM = "192.168.4.1";
const char* apGateway PROGMEM = "192.168.4.1";
const char* apSubnet PROGMEM = "255.255.255.0";

// the relays may be labelled, storage for holding the information
// [each label may be 7 characters long]
//
// is copied from a file on reboot, but in store for optimization
// when information is repeatly accessed for webpage display
//
//......................1......2......3......4......5......6......7......8
//.....................12345671234567123456712345671234567123456712345671234567
String cfgRlyLabels = "                                                        ";
int cfgRlyLabelsLength = 8 * 7;

// key string for the data from a file
const char* keyCfgLabel  PROGMEM  = "rlylabel";

// server arrangements for access via http or tcp
//
#define httpAccessed 1
#define tcpAccessed 0

//BearSSL::WiFiServerSecure server(443);

// server objects for processing. Allocated as part of arduino-> setup()
ESP8266WebServer* server;
WiFiServer* tcpServer;

// the MCP23017 controller object (special circuit)
// interface between WeMos D1 via I2c <-> MCP23017 for relay-board
//
Mcp23017* mcp23017Obj;
int mcp23017I2CAddr;

// the dynamic storage for the MAC, IP and port values
//
uint8_t mac[6] {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

//  IP                  // SoftAP                  Web-server setting example
uint8_t ipAdr[4];       //  192.168.4.1              {192, 168, 0, 91};
uint8_t ipGateAdr[4];   //  192.168.4.1              {192, 168, 0, 1};
uint8_t ipSubnetAdr[4]; //  255.255.255.0            {255, 255, 255, 128};
int httpPort = 80;      //  8011                     {80}
int tcpPort = 81;       //  81?                      (81)

// determine at restart: is SoftAP (initial state), or as-station running
boolean runningAsStation = false;

// ------------- loop ------------
void loop() {
  // double reset detection loop requirement interface
  drd.loop();

  ArduinoOTA.handle();

  // when running as a station it is possible that a connection
  // to the router may drop, attempt to recovery from this disconnection
  if (runningAsStation) {
    boolean currentConnectedState = (WiFi.status() == WL_CONNECTED);
    while (!currentConnectedState) {
      // loop until a connection can be made, with a 15 second
      // delay between each cycle
      Dprintln("Disconnected from server, attempt recovery.");

      delay(15000);

      srtWifiSvrStation();
      currentConnectedState = (WiFi.status() == WL_CONNECTED);
    } // end while
    // as-station also supports TCP access
    delay(10);
//    tcpServerFunc();
  }
  // as-station or SoftAP handle client request
  server->handleClient();

  // sleep modes: this basicially locks up the device so not implemented
  //
  //  wifi_set_sleep_type(MODEM_SLEEP_T);
  //  WiFi.forceSleepBegin();
  //
  delay(50); // cool down processor (not mission critical application)
}
// ------------- setup ------------
void setup() {
  // initialize system wide or special capabilities
  accessPermit = false;

  WiFi.disconnect();

  // generally prepare for debug purposes
  Serial.flush();
  Serial.begin(115200);
  Serial.println();
  delay(50);

  // initialize the SPIFFS, should not fail
  if (!srtSpiffs()) {
    // app is dependent on SPIFFS so there is nothing that can be done
    system_deep_sleep(0);
  }
  // if
  // - the device has been configured then running as station
  // - configured device-access password is contained within a file
  runningAsStation = SsidCfgMgr::initSsidCfgMgr();
  passwordSet = SPIFFS.exists(cfgPwdFilename);

  // if a config access AP file is present then
  // force SoftAP after reboot
  if (SPIFFS.exists(cfgaccAP)) {
    SPIFFS.remove(cfgaccAP);
    runningAsStation = false;
  } else {
    if (drd.detectDoubleReset()) {
      // override to SoftAP due to double reset
      runningAsStation = false;
    }
  }
  //
  if (passwordSet) {
    File ff = SPIFFS.open(cfgPwdFilename, "r");
    String pwd = ff.readStringUntil('\r');
    ff.close();

    pwd.toCharArray(softApPwd, 19);
  } else {
    softApPwdDefault.toCharArray(softApPwd, 19);;
  }
  Dprint("Pwd set as: ");
  Dprintln(softApPwd);

  // - - - - - - - - - - - - - - - - - - - -
  // common pages for Station or SoftAP modes
  // - - - - - - - - - - - - - - - - - - - -
  server = new ESP8266WebServer(80);
  tcpServer = new WiFiServer(81);

#ifdef DevFsUploadInstall
  // install DevFsUpload server handle
  DevFsUploadInstall(server);
#endif

  server->on("/a401", HTTP_GET, handleA401);
  server->on("/actssid", HTTP_POST, handleActssid);
  server->on("/entdata", HTTP_POST, handleEnterData);
  server->on("/pwd", HTTP_POST, handleSetPwd);
  server->on("/ssiddata", HTTP_POST, handleSsidmap);

  // there are various communication "channels" required for this
  // app.
  // 1) WiFi the b/g/n stuff
  //   1a) as an AP (access point) mpde for initial configuration to support the
  //       server/station mode
  //   1b) as station-mode for controlling relays via a web-server arrangement
  // 2) Wire for driving the relays
  //     this allows the WeMos D1 to communicate with a MCP23017 which provides
  //     an independent 8 GPIO to a 8 relay board.
  //     (basic once the WeMos D1 is up and running, there are not enough GPIO's for
  //     8 relay boards) [A WeMos D1 mini has even less GPIO's available and baically
  //     needs an external GPIO board like MCP23017: note that I2C seems to be
  //     a more recommended communication 'channel']

  if (runningAsStation) {
    Dprint("Station mode");

    mcp23017I2CAddr = loadMcpAddress();

    server->on("/", HTTP_GET, handleRoot);
    server->on("/CONFIG", HTTP_POST, handleLabelsConfig);
    server->on("/CONFIGQUERY", HTTP_POST, handleConfigQuery);

    server->on("/cfgacc", HTTP_GET, handleInitRoot);
    server->on("/access", HTTP_POST, handleAccess);

    // all else will be handled if not found, and then the file
    // will be pulled out and found on the SPIFFS
    server->onNotFound(handleNotFound);

    // with the SPIFFS running read the config data before
    // the wifi is set up
    loadLabelsConfig();
    // prepare the MCP23017 interface
    // [there is only one MCP23017 device per WeMos D1 associated. The
    // MCP needs to be set/reset via the device configuration webpage.]
    mcp23017Obj = new Mcp23017(mcp23017I2CAddr);

#ifdef MCP23017_emulate
    mcp23017Obj->emulateMode = true;
#endif

    if (!Mcp23017::connectUpI2c()) {
      // there is nothing that can be done, but things
      // may stabilize through natural processing, so continue regardless
      Dprintln("\nfail to start I2C\n");
    }
    boolean notConnected = true;
    while (notConnected) {
      notConnected = !srtWifiSvrStation();

      if (notConnected) {
        Dprintln(": FAIL STA");
        delay(10000);
      }
    }
  } else {
    Dprint("SoftAP mode");
    loadMacAddr();

    server->on("/", HTTP_GET, handleInitRoot);
    server->onNotFound(handleNotFound);

    WiFi.mode(WIFI_AP_STA);
    boolean result = WiFi.softAP("ESPsoftAP_01", softApPwd);

    ipArrayFill(ipAdr, apIpAddr);
    IPAddress ip(ipAdr); // this 3 lines for a fixed IP-address

    ipArrayFill(ipGateAdr, apGateway);
    IPAddress gateway(ipGateAdr);

    ipArrayFill(ipSubnetAdr, apSubnet);
    IPAddress subnet(ipSubnetAdr);

    if (!WiFi.softAPConfig (ip, gateway, subnet)) {
      Dprint(": FAIL");
    } else {
      server->begin(8011);
    }
  }
  Dprintln("");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

//
// - - - - - - web/tcp server loop
//
void tcpServerFunc() {
  if (WiFiClient client = tcpServer->available()) {
    client.println("CONN okay");

    // wait for a tcp request to come through via machine-to-machine
    // interaction (debug: extend time to support human/CLI-to-machine)
    //
    int waitOnTime = 100;

#ifdef  dbgPort81
    // (debug: extend time to support human/CLI-to-machine)
    waitOnTime = 30000;
#endif

    int waitAvailableFor = 10;
    while (!client.available()) {
      delay(waitAvailableFor);
      waitOnTime -= waitAvailableFor;

      if (waitOnTime <= 0) {
        // timed out, so drop the client flat
        Dprintln("client: dropped");
        client.stop();
        return;
      }
    }
    // we have a message
    String request = client.readStringUntil('\r');
    request.toUpperCase();

    int relayOperateIndex = request.indexOf("/RLY=O");
    if (relayOperateIndex > -1) {

      processRlyOxxxxx(client, request, relayOperateIndex, tcpAccessed);
    }
    //    client
    client.flush();
  }
}
// ----------------------------------------------------------
// HTTP get, post, upload files I/F
// ----------------------------------------------------------
void handleActssid() {
  Dmark("handleActssid");

  if (server->hasArg("pwdacc")) {
    if (!checkPassword(AesPrc::dePassword(server->arg("pwdacc")))) {
      return;
    }
    boolean validBool = false;

    if (server->hasArg("type")) {
      String typeValue = server->arg("type");
      if (typeValue == "ap") {
        // request a restart will come back as SoftAP
        File f = SPIFFS.open(cfgaccAP, "w");
        if (!f) {
          // failed, nothing may be done
          // TODO should consider a HTTP error response
        } else {
          f.print("ap");
          f.close();
          validBool = true;
        }
      }
      if (typeValue == "factory") {
        validBool = true;

        // scan the file system and remove all files begining with
        // '/cfg'

        Dir dir = SPIFFS.openDir("/");
        while (dir.next()) {
          String fn = dir.fileName();

          if (fn.startsWith("/cfg")) {
            SPIFFS.remove(fn);
          }
        }
      }
      if (typeValue == "sta") {
        validBool = true;
      }
    }
    // respond to the browser
    server->send(200);
    if (validBool) {
      delay(1000);
      accessPermit = true;
      handleRestart();
    }
  }
}

/*
   Handle the device-access request for password protected activities
   by setting accessPermit if the password is valid.
*/
void handleAccess() {
  Dmark("handleAccess");

  accessPermit = false;
  WiFiClient client = server->client();

  String pwdacc_s1 = "pwdacc";
  if (server->hasArg(pwdacc_s1)) {
    if (checkPassword(AesPrc::dePassword(server->arg(pwdacc_s1)))) {
      accessPermit = true;

      HttpCtrl::respondHttp200(client, "text/plain");
      client.print("okay");
      client.stop();
      return;
    }
  }
  server->send(401, "text/plain", "401 authorization not provided 2");
}

void handleA401() {
  Dmark("handleA401");

  HttpCtrl::httpFileUpload(server->client(), "/wmos401.htm");
}
void handleInitRoot() {
  Dmark("handleInitRoot");

  if (accessPermit || !runningAsStation) {
    accessPermit = false;
    HttpCtrl::httpFileUpload(server->client(), "/wmossecr.htm");
    return;
  }
  server->send(401, "text/plain", "401 authorization not provided 1");
}

void handleSetPwd() {
  Dmark("handleSetPwd");

  accessPermit = false;

  String pwdacc_s1 = "currpwd";
  if (server->hasArg(pwdacc_s1)) {
    Dprintln("ccurrpwd");
    if (!checkPassword(AesPrc::dePassword(server->arg(pwdacc_s1)))) {
      server->send(401, "text/plain", "401 authorization not provided 3");
      return;
    }
  } else {
    server->send(401, "text/plain", "401 authorization not provided 4");
    return;
  }
  String setpwb_s1 = "setpwd";
  if (server->hasArg(setpwb_s1)) {
    passwordSet = savePassword(AesPrc::dePassword(server->arg(setpwb_s1)));
  }
  // the page will be reloaded and as such access permit show
  // remain true
  accessPermit = true;
}

boolean savePassword(String pwdstr) {
  // store this password into a pwdConfig file
  File f = SPIFFS.open(cfgPwdFilename, "w");  // overwrite
  if (!f) {
    return false;
  }
  f.println(pwdstr);
  f.close();
  return true;
}

boolean checkPassword(String pwdstr) {
  File f = SPIFFS.open(cfgPwdFilename, "r");
  if (!f) {
    return pwdstr == softApPwdDefault;
  }
  String pwdOnFile = f.readStringUntil('\r');
  f.close();

  return pwdstr == pwdOnFile;
}

// - - - - - -  web/tcp server
void handleRoot() {
  Dmark("handleRoot");

  accessPermit = false;
  HttpCtrl::httpFileUpload(server->client(), "/wmosbas2.htm");
}

void handleNotFound() {
  Dmark("handleNotFound");

  accessPermit = false;

  String path = server->uri();
  WiFiClient client = server->client();

  int relayOperateIndex = path.indexOf("/RLY=O");
  if (relayOperateIndex > -1) {
    processRlyOxxxxx(client, path, relayOperateIndex, httpAccessed);

  } else {
    HttpCtrl::httpFileUpload(client, path);
  }
  //  // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
  //  server.send(404, "text/plain", "404: Not found");
}
//
// ------------ config actions ------------
//
void handleLabelsConfig() {
  Dmark("handleLabelsConfig");

  accessPermit = false;

  if (!server->hasArg("inp")) {
    return;
  }
  cfgRlyLabels = server->arg("inp");

  // save the label in a config file for persistence
  saveLabelsConfig();

  WiFiClient client = server->client();
  HttpCtrl::respondHttp200(client, "text/plain");
  client.println(cfgRlyLabels);
  client.stop(); // without this there is a 2-3 second delay
}

void handleConfigQuery() {
  Dmark("handleConfigQuery");

  accessPermit = false;

  loadLabelsConfig();

  WiFiClient client = server->client();

  HttpCtrl::respondHttp200(client, "text/plain");
  client.println(cfgRlyLabels);
  client.stop(); // without this there is a 2-3 second delay
}
//
// ------------ relay get actions ------------
//
void processRlyOxxxxx(WiFiClient client, String request, int rlyEqualIndex, int accessKind) {

//#ifdef DevFsUploadInstall
//  delay(250);
//#endif

  //..0123456789     rlyEqualIndex points at the slash
  // '/RLY=ONn HTTP.....
  // '/RLY=OFn HTTP.....
  // '/RLY=ONAll HTTP.....
  // '/RLY=OFAll HTTP.....
  // '/RLY=OQuery HTTP.....
  //
  // get the ON or OFF setting
  char onOffQuery = request[rlyEqualIndex + 6]; // 'N', 'F', 'Q'
  char nVal = request[rlyEqualIndex + 7];   // n or 'A'

  int intOfNval =  -1;
  if (onOffQuery != 'Q') {
    if (nVal != 'A') {
      // this is the relay number to control
      intOfNval = nVal - '0';
    } else {
      intOfNval = -2;
    }
    // is only N or F  (on/off)
    if (onOffQuery == 'N') { // on
      if (intOfNval == -2) {
        mcp23017Obj->onRAll(true);
      } else {
        mcp23017Obj->onR(true, intOfNval);
      }
    }
    //............................012345
    if (onOffQuery == 'F') { // off
      if (intOfNval == -2) {
        mcp23017Obj->offRAll(true);
      } else {
        mcp23017Obj->offR(true, intOfNval);
      }
    }
  }
  // QUERY is a default path and all will go this way

  //db  Dprintln("processing... Relay get");

  //?? need to sprint this stuff and send as a whole
  if (accessKind == httpAccessed) {
    HttpCtrl::respondHttp200(client, "text/plain");
  } else {
    client.print("ACK>");
  }
  // if the line is a GET /RLY=xxxxxxx HTTP/1.1 then its a
  // relay action and no files need to be transferred, only the
  // relays need to be activated on or off
  String i2cOkay = mcp23017Obj->writeRelaysA();

  client.print(mcp23017Obj->gpioAToBinaryString());
  if (intOfNval > -1) {
    // single relay impact
    client.print("=R");
    client.print(intOfNval);
  }
  if (intOfNval == -2) {
    client.print("=A");
    client.print(onOffQuery);  // 'N' of 'F'
  }
  client.println("");
  if (i2cOkay == "") {
    client.print("okay");
  } else {
    client.println(i2cOkay);
    client.print("not okay");
  }
  if (accessKind == tcpAccessed) {
    delay(100); // if not provided the buffer is not complete for tcp access response
  }
  client.flush();
  client.stop(); // without this there is a 2-3 second delay

  //  // if the line is a GET /RLY=xxxxxxx HTTP/1.1 then its a
  //  // relay action and no files need to be transferred, only the
  //  // relays need to be activated on or off
  //  mcp23017Obj->writeRelaysA();
}

// ----------------------------------------------------------
// WiFI communication
// ----------------------------------------------------------
/**
  Process an IP string, validate each field and then copy if valid.

  Return true if valid
*/
boolean ipArrayFill(uint8_t arrVar[], String ipAddrStr) {
  char buf[50];
  const char delimC[2] = ".";

  ipAddrStr.toCharArray(buf, 51);
  char *p = strtok(buf, delimC);

  for (int i = 0; i < sizeof(arrVar); i++) {
    int fieldValue = atoi(p);
    if (fieldValue < 0 || fieldValue > 255) {
      return false;
    }
    arrVar[i] = (uint8_t)(fieldValue & 0xff);
    p = strtok(NULL, delimC);
  }
  return true;
}
void macArrayFill(uint8_t arrVar[], String & macStr) {
  // prepare for processing by strtok
  const char cColon[2] = ":";
  char buf[50];
  macStr.toCharArray(buf, 50);

  char *p = strtok(buf, cColon);
  for (int i = 0; i < 6; i++) {
    // convert to hex-byte
    long int n = strtol(p, NULL, 16); // n has numeric value
    byte extract = (char)(n & 0xff);
    mac[i] = (uint8_t)(extract);

    // next hex value of MAC
    p = strtok(NULL, cColon);
  }
}

/**
  Copy contents of an array source to a destination array.

  To be used when all data has been validated and then as a collection
  stored into the real-storage.
*/
void ipCopyArray(uint8_t arrVarSrc[], uint8_t arrVarDst[]) {
  for (int i = 0; i < sizeof(arrVarSrc); i++) {
    arrVarDst[i] = arrVarSrc[i];
  }
}

boolean srtWifiSvrStation() {
  int j;
  int i;
  Dprintln("");

  WiFi.disconnect();
  // provide a time for the WiFi to do its thing before
  // scanning for networks (had issues where it appears
  // scan was incomplete)
  delay(100);

  // wifi router connection
  int netFoundCount = WiFi.scanNetworks();

  Dprint("Networks found: ");
  if (netFoundCount == 0) {
    Dprintln("none");
    return false;
  }
  Dprint(netFoundCount);
  //
  int ssidIndexToConnect = -1;

  for (int i = 0; i < maxSsids; ++i) {
    String lookupSsid = SsidCfgMgr::ssidnamArr[i];
    if (lookupSsid == "??") {
      continue;
    }
    SsidCfgMgr ssidObj = SsidCfgMgr::getSsidNameObject(lookupSsid);

    Dprintln("");
    Dprint("Find SSID " + lookupSsid + ":" + ssidObj.ssidChStr);
    delay(10);

    if (ssidObj.ssidChStr == "auto") {
      // this is an auto channel and visible SSID
      Dprintln(":visible");

      for (int j = 0; j < netFoundCount; ++j) {
        if (WiFi.SSID(j) == lookupSsid) {
          ssidIndexToConnect = i;

          // drop out of both loops by making the values
          // higher than the limit
          j = netFoundCount;
          i = maxSsids;
        }
      }
    } else {
      // is a potential invisible SSID
      Dprintln(":invisible");

      boolean connInvisible = connectToNet(i, true);
      if (connInvisible) {
        return true;
      }
    }
  }
  // found the network we wish to connect to, or NOT
  if (ssidIndexToConnect == -1) {
    return false;
  }
  return connectToNet(i, false); // visible network
}

boolean connectToNet(int ssidIndexToConnect, boolean connInvisible) {
  // begin the connection attempt, but first copy the ssid information and IP information
  // to the appropriate places
  SsidCfgMgr ssidObj = SsidCfgMgr::ssidObject[ssidIndexToConnect];

  if (ssidObj.useDevMacStr == "true") {
    Dprint("Device");
    WiFi.macAddress(mac);
  } else {
    Dprint("Defined");
    macArrayFill(mac, ssidObj.macAddrStr);
    wifi_set_macaddr(STATION_IF, mac);
  }
  Dprint(" MAC  ");
  Dprint(mac[0], HEX); Dprint(":");
  Dprint(mac[1], HEX); Dprint(":");
  Dprint(mac[2], HEX); Dprint(":");
  Dprint(mac[3], HEX); Dprint(":");
  Dprint(mac[4], HEX); Dprint(":");
  Dprintln(mac[5], HEX);

  char bufHttp[10];
  ssidObj.httpPortStr.toCharArray(bufHttp, 6);
  httpPort = atoi(bufHttp);

  char bufTcp[10];
  ssidObj.httpTcpStr.toCharArray(bufTcp, 6);
  tcpPort = atoi(bufTcp);

  if (ssidObj.useDhcpStr != "true") {
    // configure the IP addresses explicitly
    ipArrayFill(ipAdr, ssidObj.ipAddrStr);
    ipArrayFill(ipGateAdr, ssidObj.ipGatewayStr);
    ipArrayFill(ipSubnetAdr, ssidObj.ipSubnetStr);
    // - - - -
    // connect the web server
    // loadIpConfig();
    IPAddress ip(ipAdr); // this 3 lines for a fix IP-address
    IPAddress gateway(ipGateAdr);
    IPAddress subnet(ipSubnetAdr);

    WiFi.config(ip, gateway, subnet);
  }
  delay(10);

  WiFi.mode(WIFI_STA);
  delay(10);
  char* ssidNamePart = ssidObj.getName();

  Dprint(" --Attempting connect- ");
  if (ssidObj.ssidChStr != "auto") {
    char buffer[10];
    ssidObj.ssidChStr.toCharArray(buffer, 8);
    int chNum = atoi(buffer);

    WiFi.begin(ssidNamePart, ssidObj.getPwd(), chNum);
  } else {
    WiFi.begin(ssidNamePart, ssidObj.getPwd());
  }
  delay(10);

  // connection requires handshake protocol, so until it completes
  // note the connection protocol progress
  //
  // should connect within 10-30 seconds so loop appropriately
  //  Dprint("Connecting ");
  //  Dprintln(ssidNamePart);

  int delayTime = 500;

  int timeToConnect;
  if (connInvisible) {
    // invisible should connect within 10 seconds, otherwise assume not
    // there (lower the risk for password transmits)
    timeToConnect = 10000;
  } else {
    timeToConnect = 30000;
  }
  while (WiFi.status() != WL_CONNECTED) {
    delay(delayTime);
    Dprint(".");
    timeToConnect -= delayTime;

    if (timeToConnect <= 0) {
      return false;
    }
  }
  Dprint("\nConnected IP: ");
  Dprintln(WiFi.localIP());

  // start LISTENING on the HTTP and TCP port for processing
  server->begin(httpPort);
  delay(10);
  tcpServer->begin(tcpPort);
  delay(10);

  return true;
}
// ----------------------------------------------------------
// SPIFFS: flash file system
//         - htm, css, js,... files
//         - any other files
// ----------------------------------------------------------
boolean srtSpiffs() {
  Dprint("SPIFFS mount");
  if (!SPIFFS.begin()) {
    Dprintln(": FAILED");
    return false;
  }
  delay(50);
  Dprintln("ed.");
  return true;
}
// ----------------------------------------------------------
// Config file information
// ----------------------------------------------------------

// - - - - - - labels config

void loadLabelsConfig() {
  WiFiClient client = server->client();
  client.setNoDelay(1);

  if (!SPIFFS.exists(cfglabelsFilename)) {
    // the file does not exist so create it with the default
    saveLabelsConfig();
  }
  // process the file
  File f = SPIFFS.open(cfglabelsFilename, "r");
  if (!f) {
    client.print("File on server fail: ");
    client.println(cfglabelsFilename);
    return;
  }
  while (f.available()) {
    setConfigVariable(f.readStringUntil('\r'));
  }
  f.close();
}

void saveLabelsConfig() {
  WiFiClient client = server->client();
  // process the file
  File f = SPIFFS.open(cfglabelsFilename, "w");
  if (!f) {
    client.print("File on server fail: ");
    client.println(cfglabelsFilename);
    return;
  }
  writeCfgLine(f, keyCfgLabel, cfgRlyLabels);
  f.close();
}

void writeCfgLine(File f, String key, String data) {
  // key=data
  // if data has a '=' character, then reading the data depends on
  // from the first index of '=' character
  f.print(key);
  f.print("=");
  f.println(data);
}

/**
  Set configuration variables appropriately based on the
  'key=data' configDara content.

  The associations are hardcoded as ESP8266 does not have
  associative tables.
*/
void setConfigVariable(String configData) {
  // get the key for the data
  int firstEqIdx = configData.indexOf("=");

  String key = configData.substring(0, firstEqIdx);
  key.trim();
  String data = configData.substring(firstEqIdx + 1);

  if (key == keyCfgLabel) {
    cfgRlyLabels = data;
  }
}
//
// - - - - MCP23017 address config
//
byte loadMcpAddress() {
  File f = SPIFFS.open(cfgMcpFilename, "r");
  if (!f) {
    return 0xFF;
  }
  String line = f.readStringUntil('\r');
  char buf[10];
  line.toCharArray(buf, 5);

  long addr = strtoul(buf, NULL, 16);
  byte addrHex = addr & 0xff;
  f.close();
  return addrHex;
}

void saveMcpAddr(String mcpAdr) {
  File f = SPIFFS.open(cfgMcpFilename, "w");
  if (!f) {
    // nothing can be done
    return;
  }
  f.println(mcpAdr);
  f.close();
}
//
// - - - - MAC address config
//
void loadMacAddr() {
  // the board has a unique MAC address but this may be overridden
  // as part of a users setting
  WiFi.macAddress(mac);
  //
  // use boards mac addr
  Dprintln("Using board MAC");
  Dprint(mac[0], HEX); Dprint(":");
  Dprint(mac[1], HEX); Dprint(":");
  Dprint(mac[2], HEX); Dprint(":");
  Dprint(mac[3], HEX); Dprint(":");
  Dprint(mac[4], HEX); Dprint(":");
  Dprintln(mac[5], HEX);
}

// - - - - - -
void validateIpFld(char* argKey,  String & dataStore, bool & validBool) {
  if (!server->hasArg(argKey)) {
    validBool = false;
    return;
  }
  dataStore = server->arg(argKey);

  uint8_t tmpArr[4];
  validBool = ipArrayFill(tmpArr, dataStore);
}

void validatePortFld(char*  argKey,  String & dataStore, bool & validBool) {
  if (!server->hasArg(argKey)) {
    validBool = false;
    return;
  }
  dataStore = server->arg(argKey);

  char dataChar[100];
  dataStore.toCharArray(dataChar, 100);

  int portTest = atoi(dataChar);
  validBool = !(portTest < 0 || portTest > 65535);
}
boolean validateSsid(char*  argKey, String & dataStore, bool & validBool) {
  boolean returnNewSsid = server->hasArg("ssidnuname");

  if (!server->hasArg(argKey)) {
    validBool = false;
    return returnNewSsid;
  }
  dataStore = server->arg(argKey);
  validBool = true;
  return returnNewSsid;
}
void handleEnterData() {
  bool fail = false;

  if (server->hasArg("mcpadr")) {
    // is a set the MCP address for this device request
    saveMcpAddr(server->arg("mcpadr"));
  } else {
    // assume a SSID data entry
    String macAddr, ipAdrLocal, ipGwayLocal,  ipSubnetLocal,  httpPortLocal,  tcpPortLocal,
           ssidnameP, ssidpwdP, ssidChannel, useDhcpStr, useDevMacStr;

    // this could be made to be CONST but would take more RAM in a static
    // manner, rather than a run time temporary heap arrangement
    bool useDevMacFld = false;
    bool macAddrFld = false;

    bool useDhcpFld = false;
    bool ipAdrFld = false;
    bool ipGwayFld = false;
    bool ipSubnetFld = false;

    bool httpPortFld = false;
    bool tcpPortFld = false;
    bool ssidnameFld = false;
    bool ssidpwdFld = false;
    bool ssidChannelFld = false;

    bool newSsid = true;
    bool dataValid = true;

    char* useDhcp_s1 = "dhcp";
    useDhcpFld = server->hasArg(useDhcp_s1);
    if (useDhcpFld) {
      useDhcpStr = server->arg(useDhcp_s1);

      if (useDhcpStr == "true") {
        ipAdrFld = true;
        ipGwayFld = true;
        ipSubnetFld = true;
      } else {
        validateIpFld("ipAdr", ipAdrLocal, ipAdrFld);
        validateIpFld("ipGway", ipGwayLocal, ipGwayFld);
        validateIpFld("ipSubnet", ipSubnetLocal, ipSubnetFld);
      }
    }
    validatePortFld("portHttp", httpPortLocal, httpPortFld);
    validatePortFld("portTcp", tcpPortLocal, tcpPortFld);

    // may be NEW or CHANGE
    if (validateSsid("ssidnuname",  ssidnameP, ssidnameFld)) {
      newSsid = true;
      validateSsid("ssidpwd",  ssidpwdP, ssidpwdFld);
    } else {
      // could be a change requirement
      validateSsid("ssidchgname",  ssidnameP, ssidpwdFld);
      newSsid = false;
    }

    char* mac_s1 = "mac";
    useDevMacFld = server->hasArg(mac_s1);
    if (useDevMacFld) {
      useDevMacStr = server->arg(mac_s1);

      if (useDevMacStr == "true") {
        macAddrFld = true;
      } else {
        char* macadr_s1 = "macAdr";
        macAddrFld = server->hasArg(macadr_s1);
        if (macAddrFld) {
          macAddr = server->arg(macadr_s1);
        }
      }
    }

    char* ssidch_s1 = "ssidch";
    ssidChannelFld = server->hasArg(ssidch_s1);
    if (ssidChannelFld) {
      ssidChannel = server->arg(ssidch_s1);

      if (ssidChannel == "auto") {
        ssidChannelFld = true;
      } else {
        char buf[20];
        ssidChannel.toCharArray(buf, 5);

        // returns 0 if not valid, as is 0 is not valid too
        int num = atoi(buf);
        ssidChannelFld = (num > 0 && num < 15);
      }
    }
    // all the fields need to have been set
    dataValid = dataValid && macAddrFld && ipAdrFld && ipGwayFld && ipSubnetFld &&
                httpPortFld && tcpPortFld && ssidnameFld && ssidpwdFld && ssidChannelFld &&
                useDhcpFld && useDevMacFld;

    Dprintln("");
    Dprint("dataValid "); Dprintln(dataValid);

    Dprint("useDevMacFld "); Dprintln(useDevMacFld);
    Dprint("macAddrFld "); Dprintln(macAddrFld);

    Dprint("useDhcpFld "); Dprintln(useDhcpFld);
    Dprint("ipAdrFld "); Dprintln(ipAdrFld);
    Dprint("ipGwayFld "); Dprintln(ipGwayFld);
    Dprint("ipSubnetFld "); Dprintln(ipSubnetFld);

    Dprint("ssidnameFld "); Dprintln(ssidnameFld);
    Dprint("ssidpwdFld "); Dprintln(ssidpwdFld);
    Dprint("ssidChannelFld "); Dprintln(ssidChannelFld);

    Dprintln(httpPortFld);
    Dprintln(tcpPortFld);

    if (dataValid) {
      Dprint("Data valid - ");
      // process the data in to the FLASH SPIFFS files
      if (newSsid) {
        Dprintln("new");
        fail = !(ssidProcessNew( ssidnameP, ssidpwdP, ssidChannel,
                                 useDevMacStr, macAddr,
                                 useDhcpStr, ipAdrLocal, ipGwayLocal, ipSubnetLocal,
                                 httpPortLocal, tcpPortLocal));
      } else {
        Dprintln("change");
        // change the SSID information
        fail = !(ssidProcessChg( ssidnameP, ssidpwdP, ssidChannel, useDevMacStr, macAddr,
                                 useDhcpStr, ipAdrLocal, ipGwayLocal, ipSubnetLocal,
                                 httpPortLocal, tcpPortLocal));
      }
    } else {
      fail = true;
    }
  }
  // confirm processing
  WiFiClient client = server->client();

  HttpCtrl::respondHttp200(client, "text/plain");
  if (fail) {
    client.print("not okay");
  } else {
    client.print("okay");
    delay(500);
  }
  client.flush();
  client.stop();
}

boolean ssidProcessNew( String ssidnameP, String ssidPwdP, String ssidChannel,
                        String useDevMacStr, String macAddr,
                        String useDhcpStr,
                        String ipAdrLocal, String ipGwayLocal, String ipSubnetLocal,
                        String httpPortLocal, String tcpPortLocal) {

  return SsidCfgMgr::addSsid( ssidnameP, ssidPwdP, ssidChannel,
                              useDevMacStr, macAddr, useDhcpStr,
                              ipAdrLocal, ipGwayLocal, ipSubnetLocal,
                              httpPortLocal, tcpPortLocal);
}

boolean ssidProcessChg( String ssidnameP, String ssidPwdP,  String ssidChannel,
                        String useDevMacStr, String macAddr,
                        String useDhcpStr,
                        String ipAdrLocal, String ipGwayLocal, String ipSubnetLocal,
                        String httpPortLocal, String tcpPortLocal) {

  SsidCfgMgr updateObject = SsidCfgMgr::getSsidNameObject(ssidnameP);


  updateObject.useDevMacStr = useDevMacStr;
  updateObject.macAddrStr = macAddr;

  updateObject.ssidChStr = ssidChannel;

  updateObject.useDhcpStr = useDhcpStr;
  updateObject.ipAddrStr = ipAdrLocal;
  updateObject.ipGatewayStr = ipGwayLocal;
  updateObject.ipSubnetStr = ipSubnetLocal;

  updateObject.httpPortStr = httpPortLocal;
  updateObject.httpTcpStr = tcpPortLocal;

  updateObject.save();

  return true;
}

// - - -  - - - utility
void handleRestart() {
  Dmark("handleRestart");

  //  pinMode(D2, OUTPUT);  // GPIO 16
  //  digitalWrite(0, 0);

  WiFi.disconnect();
  delay(500);

  // this code will not work after a board has been downloaded via the
  // USB port (known ESP8266 issue).
  //
  // NB: reset or power-on for this code to be functional
  //
  pinMode(0, OUTPUT);
  digitalWrite(0, 1);

  pinMode(2, OUTPUT);
  digitalWrite(2, 1);

  pinMode(15, OUTPUT);
  digitalWrite(15, 0);

  delay(50);
  ESP.restart();
}

void handleSsidmap() {
  Dmark("handleSsidmap");

  // there are a number of actions which fall into 2 categories
  // 1) query data for ssid
  // 2) remove, up or down an ssid item

  char* keyName = "ssidName";
  char* keyAction = "ssidact";

  WiFiClient client = server->client();
  HttpCtrl::respondHttp200(client, "text/plain");

  boolean error = true;

  if (server->hasArg(keyName) && server->hasArg(keyAction)) {
    String ssiddataFor = server->arg(keyName);
    String actionStr = server->arg(keyAction);

    SsidCfgMgr ssidObj = SsidCfgMgr::getSsidNameObject(ssiddataFor);

    if (!ssidObj.isNullObj()) {
      if (actionStr == "query") {
        if (querySsid(client, ssidObj)) {
          return;
        }
      }
      if (actionStr == "remove") {
        SsidCfgMgr::deleteSsid(ssiddataFor);
        error = false;
      }
      if (actionStr == "up") {
        SsidCfgMgr::reOrder(ssiddataFor, true);
        error = false;
      }
      if (actionStr == "down" ) {
        SsidCfgMgr::reOrder(ssiddataFor, false);
        error = false;
      }
    }
  }
  String respOK = "";
  if (error) {
    respOK = "not ";
  }
  respOK += "okay";

  client.print(respOK);
  client.flush();
  client.stop();
}

boolean querySsid(WiFiClient client, SsidCfgMgr ssidObj) {
  // gather the data and send back in a response
  String response = "ssidch=";
  if (ssidObj.ssidChStr == "-1") {
    response += "auto";
  } else {
    response +=  ssidObj.ssidChStr;
  }
  response += "&mac=" + ssidObj.useDevMacStr;
  if (ssidObj.useDevMacStr == "false") {
    response += "&macadr=" + ssidObj.macAddrStr;
  }
  response += "&http=" + ssidObj.httpPortStr;
  response += "&tcp=" + ssidObj.httpTcpStr;

  response += "&dhcp=" + ssidObj.useDhcpStr;
  if (ssidObj.useDhcpStr == "false") {
    response += "&ipadr=" + ssidObj.ipAddrStr;
    response += "&ipgw=" + ssidObj.ipGatewayStr;
    response += "&ipsn=" + ssidObj.ipSubnetStr;
  }
  client.print(response);
  client.flush();
  client.stop();

  return true;
}
