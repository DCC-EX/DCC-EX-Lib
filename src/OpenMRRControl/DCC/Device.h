#ifndef Device_h
#define Device_h

#include "Packet.h"

struct Device
{
    Packet packet[2];
    Packet* activePacket;
    Packet* updatePacket;
    void initPackets();
};

#endif