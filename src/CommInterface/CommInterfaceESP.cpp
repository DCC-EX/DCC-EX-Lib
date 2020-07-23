/*
 *  CommInterfaceSerial.cpp
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

#include "CommInterfaceESP.h"

#include <Arduino.h>

#include "CommManager.h"
#include "DCCEXParser.h"

#if defined(ARDUINO_AVR_UNO)
ESPInterface::ESPInterface(SoftwareSerial &serial, long baud) : serialStream(serial), baud(baud), buffer(""), inCommandPayload(false)
{
  serialStream.begin(baud);
  serialStream.flush();
}
#else
ESPInterface::ESPInterface(HardwareSerial &serial, long baud) : serialStream(serial), baud(baud), buffer(""), inCommandPayload(false)
{
  serialStream.begin(baud);
  serialStream.flush();
}
#endif

void ESPInterface::process()
{
  while (serialStream.available())
  {
    char ch = serialStream.read();
    if (ch == '[')
    {
      inCommandPayload = true;
      buffer = "";
    }
    else if (ch == ']')
    {
      int id = buffer.charAt(0);
      buffer.remove(0, 3);
      DCCEXParser::parse(buffer.c_str());
      buffer = "";
      inCommandPayload = false;
    }
    else if (inCommandPayload)
    {
      buffer += ch;
    }
  }
}

void ESPInterface::showConfiguration()
{
  serialStream.print("[0 Wifi Serial - Speed:]");
  serialStream.println(baud);
}

void ESPInterface::showInitInfo()
{
  CommManager::printf("[0 <N0:SERIAL>]");
}

void ESPInterface::send(const char *buf)
{
  CommManager::printf(F("[0 %s]"), buf);
}