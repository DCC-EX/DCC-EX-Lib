#ifndef OpenMRRControl_h
#define OpenMRRControl_h

#define VERSION "1.0.0"
#define BOARD_NAME "DCC++ CommandStation"

#include "DCC/DCC.h"
#include "CommInterface/CommManager.h"
#include "CommInterface/CommInterfaceSerial.h"

#if defined (ARDUINO_ARCH_SAMD)
    #include "CommInterface/CommInterfaceUSB.h"
#endif

#include "CommInterface/DCCEXParser.h"
#include "Accessories/EEStore.h"

#endif