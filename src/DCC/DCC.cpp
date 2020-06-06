#include "DCC.h"
#include "../CommInterface/CommManager.h"

uint16_t DCC::counterID = 0;

DCC::DCC(int numDev, Hardware settings) {
    this->hdw = settings;            
    this->numDev = numDev;      

    // Set up the state of the waveform generator
    state = 0;
    currentBit = 0;
    transmitRepeats = 0;
    remainingPreambles = 0;
    generateStartBit = false;
    nextDev = 0;
    railcomData = false;
    generateRailcomCutout = false;
    inRailcomCutout = false;
    ackNeeded = 0;
    modeCV = READCV;
    cvBeingWorked = 0;
    cvBitNum = 0;
    cvValue = 0;
    cvCallbackSub = 0;
    cvCallback = 0;
    numAcksNeeded = 0;
    ackBuffer = 0;
    ackNeeded = 0;
    inVerify = false;

    lastID = counterID;

    packetQueue.clear();
    
    // Allocate memory for the speed table
    speedTable = (Speed *)calloc(numDev+1, sizeof(Speed));
    for (int i = 0; i <= numDev+1; i++)
    {
        speedTable[i].cab = 0;      // Initialize to zero so we don't get a bunch of noise on the track at startup
        speedTable[i].forward = true;
        speedTable[i].speed = 0;
    }
}

void DCC::schedulePacket(const uint8_t buffer[], uint8_t byteCount, uint8_t repeats, uint16_t identifier) {
    if (byteCount>=DCC_PACKET_MAX_SIZE) return; // allow for chksum
    
    Packet newPacket;

    uint8_t checksum=0;
    for (int b=0; b<byteCount; b++) {
        checksum ^= buffer[b];
        newPacket.payload[b] = buffer[b];
    }
    newPacket.payload[byteCount] = checksum;
    newPacket.length = byteCount+1;
    newPacket.repeats = repeats;
    newPacket.transmitID = identifier;

    const Packet pushPacket = newPacket;    // Todo: is there a better way to convert from non-const to const?
    noInterrupts();
    packetQueue.push(pushPacket);
    interrupts();   
}

void DCC::updateSpeed() {
    if (packetQueue.count() > 0) return;  // Don't let this fill the packetQueue with nonsense

    for (; nextDev < numDev; nextDev++) {
        if (speedTable[nextDev].cab > 0) {
            setThrottleResponse response;
            setThrottle(nextDev, speedTable[nextDev].cab, speedTable[nextDev].speed, speedTable[nextDev].forward, response);
            nextDev++;
            return;
        }
    }
    for (nextDev = 0; nextDev < numDev; nextDev++) {
        if (speedTable[nextDev].cab > 0) {
            setThrottleResponse response;
            setThrottle(nextDev, speedTable[nextDev].cab, speedTable[nextDev].speed, speedTable[nextDev].forward, response);
            nextDev++;
            return;
        }
    }
}

int DCC::setThrottle(uint8_t nDev, uint16_t cab, uint8_t tSpeed, bool tDirection, setThrottleResponse& response) {
    uint8_t b[5];                      // save space for checksum byte
    uint8_t nB=0;

    if(nDev<1 || nDev>numDev)
        return ERR_OUT_OF_RANGE;

    if(cab>127)
        b[nB++]=highByte(cab) | 0xC0;      // convert train number into a two-byte address

    b[nB++]=lowByte(cab);
    b[nB++]=0x3F;                        // 128-step speed control byte
    if(tSpeed>=0)
        b[nB++]=tSpeed+(tSpeed>0)+tDirection*128;   // max speed is 126, but speed codes range from 2-127 (0=stop, 1=emergency stop)
    else{
        b[nB++]=1;
        tSpeed=0;
    }

    incrementCounterID();
    schedulePacket(b, nB, 0, counterID);

    speedTable[nDev].speed = tSpeed;
    speedTable[nDev].cab = cab;
    speedTable[nDev].forward = tDirection; 

    response.device = nDev;
    response.direction = tDirection;
    response.speed = tSpeed;
    response.transactionID = counterID;

    return ERR_OK;
}

int DCC::setFunction(uint16_t cab, uint8_t byte1, setFunctionResponse& response) {
    uint8_t b[4];
    uint8_t nB = 0;

    if(cab>127)
        b[nB++]=highByte(cab) | 0xC0;      // convert train number into a two-byte address
    b[nB++]=lowByte(cab);
    b[nB++]=(byte1 | 0x80) & 0xBF;

    incrementCounterID();
    schedulePacket(b, nB, 4, counterID);       // Repeat the packet four times

    response.transactionID = counterID;

    return ERR_OK;       // Will implement error handling later
}

int DCC::setFunction(uint16_t cab, uint8_t byte1, uint8_t byte2, setFunctionResponse& response) {
    uint8_t b[5];
    uint8_t nB = 0;

    if(cab>127)
        b[nB++]=highByte(cab) | 0xC0;      // convert train number into a two-byte address
    b[nB++]=lowByte(cab);
    b[nB++]=(byte1 | 0xDE) & 0xDF;     // for safety this guarantees that first byte will either be 0xDE (for F13-F20) or 0xDF (for F21-F28)
    b[nB++]=byte2;
    
    incrementCounterID();
    schedulePacket(b, nB, 4, counterID);       // Repeat the packet four times

    response.transactionID = counterID;

    return ERR_OK;       // Will implement error handling later
}

int DCC::setAccessory(uint16_t address, uint8_t number, bool activate, setAccessoryResponse& response) {
    byte b[3];                      // save space for checksum byte
    
    b[0]=address%64+128;                                           // first byte is of the form 10AAAAAA, where AAAAAA represent 6 least signifcant bits of accessory address
    b[1]=((((address/64)%8)<<4) + (number%4<<1) + activate%2) ^ 0xF8;      // second byte is of the form 1AAACDDD, where C should be 1, and the least significant D represent activate/deactivate

    incrementCounterID();
    schedulePacket(b, 2, 4, counterID);        // Repeat the packet four times

    response.transactionID = counterID;

    return ERR_OK;        // Will implement error handling later
}

int DCC::writeCVByteMain(uint16_t cab, uint16_t cv, uint8_t bValue, writeCVByteMainResponse& response) {
    byte b[6];                      // save space for checksum byte
    byte nB=0;

    cv--;

    if(cab>127)
        b[nB++]=highByte(cab) | 0xC0;      // convert train number into a two-byte address

    b[nB++]=lowByte(cab);
    b[nB++]=0xEC + (highByte(cv) & 0x03);   // any CV>1023 will become modulus(1024) due to bit-mask of 0x03
    b[nB++]=lowByte(cv);
    b[nB++]=bValue;

    incrementCounterID();
    schedulePacket(b, nB, 4, counterID);

    response.transactionID = counterID;

    return ERR_OK;
}

int DCC::writeCVBitMain(uint16_t cab, uint16_t cv, uint8_t bNum, uint8_t bValue, writeCVBitMainResponse& response) {
    byte b[6];                      // save space for checksum byte
    byte nB=0;

    cv--;

    bValue=bValue%2;
    bNum=bNum%8;

    if(cab>127)
        b[nB++]=highByte(cab) | 0xC0;      // convert train number into a two-byte address

    b[nB++]=lowByte(cab);
    b[nB++]=0xE8+(highByte(cv)&0x03);   // any CV>1023 will become modulus(1024) due to bit-mask of 0x03
    b[nB++]=lowByte(cv);
    b[nB++]=0xF0+bValue*8+bNum;

    incrementCounterID();
    schedulePacket(b, nB, 4, counterID);

    response.transactionID = counterID;

    return ERR_OK;
}

int DCC::writeCVByte(uint16_t cv, uint8_t bValue, uint16_t callback, uint16_t callbackSub) {
    if(ackNeeded != 0) return ERR_BUSY;
    
    uint8_t bWrite[4];

    cv--;                                  // actual CV addresses are cv-1 (0-1023)

    bWrite[0]=0x7C+(highByte(cv)&0x03);   // any CV>1023 will become modulus(1024) due to bit-mask of 0x03
    bWrite[1]=lowByte(cv);
    bWrite[2]=bValue;

    hdw.setBaseCurrent();

    modeCV = WRITECV;
    numAcksNeeded = 1;
    ackNeeded = 0b00000001;
    ackBuffer = 0b00000000;
    inVerify = false;
    
    incrementCounterID();
    schedulePacket(resetPacket, 2, 3, counterID);          // NMRA recommends starting with 3 reset packets
    schedulePacket(bWrite, 3, 5, counterID);               // NMRA recommends 5 verify packets
    schedulePacket(bWrite, 3, 6, counterID);               // NMRA recommends 6 write or reset packets for decoder recovery time
    
    incrementCounterID();
    bWrite[0]=0x74+(highByte(cv)&0x03);   // set-up to re-verify entire byte
    schedulePacket(resetPacket, 2, 3, counterID);          // NMRA recommends starting with 3 reset packets
    schedulePacket(bWrite, 3, 5, counterID);               // NMRA recommends 5 verify packets
    schedulePacket(bWrite, 3, 6, counterID);               // NMRA recommends 6 write or reset packets for decoder recovery time
    ackPacketID[0] = counterID;
    cvCallback = callback;
    cvCallbackSub = callbackSub;
    cvBeingWorked = cv+1;
    cvValue = bValue;
    
    incrementCounterID();
    schedulePacket(resetPacket, 2, 1, counterID);          // Final reset packet (and decoder begins to respond) todo: is this supposed to be one packet or one repeat?

    return ERR_OK;
}


int DCC::writeCVBit(uint16_t cv, uint8_t bNum, uint8_t bValue, uint16_t callback, uint16_t callbackSub) {
    if(ackNeeded != 0) return ERR_BUSY;
    
    byte bWrite[4];

    cv--;                                 // actual CV addresses are cv-1 (0-1023)
    bValue=bValue%2;
    bNum=bNum%8;

    bWrite[0]=0x78+(highByte(cv)&0x03);   // any CV>1023 will become modulus(1024) due to bit-mask of 0x03
    bWrite[1]=lowByte(cv);
    bWrite[2]=0xF0+bValue*8+bNum;

    hdw.setBaseCurrent();

    modeCV = WRITECVBIT;
    numAcksNeeded = 1;
    ackNeeded = 0b00000001;
    ackBuffer = 0b00000000;
    inVerify = false;

    incrementCounterID();
    schedulePacket(resetPacket, 2, 3, counterID);           // NMRA recommends starting with 3 reset packets
    schedulePacket(bWrite, 3, 5, counterID);                // NMRA recommends 5 verify packets
    schedulePacket(bWrite, 3, 6, counterID);                // NMRA recommends 6 write or reset packets for decoder recovery time
    
    incrementCounterID();
    bitClear(bWrite[2],4);                          // change instruction code from Write Bit to Verify Bit
    schedulePacket(resetPacket, 2, 3, counterID);           // NMRA recommends starting with 3 reset packets
    schedulePacket(bWrite, 3, 5, counterID);                // NMRA recommends 5 verify packets
    schedulePacket(bWrite, 3, 6, counterID);                // NMRA recommends 6 write or reset packets for decoder recovery time
    ackPacketID[0] = counterID;
    cvCallback = callback;
    cvCallbackSub = callbackSub;
    cvBeingWorked = cv+1;
    cvBitNum = bNum;
    cvValue = bValue;

    incrementCounterID();
    schedulePacket(resetPacket, 2, 1, counterID);          // Final reset packet (and decoder begins to respond) todo: is this supposed to be one packet or one repeat?

    return ERR_OK;
}


int DCC::readCV(uint16_t cv, uint16_t callback, uint16_t callbackSub) {
    if(ackNeeded != 0) return ERR_BUSY;
    
    uint8_t bRead[4];

    cv--;                                    // actual CV addresses are cv-1 (0-1023)

    bRead[0]=0x78+(highByte(cv)&0x03);       // any CV>1023 will become modulus(1024) due to bit-mask of 0x03
    bRead[1]=lowByte(cv);

    hdw.setBaseCurrent();
    incrementCounterID();

    modeCV = READCV;
    numAcksNeeded = 8;
    ackNeeded = 0b11111111;
    ackBuffer = 0b00000000;
    inVerify = false;


    for(int i=0;i<8;i++) {                      // Queue up all 24 unique packets required for the CV read. Each repeats several times.
        bRead[2]=0xE8+i;

        incrementCounterID();

        schedulePacket(resetPacket, 2, 3, counterID);          // NMRA recommends starting with 3 reset packets
        schedulePacket(bRead, 3, 5, counterID);                // NMRA recommends 5 verify packets
        schedulePacket(resetPacket, 2, 0, counterID);           // NMRA recommends 6 idle or reset packets for decoder recovery time

        ackPacketID[i] = counterID;
    }

    incrementCounterID();
    schedulePacket(resetPacket, 2, 0, counterID);
    
    cvCallback = callback;
    cvCallbackSub = callbackSub;
    cvBeingWorked = cv+1;

    verifyPayload[0]=0x74+(highByte(cv)&0x03);     // set-up to re-verify entire byte
    verifyPayload[1]=lowByte(cv);
    verifyPayload[2]=0;
    verifyPayload[3]=0;

    backToIdle = false;

    return ERR_OK;
}

void DCC::checkAck() {
    
    if(!inVerify && (ackNeeded == 0)) return;

    if(!inVerify) {
        uint16_t currentAckID;
        for (uint8_t i = 0; i < numAcksNeeded; i++)
        {
            if(!bitRead(ackNeeded, i)) continue;    // We don't need an ack on this bit, we already got one

            currentAckID = ackPacketID[i];
            uint16_t compareID = transmitID;
            if(currentAckID == compareID) {
                float currentMilliamps = hdw.getMilliamps(hdw.readCurrent());
                if((currentMilliamps - hdw.baseMilliamps) > ACK_SAMPLE_THRESHOLD) {
                    bitSet(ackBuffer, i);       // We got an ack on this bit
                    bitClear(ackNeeded, i);     // We no longer need an ack on this bit
                }
            }
            else if(compareID > currentAckID || backToIdle) {    // Todo: check for wraparound
                bitClear(ackBuffer, i);       // We didn't get an ack on this bit (timeout)
                bitClear(ackNeeded, i);     // We no longer need an ack on this bit
            }

            if(ackNeeded == 0) {        // If we've now gotten all the ACKs we need
                switch (modeCV)
                {
                case READCV:
                    verifyPayload[2] = ackBuffer;
                    inVerify = true;
                    incrementCounterID();
                    schedulePacket(resetPacket, 2, 3, counterID);
                    schedulePacket(verifyPayload, 3, 5, counterID);
                    schedulePacket(resetPacket, 2, 0, counterID);

                    ackPacketID[0] = counterID;

                    incrementCounterID();
                    schedulePacket(resetPacket, 2, 0, counterID);

                    backToIdle = false;
                    
                    break;
                case WRITECV:
                    inVerify = false;
                    if(bitRead(ackBuffer, 0)) {
                        CommManager::printf(F("<r%d|%d|%d %d>"), cvCallback, cvCallbackSub, cvBeingWorked, cvValue);
                    }
                    else {
                        CommManager::printf(F("<r%d|%d|%d %d>"), cvCallback, cvCallbackSub, cvBeingWorked, -1);
                    }
                    break;
                case WRITECVBIT:
                    inVerify = false;
                    if(bitRead(ackBuffer, 0)) {
                        CommManager::printf(F("<r%d|%d|%d %d %d>"), cvCallback, cvCallbackSub, cvBeingWorked, cvBitNum, cvValue);
                    }
                    else {
                        CommManager::printf(F("<r%d|%d|%d %d %d>"), cvCallback, cvCallbackSub, cvBeingWorked, cvBitNum, -1);
                    }
                    break;
                }
                break;
            }
        }    
    }
    else {
        uint16_t compareID = transmitID;
        if(ackPacketID[0] == compareID) {
            float currentMilliamps = hdw.getMilliamps(hdw.readCurrent());
            if((currentMilliamps - hdw.baseMilliamps) > ACK_SAMPLE_THRESHOLD) {
                inVerify = false;
                CommManager::printf(F("<r%d|%d|%d %d>"), cvCallback, cvCallbackSub, cvBeingWorked, ackBuffer);
            }
        }
        else if(compareID > ackPacketID[0] || backToIdle) {    // Todo: check for wraparound
            inVerify = false;
            CommManager::printf(F("<r%d|%d|%d %d>"), cvCallback, cvCallbackSub, cvBeingWorked, -1);
        }
    }
}