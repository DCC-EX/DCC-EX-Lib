#include "DCC.h"
#include "Railcom.h"
#include "../CommInterface/CommManager.h"

#if defined(ARDUINO_ARCH_SAMD)
const uint8_t railcom_decode[256] =
{      INV,    INV,    INV,    INV,    INV,    INV,    INV,    INV,
       INV,    INV,    INV,    INV,    INV,    INV,    INV,   NACK,
       INV,    INV,    INV,    INV,    INV,    INV,    INV,   0x33,
       INV,    INV,    INV,   0x34,    INV,   0x35,   0x36,    INV,
       INV,    INV,    INV,    INV,    INV,    INV,    INV,   0x3A,
       INV,    INV,    INV,   0x3B,    INV,   0x3C,   0x37,    INV,
       INV,    INV,    INV,   0x3F,    INV,   0x3D,   0x38,    INV,
       INV,   0x3E,   0x39,    INV, RESVD3,    INV,    INV,    INV,
       INV,    INV,    INV,    INV,    INV,    INV,    INV,   0x24,
       INV,    INV,    INV,   0x23,    INV,   0x22,   0x21,    INV,
       INV,    INV,    INV,   0x1F,    INV,   0x1E,   0x20,    INV,
       INV,   0x1D,   0x1C,    INV,   0x1B,    INV,    INV,    INV,
       INV,    INV,    INV,   0x19,    INV,   0x18,   0x1A,    INV,
       INV,   0x17,   0x16,    INV,   0x15,    INV,    INV,    INV,
       INV,   0x25,   0x14,    INV,   0x13,    INV,    INV,    INV,
      0x32,    INV,    INV,    INV,    INV,    INV,    INV,    INV,
       INV,    INV,    INV,    INV,    INV,    INV,    INV, RESVD2,
       INV,    INV,    INV,   0x0E,    INV,   0x0D,   0x0C,    INV,
       INV,    INV,    INV,   0x0A,    INV,   0x09,   0x0B,    INV,
       INV,   0x08,   0x07,    INV,   0x06,    INV,    INV,    INV,
       INV,    INV,    INV,   0x04,    INV,   0x03,   0x05,    INV,
       INV,   0x02,   0x01,    INV,   0x00,    INV,    INV,    INV,
       INV,   0x0F,   0x10,    INV,   0x11,    INV,    INV,    INV,
      0x12,    INV,    INV,    INV,    INV,    INV,    INV,    INV,
       INV,    INV,    INV, RESVD1,    INV,   0x2B,   0x30,    INV,
       INV,   0x2A,   0x2F,    INV,   0x31,    INV,    INV,    INV,
       INV,   0x29,   0x2E,    INV,   0x2D,    INV,    INV,    INV,
      0x2C,    INV,    INV,    INV,    INV,    INV,    INV,    INV,
       INV,   BUSY,   0x28,    INV,   0x27,    INV,    INV,    INV,
      0x26,    INV,    INV,    INV,    INV,    INV,    INV,    INV,
       ACK,    INV,    INV,    INV,    INV,    INV,    INV,    INV,
       INV,    INV,    INV,    INV,    INV,    INV,    INV,    INV,
};

void DCC::readRailcomData() {
    if(inRailcomCutout) return;
    int bytes = mainRailcomUART.available();
    if(bytes > 8) bytes = 8;
    uint8_t data[bytes];
    mainRailcomUART.readBytes(data, bytes);
    if(!railcomData) return;

    // for (size_t i = 0; i < bytes; i++)
    // {
    //     SerialUSB.print(data[i], HEX);
    //     SerialUSB.print(" ");
    // }
    // SerialUSB.println();
    // for (size_t i = 0; i < bytes; i++)
    // {
    //     SerialUSB.print(railcom_decode[data[i]], HEX);
    //     SerialUSB.print(" ");
    // }
    // SerialUSB.println();

    Railcom::parseData(data);

    railcomData = false;
}


void Railcom::parseData(const uint8_t data[8]) {
    
    
    for (size_t i = 0; i < 8; i++)
    {
        uint8_t decode = railcom_decode[data[i]];
        uint8_t type = 0xFF;
        if(decode == ACK) {
            type = ACK; 
        }
        else if(decode == NACK) {
            type = NACK;
        } 
        else if(decode == BUSY) {
            type = BUSY;
        }
        else if(decode >= 64) {
            
        }

        // TODO: Finish parseData function
    }
    
}
#endif