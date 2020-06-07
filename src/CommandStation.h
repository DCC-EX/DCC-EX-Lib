#ifndef COMMANDSTATION_COMMANDSTATION_H_
#define COMMANDSTATION_COMMANDSTATION_H_

#include "Accessories/EEStore.h"
#include "CommInterface/CommManager.h"
#include "CommInterface/CommInterfaceSerial.h"
#include "CommInterface/DCCEXParser.h"
#include "DCC/DCC.h"

#if defined (ARDUINO_ARCH_SAMD)
    #include "CommInterface/CommInterfaceUSB.h"
#endif

#define VERSION "1.0.0"
#define BOARD_NAME "DCC++ Command Station"

#endif  // COMMANDSTATION_COMMANDSTATION_H_