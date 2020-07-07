
#ifndef WifiInterface_h
#define WifiInterface_h
#include "..\CommInterface\DCCEXParser.h"
#include "MemStream.h"
#include <Arduino.h>
#include <avr/pgmspace.h>

class WifiInterface {

 public:
    static void setup(Stream & wifiStream, const __FlashStringHelper* SSSid, const __FlashStringHelper* password, const __FlashStringHelper* hostname, const __FlashStringHelper* servername, int port);
    static void loop(Stream & wifiStream);
    
  private:
    
    static DCCEXParser parser;
    static bool setup2(Stream & wifiStream, const __FlashStringHelper* SSSid, const __FlashStringHelper* password, const __FlashStringHelper* hostname, const __FlashStringHelper* servername, int port);
    static bool checkForOK(Stream & wifiStream, const unsigned int timeout, const char* waitfor, bool echo);
    static bool isHTML();
    static bool connected;
    static byte loopstate;
    static int  datalength;
    static int connectionId;
    static const byte MAX_WIFI_BUFFER=250;
    static char buffer[MAX_WIFI_BUFFER];
    static MemStream  streamer;
};

#endif
