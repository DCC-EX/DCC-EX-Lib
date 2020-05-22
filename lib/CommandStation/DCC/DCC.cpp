#include "DCC.h"

#define  CURRENT_SAMPLE_TIME        1
#define  CURRENT_SAMPLE_SMOOTHING   0.01
#define  RETRY_MILLIS    1000

uint8_t DCC::idlePacket[3]={0xFF,0x00,0};                 // always leave extra byte for checksum computation
uint8_t DCC::resetPacket[3]={0x00,0x00,0};

DCC::DCC(int numDev, DCChdw hdw) {
    this->hdw = hdw;            // Save the hardware settings for this track
    this->numDev = numDev;              // Save the number of devices allowed on this track
    
    // Set up the enable pin for this track
    pinMode(hdw.enable_pin, OUTPUT);
    digitalWrite(hdw.enable_pin, LOW);

    // Create and initialize a device table
    dev = (Device*)calloc(numDev+1, sizeof(Device));     // Device memory allocation. Happens dynamically.
    for (size_t i = 0; i <= numDev; i++)                    
        dev[i].initPackets();                               // Initialize packets in each device
    devMap = (Device**)calloc(numDev+1, sizeof(Device *));
    devMap[0] = dev;

    maxLoadedDev = dev;
    currentDev = dev;
    nextDev = NULL;
    lastDev = NULL;

    currentBit = 0;
    nRepeat = 0;
    preambleLeft = 0;
    generateStartBit = false;

    speedTable = (int *)calloc(numDev+1, sizeof(int *));

    loadPacket(1, idlePacket, 2, 0);
    init_timers();
}

void DCC::loadPacket(int nDev, uint8_t *b, uint8_t nBytes, uint8_t nRepeat) volatile {
    // LOAD DCC PACKET INTO TEMPORARY REGISTER 0, OR PERMANENT REGISTERS 1 THROUGH DCC_PACKET_QUEUE_MAX (INCLUSIVE)
    // CONVERTS 2, 3, 4, OR 5 BYTES INTO A DCC BIT STREAM WITH CHECKSUM AND PROPER BYTE SEPARATORS
    // BITSTREAM IS STORED IN UP TO A 10-BYTE ARRAY (USING AT MOST 76 OF 80 BITS)

    nDev=nDev % (numDev+1);          // force nDev to be between 0 and numDev, inclusive

    while(nextDev!=NULL);              // pause while there is a device already waiting to be updated -- nextDev will be reset to NULL by interrupt when prior Register updated fully processed

    if(devMap[nDev]==NULL)              // first time this device Number has been called
        devMap[nDev]=maxLoadedDev+1;       // set Register Pointer for this Register Number to next available Register

    Device *r=devMap[nDev];           // set Register to be updated
    Packet *p=r->updatePacket;          // set Packet in the Register to be updated
    uint8_t *payload=p->payload;                   // set byte buffer in the Packet to be updated

    b[nBytes]=b[0];                        // copy first byte into what will become the checksum byte
    for(int i=1;i<nBytes;i++)              // XOR remaining bytes into checksum byte
        b[nBytes]^=b[i];
    nBytes++;                              // increment number of bytes in packet to include checksum byte

    payload[0] = b[0];                          // b[0], full byte
    payload[1] = b[1];
    payload[2] = b[2]; 

    if(nBytes==3){
        p->nBytes = 3;
    } else{
        payload[3] = b[3];
        if(nBytes==4){
            p->nBytes = 4;
        } else{
            payload[4] = b[4];
            if(nBytes==5){
                p->nBytes = 5;
            } else{
                payload[5] = b[5];
                p->nBytes = 6;
            } // >5 bytes
        } // >4 bytes
    } // >3 bytes

    nextDev=r;
    this->nRepeat=nRepeat;
    maxLoadedDev=max(maxLoadedDev,nextDev);
}

int DCC::setThrottle(uint8_t nDev, uint16_t cab, uint8_t tSpeed, bool tDirection, setThrottleResponse& response) volatile {
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

    loadPacket(nDev,b,nB,0);

    response.device = nDev;
    response.direction = tDirection;
    response.speed = tSpeed;

    speedTable[nDev]=tDirection==1?tSpeed:-tSpeed;

    return ERR_OK;
}

int DCC::setFunction(uint16_t cab, uint8_t byte1, setFunctionResponse& response) volatile {
    uint8_t b[4];
    uint8_t nB = 0;

    if(cab>127)
        b[nB++]=highByte(cab) | 0xC0;      // convert train number into a two-byte address
    b[nB++]=lowByte(cab);
    b[nB++]=(byte1 | 0x80) & 0xBF;

    loadPacket(0,b,nB,4);       // Repeat the packet four times

    return ERR_OK;       // Will implement error handling later
}

int DCC::setFunction(uint16_t cab, uint8_t byte1, uint8_t byte2, setFunctionResponse& response) volatile {
    uint8_t b[5];
    uint8_t nB = 0;

    if(cab>127)
        b[nB++]=highByte(cab) | 0xC0;      // convert train number into a two-byte address
    b[nB++]=lowByte(cab);
    b[nB++]=(byte1 | 0xDE) & 0xDF;     // for safety this guarantees that first byte will either be 0xDE (for F13-F20) or 0xDF (for F21-F28)
    b[nB++]=byte2;

    loadPacket(0,b,nB,4);       // Repeat the packet four times

    return ERR_OK;       // Will implement error handling later
}

int DCC::setAccessory(uint16_t address, uint8_t number, bool activate, setAccessoryResponse& response) volatile{
    byte b[3];                      // save space for checksum byte
    
    b[0]=address%64+128;                                           // first byte is of the form 10AAAAAA, where AAAAAA represent 6 least signifcant bits of accessory address
    b[1]=((((address/64)%8)<<4) + (number%4<<1) + activate%2) ^ 0xF8;      // second byte is of the form 1AAACDDD, where C should be 1, and the least significant D represent activate/deactivate

    loadPacket(0,b,2,4);        // Repeat the packet four times

    return ERR_OK;        // Will implement error handling later
}

int DCC::writeCVByteMain(uint16_t cab, uint16_t cv, uint8_t bValue, writeCVByteMainResponse& response) volatile {
    byte b[6];                      // save space for checksum byte
    byte nB=0;

    cv--;

    if(cab>127)
        b[nB++]=highByte(cab) | 0xC0;      // convert train number into a two-byte address

    b[nB++]=lowByte(cab);
    b[nB++]=0xEC+(highByte(cv)&0x03);   // any CV>1023 will become modulus(1024) due to bit-mask of 0x03
    b[nB++]=lowByte(cv);
    b[nB++]=bValue;

    loadPacket(0,b,nB,4);

    return ERR_OK;         // Will implement error handling later
}

int DCC::writeCVBitMain(uint16_t cab, uint16_t cv, uint8_t bNum, uint8_t bValue, writeCVBitMainResponse& response) volatile {
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

    loadPacket(0,b,nB,4);

    return ERR_OK;

} // RegisterList::writeCVBitMain()

int DCC::writeCVByte(uint16_t cv, uint8_t bValue, uint16_t callback, uint16_t callbackSub, writeCVByteResponse& response) volatile {
    uint8_t bWrite[4];
    int c,d,base;

    cv--;                                  // actual CV addresses are cv-1 (0-1023)

    bWrite[0]=0x7C+(highByte(cv)&0x03);   // any CV>1023 will become modulus(1024) due to bit-mask of 0x03
    bWrite[1]=lowByte(cv);
    bWrite[2]=bValue;

    loadPacket(0,resetPacket,2,3);        // NMRA recommends starting with 3 reset packets
    loadPacket(0,bWrite,3,5);             // NMRA recommends 5 verify packets
    loadPacket(0,bWrite,3,6);             // NMRA recommends 6 write or reset packets for decoder recovery time
        
    c=0;
    d=0;
    base=0;

    for(int j=0;j<ACK_BASE_COUNT;j++)
        base+=analogRead(hdw.current_sense_pin);
    base/=ACK_BASE_COUNT;
    
    bWrite[0]=0x74+(highByte(cv)&0x03);   // set-up to re-verify entire byte

    loadPacket(0,resetPacket,2,3);        // NMRA recommends starting with 3 reset packets
    loadPacket(0,bWrite,3,5);             // NMRA recommends 5 verify packets
    loadPacket(0,bWrite,3,6);             // NMRA recommends 6 write or reset packets for decoder recovery time
    
    for(int j=0;j<ACK_SAMPLE_COUNT;j++){
        c=(analogRead(hdw.current_sense_pin)-base)*ACK_SAMPLE_SMOOTHING+c*(1.0-ACK_SAMPLE_SMOOTHING);
        if(c>ACK_SAMPLE_THRESHOLD)
        d=1;
    }
    
    loadPacket(0,resetPacket,2,1);        // Final reset packet (and decoder begins to respond)
    
    if(d==0)    // verify unsuccessful
        bValue=-1;

    response.callback = callback;
    response.callbackSub = callbackSub;
    response.cv = cv+1;
    response.bValue = bValue;

    return ERR_OK;
}


int DCC::writeCVBit(uint16_t cv, uint8_t bNum, uint8_t bValue, uint16_t callback, uint16_t callbackSub, writeCVBitResponse& response) volatile {
    byte bWrite[4];
    int c,d,base;

    cv--;                                 // actual CV addresses are cv-1 (0-1023)
    bValue=bValue%2;
    bNum=bNum%8;

    bWrite[0]=0x78+(highByte(cv)&0x03);   // any CV>1023 will become modulus(1024) due to bit-mask of 0x03
    bWrite[1]=lowByte(cv);
    bWrite[2]=0xF0+bValue*8+bNum;
    loadPacket(0,resetPacket,2,3);        // NMRA recommends starting with 3 reset packets
    loadPacket(0,bWrite,3,5);             // NMRA recommends 5 verify packets
    loadPacket(0,bWrite,3,6);             // NMRA recommends 6 write or reset packets for decoder recovery time
        
    c=0;
    d=0;
    base=0;

    for(int j=0;j<ACK_BASE_COUNT;j++)
        base+=analogRead(hdw.current_sense_pin);
    base/=ACK_BASE_COUNT;
    
    bitClear(bWrite[2],4);              // change instruction code from Write Bit to Verify Bit
    loadPacket(0,resetPacket,2,3);      // NMRA recommends starting with 3 reset packets
    loadPacket(0,bWrite,3,5);           // NMRA recommends 5 verify packets
    loadPacket(0,bWrite,3,6);           // NMRA recommends 6 write or reset packets for decoder recovery time
        
    for(int j=0;j<ACK_SAMPLE_COUNT;j++){
        c=(analogRead(hdw.current_sense_pin)-base)*ACK_SAMPLE_SMOOTHING+c*(1.0-ACK_SAMPLE_SMOOTHING);
        if(c>ACK_SAMPLE_THRESHOLD)
        d=1;
    }
    
    loadPacket(0,resetPacket,2,1);      // Final reset packetcompleted (and decoder begins to respond)

    if(d==0)    // verify unsuccessful
        bValue=-1;
    
    response.callback = callback;
    response.callbackSub = callbackSub;
    response.bNum = bNum;
    response.bValue = bValue;
    response.cv = cv+1;

    return ERR_OK;
}


int DCC::readCV(uint16_t cv, uint16_t callback, uint16_t callbackSub, readCVResponse& response) volatile {
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

    loadPacket(0,resetPacket,2,3);            // NMRA recommends starting with 3 reset packets
    loadPacket(0,bRead,3,5);                  // NMRA recommends 5 verify packets
    loadPacket(0, idlePacket, 2, 6);          // NMRA recommends 6 idle or reset packets for decoder recovery time

        for(int j=0;j<ACK_SAMPLE_COUNT;j++){
        c=(analogRead(hdw.current_sense_pin)-base)*ACK_SAMPLE_SMOOTHING+c*(1.0-ACK_SAMPLE_SMOOTHING);
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

    loadPacket(0,resetPacket,2,3);        // NMRA recommends starting with 3 reset packets
    loadPacket(0,bRead,3,5);              // NMRA recommends 5 verify packets
    loadPacket(0, idlePacket, 2, 6);      // NMRA recommends 6 idle or reset packets for decoder recovery time
    
    for(int j=0;j<ACK_SAMPLE_COUNT;j++){
        c=(analogRead(hdw.current_sense_pin)-base)*ACK_SAMPLE_SMOOTHING+c*(1.0-ACK_SAMPLE_SMOOTHING);
        if(c>ACK_SAMPLE_THRESHOLD)
        d=1;
    }
    
    loadPacket(0,resetPacket,2,1);        // Final reset packet completed (and decoder begins to respond)
    
    if(d==0)    // verify unsuccessful
        bValue=-1;

    response.cv = cv+1;
    response.callback = callback;
    response.callbackSub = callbackSub;
    response.bValue = bValue;

    return ERR_OK;
}

void DCC::check() volatile {
    // if we have exceeded the CURRENT_SAMPLE_TIME we need to check if we are over/under current.
	if(millis() - lastCheckTime > CURRENT_SAMPLE_TIME) { // TODO can we integrate this with the readBaseCurrent and ackDetect routines?
		lastCheckTime = millis();
		reading = analogRead(hdw.current_sense_pin) * CURRENT_SAMPLE_SMOOTHING + reading * (1.0 - CURRENT_SAMPLE_SMOOTHING);
		current = (reading * hdw.current_conversion_factor); // get current in milliamps
		if(current > hdw.trigger_value && digitalRead(hdw.enable_pin)) { // TODO convert this to integer match
			powerOff();
			tripped=true;
			lastTripTime=millis();
		} else if(current < hdw.trigger_value && tripped) { // TODO need to put a delay in here so it only tries after X seconds
			if (millis() - lastTripTime > RETRY_MILLIS) {  // TODO make this a global constant
			  powerOn();
			  tripped=false;
			}
		}
	}

}

void DCC::powerOn() volatile {
	digitalWrite(hdw.enable_pin, HIGH);
}

void DCC::powerOff() volatile {
	digitalWrite(hdw.enable_pin, LOW);
}

int DCC::getLastRead() volatile {
	return current;
}

