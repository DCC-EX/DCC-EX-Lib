#ifndef Packet_h
#define Packet_h

#include "Arduino.h"

#define DCC_PACKET_MAX_SIZE 6       // 5 Bytes plus checksum. Preamble and start bits are added by the interrupt handler.

struct Packet
{
    /// Packet payload bytes
    uint8_t payload[DCC_PACKET_MAX_SIZE];
    /// Number of bits in the packet
    uint8_t nBytes;
};

#endif