#ifndef Packet_h
#define Packet_h

#include "Arduino.h"

#define DCC_PACKET_MAX_SIZE 10

struct Packet
{
    /// Packet payload bytes
    uint8_t buf[DCC_PACKET_MAX_SIZE];
    /// Number of bits in the packet
    uint8_t nBits;
    /// Feedback key. Identifier used to attribute Railcom data to a specific packet. 
    /// Will be updated after packet is transmitted to the track.
    uintptr_t feedback_key;
};

#endif