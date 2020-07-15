/*
 *  CommManager.h
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

#ifndef COMMANDSTATION_COMMINTERFACE_COMMMANAGER_H_
#define COMMANDSTATION_COMMINTERFACE_COMMMANAGER_H_

#include "CommInterface.h"
#include "../DCC/DCCMain.h"
#include "../DCC/DCCService.h"

#if defined(ARDUINO_ARCH_SAMD)
#include <cstdarg>
#endif

class CommManager {
public:
	static void update();
	static void registerInterface(CommInterface *interface);
	static void showConfiguration();
	static void showInitInfo();
	static void printf(const char *fmt, ...);
	static void printf(const char *fmt, va_list args);
	static void printf(const __FlashStringHelper* fmt, ...);
private:
	static CommInterface *interfaces[5];
	static int nextInterface;
};

#endif	// COMMANDSTATION_COMMINTERFACE_COMMMANAGER_H_