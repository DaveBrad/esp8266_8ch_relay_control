#include "HttpCtrl.h"
#include "Arduino.h"
#include "WiFiClient.h"
#include "Mcp23017.h"
#include "FS.h"
#include "SsidCfgMgr.h"

extern uint8_t mac[6];
extern bool passwordSet;
extern Mcp23017* mcp23017Obj;
extern String cfgRlyLabels;
extern int mcp23017I2CAddr;
extern String ssidnamArr;

/**
   HttpCtrl

   Module processes the http code response and such
*/

// http response forms
// . . . . . . . . . . . 1234567890123456 7 89
const char* HttpCtrl::http200LN PROGMEM = "HTTP/1.1 200 OK\r\n";
const int HttpCtrl::http200LNlen = 17;

// . . . . . . . . . .................... . . 12345678901234567890
const char* HttpCtrl::httpconttype PROGMEM = "Content-Type: ";
const int HttpCtrl::httpconttypelen = 14;

const char* HttpCtrl::httpCTplain PROGMEM = "plain\r\n";
const char* HttpCtrl::httpCTjs PROGMEM = "javascript\r\n";
const char* HttpCtrl::httpCThtm PROGMEM = "html\r\n";
const char* HttpCtrl::httpCTcss PROGMEM = "css\r\n";

const char* HttpCtrl::httphdrend PROGMEM = "\r\n";
const int HttpCtrl::httphdrendLen  = 2;

// - - - - - http responder - - - - -
void  HttpCtrl::httpFileUpload(WiFiClient client, String filename) {
  // the http header response
  String contentTypeStr = getContentMIMEType(filename);
  respondHttp200(client, contentTypeStr);

  // process the file and cause download
  File f = SPIFFS.open(filename, "r");
  if (!f) {
    client.print("File on server fail: ");
    client.println(filename);
  } else {
    if (filename.endsWith(".js")) {
      // the wmosbas2.js file has special information that needs to be
      // preset as part of the upload process
      if (filename == "/wmosbas2.js") {
        const int pairItems = 2;

        String pairArr[pairItems][3] = {
          {"var gpioStatesA", mcp23017Obj->gpioAToBinaryString(), "string"},
          {"var relayLabelA", cfgRlyLabels, "string"}
        };
        HttpCtrl::processGenericFile(client, f, pairItems, pairArr);

      } else if (filename == "/wmossecr.js") {
        const int pairItems = 1;
        //
        String pwdstr = passwordSet ? "************" : "";

        // store the overwrite settings
        String pairArr[pairItems][3] = {
          {"var nopwd", pwdstr, "string"}
        };

        HttpCtrl::processGenericFile(client, f, pairItems, pairArr);

      } else if (filename == "/wmossid.js") {

        // get the list of known SSID names
        String ssidListStr = "";
        int i;
        for (i = 0; i < maxSsids; i++) {

          String ssidnam = SsidCfgMgr::ssidnamArr[i];

          if (ssidnam == "??") {
            continue;
          }
          ssidListStr += ",\"";
          ssidListStr += ssidnam;
          ssidListStr += "\"";
        }
        if (ssidListStr != "") {
          // remove leading comma
          ssidListStr = ssidListStr.substring(1);
        }
        char buf[320];
        ssidListStr.toCharArray(buf, 319);

        const int pairItems = 1;
        // extract the MAC address and overwrite into the js file variables
        char buffer[320]; //?? have issue for  changes in maxSsids
        sprintf(buffer, "[%s]", buf);

        // store the overwrite settings
        String pairArr[pairItems][3] = {
          {"var g_orderSsidArr", buffer, "raw"}
        };
        HttpCtrl::processGenericFile(client, f, pairItems, pairArr);

      } else if (filename == "/wmosseda.js") {
        const int pairItems = 2;

        // get the MCP23017 address
        char bufMcp[6] = "-1";
        if(mcp23017I2CAddr != -1){
          const char* mcpFormat = "%0.2x";
          sprintf(bufMcp, mcpFormat, mcp23017I2CAddr);
        }       
        // extract the MAC address and overwrite into the js file variables
        const char* macFormat = "%0.2x:%0.2x:%0.2x:%0.2x:%0.2x:%0.2x";
        
        char buffer[50];
        sprintf(buffer, macFormat,
                (byte)mac[0], (byte)mac[1], (byte)mac[2],
                (byte)mac[3], (byte)mac[4], (byte)mac[5]);

        // store the overwrite settings
        String pairArr[pairItems][3] = {
          {"var mcpadr", bufMcp, "string"},
          {"var macAddr", buffer, "string"}          
        };
        HttpCtrl::processGenericFile(client, f, pairItems, pairArr);

      } else {
        fileToClient(f, client);
      }
    } else {
      fileToClient(f, client);
    }
    f.close();
  }
  client.flush();
  client.stop(); // without this there is a 2-3 second delay
}
void  HttpCtrl::processGenericFile(WiFiClient client, File f, int itemsN, String pairArr[][3]) {   //String startWithArr[], String contentArr[]) {
  // looking for leading lines containing the startWithArr and needing to be changed
  //
  // assumption is the order of lines as in the file
  //
  boolean allDoneChanges = false;
  int item = 0;

  String startsWithN = pairArr[item][0];
  String contentItem = pairArr[item][1];
  String format = pairArr[item][2];

  while (f.available() && !allDoneChanges) {
    // read to newline (if coding on non Newline line separator platform
    // this will not work too good)

    // reading to \n means the startsWith may be used to test for the
    // variable code, making the 1st char what was expect. Otherwise
    // there is the potential for a floating (leading) \n if we read to \r.
    //
    // Using a indexOf would process the whole line of characters
    // rather than failing at the 1st char on the line as with startsWith
    //
    String testStr = f.readStringUntil('\n');
    
    boolean found = testStr.startsWith(startsWithN);
    if (found) {
      // replace this out
      if (format == "raw") {
        // raw format
        testStr = startsWithN + " = " + contentItem + ";";
      } else {
        testStr = startsWithN + " = \"" + contentItem + "\";";
      }
      // advance to the next item
      item++;
      if (item < itemsN) {
        // still have items in the queue to process
        startsWithN = pairArr[item][0];
        contentItem = pairArr[item][1];
      } else {
        // all done
        allDoneChanges = true;
      }
    } // end if found
    client.println(testStr);
  } // end while
  //
  // the remainder of the file may be processed out to the client
  fileToClient(f, client);
}

void  HttpCtrl::fileToClient(File f, WiFiClient client) {
  // output the file to the  client

  // the File f variable contains any position pointer and processing
  // will be from that point
  int bufferSize = 1460;
  char buffer[bufferSize];

  while (f.available()) {
    // reading line-by-line is small buffer arragement as well as
    // looking at the contents of the each line (too slow), just transfer the
    // file directly up-wards0
    int readSize = f.readBytes(buffer, bufferSize);
    client.write(buffer, readSize);
  }
}

void  HttpCtrl::respondHttp200(WiFiClient client, String contentTypeStr) {
  // the http header response
  client.write(HttpCtrl::http200LN, HttpCtrl::http200LNlen);
  client.write(HttpCtrl::httpconttype, HttpCtrl::httpconttypelen);
  client.println(contentTypeStr);
  client.write(HttpCtrl::httphdrend, HttpCtrl::httphdrendLen);
}

/**
   Get the MIME type for the most common files, or plain if not
   known.
*/
String HttpCtrl::getContentMIMEType(String filename) {
  if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".bmp")) return "image/bmp";
   else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}
