/*
 *  CommandStation.h
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

#ifndef COMMANDSTATION_COMMANDSTATION_H_
#define COMMANDSTATION_COMMANDSTATION_H_

#include "Accessories/EEStore.h"
#include "CommInterface/CommManager.h"
#include "CommInterface/CommInterfaceSerial.h"
#include "CommInterface/DCCEXParser.h"
#include "DCC/DCCMain.h"
#include "DCC/DCCService.h"

#if defined(ARDUINO_ARCH_SAMD)
#include "CommInterface/CommInterfaceUSB.h"
#endif

#if defined(CONFIG_ENABLE_WIFI)
#include "CommInterface/CommInterfaceESP.h"
#endif

#define VERSION "1.0.0"
#define BOARD_NAME "DCC++ Command Station"

#endif // COMMANDSTATION_COMMANDSTATION_H_