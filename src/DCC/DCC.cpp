#include "DCC.h"

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
    packetPending = false;
    railcomData = false;
    generateRailcomCutout = false;
    inRailcomCutout = false;
    
    // Allocate memory for the speed table
    speedTable = (Speed *)calloc(numDev+1, sizeof(Speed));
    for (int i = 0; i <= numDev+1; i++)
    {
        speedTable[i].cab = 0;      // Initialize to zero so we don't get a bunch of noise on the track at startup
        speedTable[i].forward = true;
        speedTable[i].speed = 0;
    }
}

void DCC::schedulePacket(const uint8_t buffer[], uint8_t byteCount, uint8_t repeats) {
    if (byteCount>=DCC_PACKET_MAX_SIZE) return; // allow for chksum
    while(packetPending) delay(1);       // Wait for the pending packet to be transmitted
    
    uint8_t checksum=0;
    for (int b=0; b<byteCount; b++) {
        checksum ^= buffer[b];
        pendingPacket[b] = buffer[b];
    }
    pendingPacket[byteCount] = checksum;
    pendingLength = byteCount+1;
    pendingRepeats = repeats;
    packetPending = true;
}

void DCC::updateSpeed() {
    if (packetPending) return;  // Don't let this block the main loop

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

    schedulePacket(b, nB, 0);

    speedTable[nDev].speed = tSpeed;
    speedTable[nDev].cab = cab;
    speedTable[nDev].forward = tDirection; 

    response.device = nDev;
    response.direction = tDirection;
    response.speed = tSpeed;

    return ERR_OK;
}

int DCC::setFunction(uint16_t cab, uint8_t byte1, setFunctionResponse& response) {
    uint8_t b[4];
    uint8_t nB = 0;

    if(cab>127)
        b[nB++]=highByte(cab) | 0xC0;      // convert train number into a two-byte address
    b[nB++]=lowByte(cab);
    b[nB++]=(byte1 | 0x80) & 0xBF;

    schedulePacket(b, nB, 4);       // Repeat the packet four times

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

    schedulePacket(b, nB, 4);       // Repeat the packet four times

    return ERR_OK;       // Will implement error handling later
}

int DCC::setAccessory(uint16_t address, uint8_t number, bool activate, setAccessoryResponse& response) {
    byte b[3];                      // save space for checksum byte
    
    b[0]=address%64+128;                                           // first byte is of the form 10AAAAAA, where AAAAAA represent 6 least signifcant bits of accessory address
    b[1]=((((address/64)%8)<<4) + (number%4<<1) + activate%2) ^ 0xF8;      // second byte is of the form 1AAACDDD, where C should be 1, and the least significant D represent activate/deactivate

    schedulePacket(b, 2, 4);        // Repeat the packet four times

    return ERR_OK;        // Will implement error handling later
}

int DCC::writeCVByteMain(uint16_t cab, uint16_t cv, uint8_t bValue, writeCVByteMainResponse& response) {
    byte b[6];                      // save space for checksum byte
    byte nB=0;

    cv--;

    if(cab>127)
        b[nB++]=highByte(cab) | 0xC0;      // convert train number into a two-byte address

    b[nB++]=lowByte(cab);
    b[nB++]=0xEC+(highByte(cv)&0x03);   // any CV>1023 will become modulus(1024) due to bit-mask of 0x03
    b[nB++]=lowByte(cv);
    b[nB++]=bValue;

    schedulePacket(b, nB, 4);

    return ERR_OK;         // Will implement error handling later
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

    schedulePacket(b, nB, 4);

    return ERR_OK;

} // RegisterList::writeCVBitMain()

int DCC::writeCVByte(uint16_t cv, uint8_t bValue, uint16_t callback, uint16_t callbackSub, writeCVByteResponse& response) {
    uint8_t bWrite[4];
    int c,d,base;

    cv--;                                  // actual CV addresses are cv-1 (0-1023)

    bWrite[0]=0x7C+(highByte(cv)&0x03);   // any CV>1023 will become modulus(1024) due to bit-mask of 0x03
    bWrite[1]=lowByte(cv);
    bWrite[2]=bValue;

    
    schedulePacket(resetPacket, 2, 3);          // NMRA recommends starting with 3 reset packets
    schedulePacket(bWrite, 3, 5);               // NMRA recommends 5 verify packets
    schedulePacket(bWrite, 3, 6);               // NMRA recommends 6 write or reset packets for decoder recovery time
        
    c=0;
    d=0;
    base=0;

    for(int j=0;j<ACK_BASE_COUNT;j++)
        base+=analogRead(hdw.current_sense_pin);
    base/=ACK_BASE_COUNT;
    
    bWrite[0]=0x74+(highByte(cv)&0x03);   // set-up to re-verify entire byte

    schedulePacket(resetPacket, 2, 3);          // NMRA recommends starting with 3 reset packets
    schedulePacket(bWrite, 3, 5);               // NMRA recommends 5 verify packets
    schedulePacket(bWrite, 3, 6);               // NMRA recommends 6 write or reset packets for decoder recovery time
    
    for(int j=0;j<ACK_SAMPLE_COUNT;j++){
        c=analogRead(hdw.current_sense_pin)-base;
        if(c>ACK_SAMPLE_THRESHOLD)
        d=1;
    }
    
    schedulePacket(resetPacket, 2, 1);          // Final reset packet (and decoder begins to respond) todo: is this supposed to be one packet or one repeat?
    
    if(d==0)    // verify unsuccessful
        bValue=-1;

    response.callback = callback;
    response.callbackSub = callbackSub;
    response.cv = cv+1;
    response.bValue = bValue;

    return ERR_OK;
}


int DCC::writeCVBit(uint16_t cv, uint8_t bNum, uint8_t bValue, uint16_t callback, uint16_t callbackSub, writeCVBitResponse& response) {
    byte bWrite[4];
    int c,d,base;

    cv--;                                 // actual CV addresses are cv-1 (0-1023)
    bValue=bValue%2;
    bNum=bNum%8;

    bWrite[0]=0x78+(highByte(cv)&0x03);   // any CV>1023 will become modulus(1024) due to bit-mask of 0x03
    bWrite[1]=lowByte(cv);
    bWrite[2]=0xF0+bValue*8+bNum;
    schedulePacket(resetPacket, 2, 3);          // NMRA recommends starting with 3 reset packets
    schedulePacket(bWrite, 3, 5);               // NMRA recommends 5 verify packets
    schedulePacket(bWrite, 3, 6);               // NMRA recommends 6 write or reset packets for decoder recovery time
        
    c=0;
    d=0;
    base=0;

    for(int j=0;j<ACK_BASE_COUNT;j++)
        base+=analogRead(hdw.current_sense_pin);
    base/=ACK_BASE_COUNT;
    
    bitClear(bWrite[2],4);              // change instruction code from Write Bit to Verify Bit
    schedulePacket(resetPacket, 2, 3);          // NMRA recommends starting with 3 reset packets
    schedulePacket(bWrite, 3, 5);               // NMRA recommends 5 verify packets
    schedulePacket(bWrite, 3, 6);               // NMRA recommends 6 write or reset packets for decoder recovery time
        
    for(int j=0;j<ACK_SAMPLE_COUNT;j++){
        c=analogRead(hdw.current_sense_pin)-base;
        if(c>ACK_SAMPLE_THRESHOLD)
            d=1;
    }
    
    schedulePacket(resetPacket, 2, 1);          // Final reset packet (and decoder begins to respond) todo: is this supposed to be one packet or one repeat?

    if(d==0)    // verify unsuccessful
        bValue=-1;
    
    response.callback = callback;
    response.callbackSub = callbackSub;
    response.bNum = bNum;
    response.bValue = bValue;
    response.cv = cv+1;

    return ERR_OK;
}


int DCC::readCV(uint16_t cv, uint16_t callback, uint16_t callbackSub, readCVResponse& response) {
    byte bRead[4];
    int bValue;
    int c,d,base;

    cv--;                                    // actual CV addresses are cv-1 (0-1023)

    bRead[0]=0x78+(highByte(cv)&0x03);       // any CV>1023 will become modulus(1024) due to bit-mask of 0x03
    bRead[1]=lowByte(cv);

    bValue=0;

    for(int i=0;i<8;i++) {
        c=0;
        d=0;
        base=0;
        for(int j=0;j<ACK_BASE_COUNT;j++) {
            base+=analogRead(hdw.current_sense_pin);
        }
    base/=ACK_BASE_COUNT;

    bRead[2]=0xE8+i;

    schedulePacket(resetPacket, 2, 3);          // NMRA recommends starting with 3 reset packets
    schedulePacket(bRead, 3, 5);                // NMRA recommends 5 verify packets
    schedulePacket(idlePacket, 2, 6);           // NMRA recommends 6 idle or reset packets for decoder recovery time

        for(int j=0;j<ACK_SAMPLE_COUNT;j++){
            c=analogRead(hdw.current_sense_pin)-base;
            if(c>ACK_SAMPLE_THRESHOLD) {
                d=1;
            }
        }
        bitWrite(bValue,i,d);
    }

    c=0;
    d=0;
    base=0;

    for(int j=0;j<ACK_BASE_COUNT;j++) {
        base+=analogRead(hdw.current_sense_pin);
    }
    base/=ACK_BASE_COUNT;

    bRead[0]=0x74+(highByte(cv)&0x03);     // set-up to re-verify entire byte
    bRead[2]=bValue;

    schedulePacket(resetPacket, 2, 3);          // NMRA recommends starting with 3 reset packets
    schedulePacket(bRead, 3, 5);                // NMRA recommends 5 verify packets
    schedulePacket(idlePacket, 2, 6);           // NMRA recommends 6 idle or reset packets for decoder recovery time
    
    for(int j=0;j<ACK_SAMPLE_COUNT;j++){
        c=(analogRead(hdw.current_sense_pin)-base)*ACK_SAMPLE_SMOOTHING+c*(1.0-ACK_SAMPLE_SMOOTHING);
        if(c>ACK_SAMPLE_THRESHOLD)
            d=1;
    }
    
    schedulePacket(resetPacket, 2, 1);        // Final reset packet completed (and decoder begins to respond) todo: is this supposed to be one packet or one repeat?
    
    if(d==0)    // verify unsuccessful
        bValue=-1;

    response.cv = cv+1;
    response.callback = callback;
    response.callbackSub = callbackSub;
    response.bValue = bValue;

    return ERR_OK;
}

