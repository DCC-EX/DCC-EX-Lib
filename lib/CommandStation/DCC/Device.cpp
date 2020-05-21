#include "Device.h"

void Device::initPackets(){
    activePacket = packet;
    updatePacket = packet+1;
}