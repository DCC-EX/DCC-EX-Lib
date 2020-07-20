/*
 *  WifiInterface.cpp
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

#include "WifiInterface.h"

#include "../CommManager.h"
#include "../DCCEXParser.h"
#include "DIAG.h"
#include "HTTPParser.h"
#include "StringFormatter.h"
#include "WiThrottle.h"

#include <string.h>
const char PROGMEM READY_SEARCH[] = "ready";
const char PROGMEM OK_SEARCH[] = "OK";
const char PROGMEM END_DETAIL_SEARCH[] = "correct flash map";
const char PROGMEM PROMPT_SEARCH[] = ">";
const char PROGMEM SEND_OK_SEARCH[] = "SEND OK";
const char PROGMEM WIFI_AUTO_CONNECT_SEARCH[] = "WIFI CONNECTED\r\nWIFI GOT IP";
bool sending = false;

char WifiInterface::buffer[MAX_WIFI_BUFFER];
MemStream WifiInterface::streamer(buffer, sizeof(buffer));

#if defined(ARDUINO_AVR_UNO)
WifiInterface::WifiInterface(SoftwareSerial &wifiSerial, const __FlashStringHelper *SSid, const __FlashStringHelper *password, const __FlashStringHelper *hostname, const __FlashStringHelper *servername, int port) : wifiStream(wifiSerial)
{

  DIAG(F("\n++++++ Wifi Setup In Progress Using SW Serial ++++++++\n"));

  connected = setup2(SSid, password, hostname, servername, port);
  wifiStream.begin(115200);
  wifiStream.flush();

  // TODO calloc the buffer and streamer and parser etc
  DIAG(F("\n++++++ Wifi Setup %S ++++++++\n"), connected ? F("OK") : F("FAILED"));
}
#else
WifiInterface::WifiInterface(HardwareSerial &wifiSerial, const __FlashStringHelper *SSid, const __FlashStringHelper *password, const __FlashStringHelper *hostname, const __FlashStringHelper *servername, int port) : wifiStream(wifiSerial)
{

  DIAG(F("\n++++++ Wifi Setup In Progress Using HW Serial ++++++++\n"));
  wifiStream.begin(115200);
  wifiStream.flush();
  //MemStream streamer(buffer, sizeof(buffer));
  connected = setup2(SSid, password, hostname, servername, port);

  // TODO calloc the buffer and streamer and parser etc
  DIAG(F("\n++++++ Wifi Setup %S ++++++++\n"), connected ? F("OK") : F("FAILED"));
}
#endif

bool WifiInterface::setup2(const __FlashStringHelper *SSid, const __FlashStringHelper *password, const __FlashStringHelper *hostname, const __FlashStringHelper *servername, int port)
{
  delay(1000);

  StringFormatter::send(wifiStream, F("AT+RST\r\n")); // reset module
  //checkForOK(1000, OK_SEARCH, true);

  //checkForOK(5000, END_DETAIL_SEARCH, true); // Show startup but ignore unreadable upto ready

  if (!checkForOK(5000, READY_SEARCH, false))
    return false;

  if (!checkForOK(20000, WIFI_AUTO_CONNECT_SEARCH, false))
  {
    StringFormatter::send(wifiStream, F("AT+CWMODE=1\r\n")); // Configure as Wireless client
    if (!checkForOK(10000, OK_SEARCH, false))
      return false;
    StringFormatter::send(wifiStream, F("AT+CWJAP=\"%S\",\"%S\"\r\n"), SSid, password); // Connect to wifi access point
    if (!checkForOK(20000, OK_SEARCH, false))
    {
      if (!checkForOK(30000, WIFI_AUTO_CONNECT_SEARCH, false))
      {
        return false;
      }
    }
  }

  StringFormatter::send(wifiStream, F("AT+CIFSR\r\n")); // get ip address //192.168.4.1
  if (!checkForOK(10000, OK_SEARCH, true))
    return false;

  StringFormatter::send(wifiStream, F("AT+CWHOSTNAME=\"%S\"\r\n"), hostname); // Set Host name for Wifi Client
  checkForOK(5000, OK_SEARCH, true);

  StringFormatter::send(wifiStream, F("AT+CIPMUX=1\r\n")); // configure for multiple connections
  if (!checkForOK(10000, OK_SEARCH, false))
    return false;

  StringFormatter::send(wifiStream, F("AT+CIPSERVER=1,%d\r\n"), port); // turn on server on port 80
  if (!checkForOK(10000, OK_SEARCH, false))
    return false;

  StringFormatter::send(wifiStream, F("AT+MDNS=1,\"%S\",\"%S\",%d\r\n"), hostname, servername, port); // Setup mDNS for Server
  if (!checkForOK(5000, OK_SEARCH, false))
    return false;
  return true;
}

bool WifiInterface::checkForOK(const unsigned int timeout, const char *waitfor, bool echo)
{
  unsigned long startTime = millis();
  char const *locator = waitfor;
  DIAG(F("\nWifi Check: %E"), waitfor);
  while (millis() - startTime < timeout)
  {
    while (wifiStream.available())
    {
      int ch = wifiStream.read();
      if (echo)
      {
#if defined(ARDUINO_ARCH_AVR)
        Serial.write(ch);
#else
        SerialUSB.write(ch);
#endif
      }
      if (ch != pgm_read_byte_near(locator))
        locator = waitfor;
      if (ch == pgm_read_byte_near(locator))
      {
        locator++;
        if (!pgm_read_byte_near(locator))
        {
          DIAG(F("\nChecked after %dms"), millis() - startTime);
          return true;
        }
      }
    }
  }
  DIAG(F("\nTIMEOUT after %dms\n"), timeout);
  return false;
}

bool WifiInterface::isHTML()
{

  // POST GET PUT PATCH DELETE
  // You may think a simple strstr() is better... but not when ram & time is in short supply
  switch (buffer[0])
  {
  case 'P':
    if (buffer[1] == 'U' && buffer[2] == 'T' && buffer[3] == ' ')
      return true;
    if (buffer[1] == 'O' && buffer[2] == 'S' && buffer[3] == 'T' && buffer[4] == ' ')
      return true;
    if (buffer[1] == 'A' && buffer[2] == 'T' && buffer[3] == 'C' && buffer[4] == 'H' && buffer[5] == ' ')
      return true;
    return false;
  case 'G':
    if (buffer[1] == 'E' && buffer[2] == 'T' && buffer[3] == ' ')
      return true;
    return false;
  case 'D':
    if (buffer[1] == 'E' && buffer[2] == 'L' && buffer[3] == 'E' && buffer[4] == 'T' && buffer[5] == 'E' && buffer[6] == ' ')
      return true;
    return false;
  default:
    return false;
  }
}

void WifiInterface::process()
{
  if (!connected)
  {
    return;
  }

  WiThrottle::loop(); // check heartbeats

  // read anything into a buffer, collecting info on the way
  while (loopstate != 99 && wifiStream.available())
  {
    int ch = wifiStream.read();
    switch (loopstate)
    {
    case 0: // looking for +
      connectionId = 0;
      streamer.flush();
      if (ch == '+')
        loopstate = 1;
      break;
    case 1: // Looking for I
      loopstate = (ch == 'I') ? 2 : 0;
      break;
    case 2: // Looking for P
      loopstate = (ch == 'P') ? 3 : 0;
      break;
    case 3: // Looking for D
      loopstate = (ch == 'D') ? 4 : 0;
      break;
    case 4: // Looking for ,
      loopstate = (ch == ',') ? 5 : 0;
      break;
    case 5: // reading connection id
      if (ch == ',')
        loopstate = 6;
      else
        connectionId = 10 * connectionId + (ch - '0');
      break;
    case 6: // reading for length
      if (ch == ':')
        loopstate = (datalength == 0) ? 99 : 7; // 99 is getout without reading next char
      else
        datalength = datalength * 10 + (ch - '0');
      break;
    case 7: // reading data
      streamer.write(ch);
      datalength--;
      if (datalength == 0)
        loopstate = 99;
      break;
    } // switch
  }   // while
  if (loopstate != 99)
    return;
  const char *a = "\0";
  streamer.write(a);

  DIAG(F("\nWifiRead:%d:%s\n"), connectionId, buffer);
  streamer.setBufferContentPosition(0, 0); // reset write position to start of buffer
                                           // SIDE EFFECT WARNING:::
                                           //  We know that parser will read the entire buffer before starting to write to it.
                                           //  Otherwise we would have to copy the buffer elsewhere and RAM is in short supply.

  // TODO ... tell JMRI parser that callbacks are diallowed because we dont want to handle the async
  bool closeAfter = false;
  // Intercept HTTP requests
  if (isHTML())
  {
    HTTPParser::parse(streamer, buffer);
    closeAfter = true;
  }
  else if (buffer[0] == '<')
  {
    char *command = buffer;
    DIAG(F("Sending Command:%s to DCCEXParser\n"), buffer);
    while (command[0] == '<' || command[0] == ' ')
      command++;
    DIAG(F("Command: %s"), command);
    DCCEXParser::parse(command);
  }
  else
  {
    WiThrottle::getThrottle(streamer, connectionId)->parse(streamer, buffer);
  }

  loopstate = 0; // go back to looking for +IPD
}

void WifiInterface::showConfiguration()
{
}

void WifiInterface::showInitInfo()
{
  CommManager::printf(connected ? "<WiFi:Connected>" : "<Wifi:Not Connected>");
}

void WifiInterface::send(const char *buf)
{
  streamer.flush();
  sending = true;
  streamer.write(buf);
  DIAG(F("WiFiInterface Responding client (%d) l(%d) %s\n"), connectionId, streamer.available(), buf);
  StringFormatter::send(wifiStream, F("AT+CIPSENDBUF=%d,%d\r\n"), connectionId, streamer.available());
  if (checkForOK(500, PROMPT_SEARCH, true))
  {
    wifiStream.print(buf);
  }
  checkForOK(1000, SEND_OK_SEARCH, true);
  sending = false;
}