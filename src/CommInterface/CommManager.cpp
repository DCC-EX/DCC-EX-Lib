/*
 *  CommManager.cpp
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

#include "CommManager.h"

#include <Arduino.h>

#if defined(ARDUINO_ARCH_SAMD)
#include <cstdarg>
#endif

CommInterface *CommManager::interfaces[5] = {NULL, NULL, NULL, NULL, NULL};
int CommManager::nextInterface = 0;

void CommManager::update()
{
  for (int i = 0; i < nextInterface; i++)
  {
    if (interfaces[i] != NULL)
    {
      interfaces[i]->process();
    }
  }
}

void CommManager::registerInterface(CommInterface *interface)
{
  if (nextInterface < 5)
  {
    interfaces[nextInterface++] = interface;
    interface->id = nextInterface;
  }
}

void CommManager::showConfiguration(const int comId, const int connectionId)
{
  if (interfaces[comId] != NULL)
  {
    interfaces[comId]->showConfiguration(connectionId);
  }
}

void CommManager::showInitInfo(const int comId, const int connectionId)
{
  if (interfaces[comId] != NULL)
  {
    interfaces[comId]->showInitInfo(connectionId);
  }
}

void CommManager::printf(const int comId, const int connectionId, const char *fmt, ...)
{
  char buf[256] = {0};
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  if (interfaces[comId] != NULL)
  {
    interfaces[comId]->send(buf);
  }
}

void CommManager::allprintf(const char *fmt, ...)
{
  char buf[256] = {0};
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  for (int i = 0; i < nextInterface; i++)
  {
    if (interfaces[i] != NULL)
    {
      interfaces[i]->send(buf);
    }
  }

  void CommManager::printf(const int comId, const int connectionId, const char *fmt, va_list args)
  {
    char buf[256] = {0};
    vsnprintf(buf, sizeof(buf), fmt, args);
    if (interfaces[comId] != NULL)
    {
      interfaces[comId]->send(connectionId, buf);
    }
  }

  void CommManager::printf(const int comId, const int connectionId, const __FlashStringHelper *fmt, ...)
  {
    va_list args;
    va_start(args, fmt);

    char *flash = (char *)fmt;
    char string[256];
    for (int i = 0; i < 256; ++i)
    {
      string[i] = '\0';
    }
    for (int i = 0;; ++i)
    {
      char c = pgm_read_byte_near(flash + i);
      if (c == '\0')
        break;
      string[i] = c;
    }
    printf(comId, connectionId, string, args);
    va_end(args);
  }