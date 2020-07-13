/*
 *  WifiInterface.h
 * 
 *  This file is part of CommandStation.
 *
 *  CommandStation is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  CommandStation is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CommandStation.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef COMMANDSTATION_COMMINTERFACE_WIFI_WIFIINTERFACE_H_
#define COMMANDSTATION_COMMINTERFACE_WIFI_WIFIINTERFACE_H_

#include "../CommInterface.h"
#include "MemStream.h"

#include <Arduino.h>
#include <avr/pgmspace.h>

#if defined(ARDUINO_AVR_UNO)
#include <SoftwareSerial.h>
#else
#include <HardwareSerial.h>
#endif

class WifiInterface : public CommInterface
{

public:
#if defined(ARDUINO_AVR_UNO)
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
  int connectionId = 0;
  static const byte MAX_WIFI_BUFFER = 250;
  static byte buffer[MAX_WIFI_BUFFER];
  static MemStream streamer;

protected:
#if defined(ARDUINO_AVR_UNO)
  SoftwareSerial &wifiStream;
#else
  HardwareSerial &wifiStream;
#endif
};

#endif  // COMMANDSTATION_COMMINTERFACE_WIFI_WIFIINTERFACE_H_
