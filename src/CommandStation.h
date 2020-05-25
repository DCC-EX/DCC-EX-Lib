#ifndef OpenMRRControl_h
#define OpenMRRControl_h

#define VERSION "1.0"
#define BOARD_NAME "DCC++ CommandStation"

#include "DCC/DCC.h"
#include "CommInterface/CommManager.h"
#include "CommInterface/CommInterfaceSerial.h"
#if defined (ATSAMD21G)
#include "CommInterface/CommInterfaceUSB.h"
#endif
#include "CommInterface/StringParser.h"

#endif