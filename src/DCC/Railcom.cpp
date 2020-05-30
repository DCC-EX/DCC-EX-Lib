#include "DCC.h"
#include "../CommInterface/CommManager.h"

void DCC::readRailcomData() {
    if(inRailcomCutout) return;
    int bytes = mainRailcomUART.available();
    if(bytes > 8) bytes = 8;
    uint8_t data[bytes];
    mainRailcomUART.readBytes(data, bytes);
    if(!railcomData) return;

    for (size_t i = 0; i < bytes; i++)
    {
        SerialUSB.print(data[i], HEX);
        SerialUSB.print(" ");
        
    }
    SerialUSB.println();

    railcomData = false;
    
}

