
#ifndef WifiInterface_h
#define WifiInterface_h
#include "..\CommInterface\CommInterface.h"
#include "MemStream.h"
#include <Arduino.h>
#include <avr/pgmspace.h>
#ifdef defined(ARDUINO_AVR_UNO)
#include <SoftwareSerial.h>
#else
#include <HardwareSerial.h>
#endif

class WifiInterface : public CommInterface
{

public:
#ifdef defined(ARDUINO_AVR_UNO)
  WifiInterface(SoftwareSerial &wifiStream, const __FlashStringHelper *SSSid, const __FlashStringHelper *password, const __FlashStringHelper *hostname, const __FlashStringHelper *servername, int port);
#else
  WifiInterface(HardwareSerial &wifiStream, const __FlashStringHelper *SSSid, const __FlashStringHelper *password, const __FlashStringHelper *hostname, const __FlashStringHelper *servername, int port);
#endif

  void process();
  void showConfiguration();
  void showInitInfo();
  void send(const char *buf);
  Stream *getStream() { return &wifiStream; }

private:
  bool setup2(const __FlashStringHelper *SSSid, const __FlashStringHelper *password, const __FlashStringHelper *hostname, const __FlashStringHelper *servername, int port);
  bool checkForOK(const unsigned int timeout, const char *waitfor, bool echo);
  bool isHTML();
  bool connected = false;
  byte loopstate = 0;
  int datalength = 0;
  int connectionId;
  static const byte MAX_WIFI_BUFFER = 250;
  char buffer[MAX_WIFI_BUFFER];
  MemStream streamer;

protected:
#ifdef defined(ARDUINO_AVR_UNO)
  SoftwareSerial &wifiStream;
#else
  HardwareSerial &wifiStream;
#endif
};

#endif
