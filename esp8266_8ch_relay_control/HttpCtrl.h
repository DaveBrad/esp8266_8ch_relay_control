#ifndef HttpCtrl_h
#define HttpCtrl_h

#include "Arduino.h"
#include "WiFiClient.h"
#include "FS.h"

class HttpCtrl {
  public:
    HttpCtrl(String ssidnameP, String ssidpwdP);

    static void httpFileUpload(WiFiClient client, String filename);

    static void respondHttp200(WiFiClient client, String contentTypeStr);
    static String getContentMIMEType(String filename);

  private:
    static void fileToClient(File f, WiFiClient client);
    static void processGenericFile(WiFiClient client, File f, int itemsN, String pairArr[][3]);

    // http response forms
    static const char* http200LN;
    static const int http200LNlen;

    static const char* httpconttype;
    static const int httpconttypelen;

    static const char* httpCTplain;
    static const char* httpCTjs;
    static const char* httpCThtm;
    static const char* httpCTcss;

    static const char* httphdrend;
    static const int httphdrendLen;

    String ssidname;
    String ssidpwd;
};

#endif
