#include "DCC.h"
#include "../CommInterface/CommManager.h"

#define  CURRENT_SAMPLE_TIME        1
#define  CURRENT_SAMPLE_SMOOTHING          0.01

DCC::DCC(int numDev, DCChdw hdw) {
    this->hdwSettings = hdw;
    // memcpy((void*)&hdwSettings, &hdw, sizeof(DCChdw));
    this->numDev = numDev;
    
    PORT->Group[hdwSettings.enable_pin_group].PINCFG[hdwSettings.enable_pin].bit.INEN = 1;
    PORT->Group[hdwSettings.enable_pin_group].OUTCLR.reg = 1 << hdwSettings.enable_pin;

    // Create and initialize a device (old:registers) table
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

    speedTable = (int *)calloc(numDev+1, sizeof(int *));

    loadPacket(1, idlePacket, 2, 0);
    init_timers();
}

void DCC::loadPacket(int nDev, uint8_t *b, int nBytes, int nRepeat) volatile {
    // LOAD DCC PACKET INTO TEMPORARY REGISTER 0, OR PERMANENT REGISTERS 1 THROUGH DCC_PACKET_QUEUE_MAX (INCLUSIVE)
    // CONVERTS 2, 3, 4, OR 5 BYTES INTO A DCC BIT STREAM WITH PREAMBLE, CHECKSUM, AND PROPER BYTE SEPARATORS
    // BITSTREAM IS STORED IN UP TO A 10-BYTE ARRAY (USING AT MOST 76 OF 80 BITS)

    nDev=nDev % (numDev+1);          // force nDev to be between 0 and numDev, inclusive

    while(nextDev!=NULL);              // pause while there is a device already waiting to be updated -- nextDev will be reset to NULL by interrupt when prior Register updated fully processed

    if(devMap[nDev]==NULL)              // first time this device Number has been called
        devMap[nDev]=maxLoadedDev+1;       // set Register Pointer for this Register Number to next available Register

    Device *r=devMap[nDev];           // set Register to be updated
    Packet *p=r->updatePacket;          // set Packet in the Register to be updated
    uint8_t *buf=p->buf;                   // set byte buffer in the Packet to be updated

    b[nBytes]=b[0];                        // copy first byte into what will become the checksum byte
    for(int i=1;i<nBytes;i++)              // XOR remaining bytes into checksum byte
        b[nBytes]^=b[i];
    nBytes++;                              // increment number of bytes in packet to include checksum byte

    buf[0]=0xFF;                        // first 8 bytes of 22-byte preamble
    buf[1]=0xFF;                        // second 8 bytes of 22-byte preamble
    buf[2]=0xFC + bitRead(b[0],7);      // last 6 bytes of 22-byte preamble + data start bit + b[0], bit 7
    buf[3]=b[0]<<1;                     // b[0], bits 6-0 + data start bit
    buf[4]=b[1];                        // b[1], all bits
    buf[5]=b[2]>>1;                     // b[2], bits 7-1
    buf[6]=b[2]<<7;                     // b[2], bit 0

    if(nBytes==3){
        p->nBits=49;
    } else{
        buf[6]+=b[3]>>2;                  // b[3], bits 7-2
        buf[7]=b[3]<<6;                   // b[3], bit 1-0
        if(nBytes==4){
        p->nBits=58;
        } else{
        buf[7]+=b[4]>>3;                // b[4], bits 7-3
        buf[8]=b[4]<<5;                 // b[4], bits 2-0
        if(nBytes==5){
            p->nBits=67;
        } else{
            buf[8]+=b[5]>>4;              // b[5], bits 7-4
            buf[9]=b[5]<<4;               // b[5], bits 3-0
            p->nBits=76;
        } // >5 bytes
        } // >4 bytes
    } // >3 bytes

    nextDev=r;
    this->nRepeat=nRepeat;
    maxLoadedDev=max(maxLoadedDev,nextDev);
}

int DCC::setThrottle(uint8_t nReg, uint16_t cab, uint8_t tSpeed, bool tDirection, setThrottleResponse& response) volatile {
    uint8_t b[5];                      // save space for checksum byte
    uint8_t nB=0;

    if(nReg<1 || nReg>numDev)
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

    loadPacket(nReg,b,nB,0);

    response.device = nReg;
    response.direction = tDirection;
    response.speed = tSpeed;

    speedTable[nReg]=tDirection==1?tSpeed:-tSpeed;
}

int DCC::setFunction(uint16_t cab, uint8_t byte1) volatile {
    uint8_t b[4];
    uint8_t nB = 0;

    if(cab>127)
        b[nB++]=highByte(cab) | 0xC0;      // convert train number into a two-byte address
    b[nB++]=lowByte(cab);
    b[nB++]=(byte1 | 0x80) & 0xBF;

    loadPacket(0,b,nB,4);       // Repeat the packet four times

    return 0;       // Will implement error handling later
}

int DCC::setFunction(uint16_t cab, uint8_t byte1, uint8_t byte2) volatile {
    uint8_t b[5];
    uint8_t nB = 0;

    if(cab>127)
        b[nB++]=highByte(cab) | 0xC0;      // convert train number into a two-byte address
    b[nB++]=lowByte(cab);
    b[nB++]=(byte1 | 0xDE) & 0xDF;     // for safety this guarantees that first byte will either be 0xDE (for F13-F20) or 0xDF (for F21-F28)
    b[nB++]=byte2;

    loadPacket(0,b,nB,4);       // Repeat the packet four times

    return 0;       // Will implement error handling later
}

int DCC::setAccessory(uint16_t address, uint8_t number, bool activate) volatile{
    byte b[3];                      // save space for checksum byte
    
    b[0]=address%64+128;                                           // first byte is of the form 10AAAAAA, where AAAAAA represent 6 least signifcant bits of accessory address
    b[1]=((((address/64)%8)<<4) + (number%4<<1) + activate%2) ^ 0xF8;      // second byte is of the form 1AAACDDD, where C should be 1, and the least significant D represent activate/deactivate

    loadPacket(0,b,2,4);        // Repeat the packet four times

    return 0;        // Will implement error handling later
}

int DCC::writeCVByteMain(uint16_t cab, uint16_t cv, uint8_t bValue) volatile {
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

  return 0;         // Will implement error handling later
}

int DCC::writeCVBitMain(uint16_t cab, uint16_t cv, uint8_t bNum, uint8_t bValue) volatile {
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

int DCC::writeCVByte(uint16_t cv, uint8_t bValue, uint16_t callback, uint16_t callbackSub, writeCVResponse& response) volatile {
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
        base+=analogRead(hdwSettings.current_sense_pin);
    base/=ACK_BASE_COUNT;
    
    bWrite[0]=0x74+(highByte(cv)&0x03);   // set-up to re-verify entire byte

    loadPacket(0,resetPacket,2,3);        // NMRA recommends starting with 3 reset packets
    loadPacket(0,bWrite,3,5);             // NMRA recommends 5 verify packets
    loadPacket(0,bWrite,3,6);             // NMRA recommends 6 write or reset packets for decoder recovery time
    
    for(int j=0;j<ACK_SAMPLE_COUNT;j++){
        c=(analogRead(hdwSettings.current_sense_pin)-base)*ACK_SAMPLE_SMOOTHING+c*(1.0-ACK_SAMPLE_SMOOTHING);
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
        base+=analogRead(hdwSettings.current_sense_pin);
    base/=ACK_BASE_COUNT;
    
    bitClear(bWrite[2],4);              // change instruction code from Write Bit to Verify Bit
    loadPacket(0,resetPacket,2,3);      // NMRA recommends starting with 3 reset packets
    loadPacket(0,bWrite,3,5);           // NMRA recommends 5 verify packets
    loadPacket(0,bWrite,3,6);           // NMRA recommends 6 write or reset packets for decoder recovery time
        
    for(int j=0;j<ACK_SAMPLE_COUNT;j++){
        c=(analogRead(hdwSettings.current_sense_pin)-base)*ACK_SAMPLE_SMOOTHING+c*(1.0-ACK_SAMPLE_SMOOTHING);
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
            base+=analogRead(hdwSettings.current_sense_pin);
        }
    base/=ACK_BASE_COUNT;

    bRead[2]=0xE8+i;

    loadPacket(0,resetPacket,2,3);            // NMRA recommends starting with 3 reset packets
    loadPacket(0,bRead,3,5);                  // NMRA recommends 5 verify packets
    loadPacket(0, idlePacket, 2, 6);          // NMRA recommends 6 idle or reset packets for decoder recovery time

        for(int j=0;j<ACK_SAMPLE_COUNT;j++){
        c=(analogRead(hdwSettings.current_sense_pin)-base)*ACK_SAMPLE_SMOOTHING+c*(1.0-ACK_SAMPLE_SMOOTHING);
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
        base+=analogRead(hdwSettings.current_sense_pin);
    }
    base/=ACK_BASE_COUNT;

    bRead[0]=0x74+(highByte(cv)&0x03);     // set-up to re-verify entire byte
    bRead[2]=bValue;

    loadPacket(0,resetPacket,2,3);        // NMRA recommends starting with 3 reset packets
    loadPacket(0,bRead,3,5);              // NMRA recommends 5 verify packets
    loadPacket(0, idlePacket, 2, 6);      // NMRA recommends 6 idle or reset packets for decoder recovery time
    
    for(int j=0;j<ACK_SAMPLE_COUNT;j++){
        c=(analogRead(hdwSettings.current_sense_pin)-base)*ACK_SAMPLE_SMOOTHING+c*(1.0-ACK_SAMPLE_SMOOTHING);
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
}

void DCC::check() volatile {
    if(millis() - lastCheckTime > CURRENT_SAMPLE_TIME) {
		lastCheckTime = millis();
		current = analogRead(hdwSettings.current_sense_pin) * CURRENT_SAMPLE_SMOOTHING + current * (1.0 - CURRENT_SAMPLE_SMOOTHING);
		if(current > hdwSettings.trigger_value && digitalRead(hdwSettings.enable_pin)) {
			powerOff(false, true);
			triggered=true;
		} else if(current < hdwSettings.trigger_value && triggered) {
			powerOn();
			triggered=false;
		}
	}
}

void DCC::powerOn(bool announce) volatile {
	digitalWrite(hdwSettings.enable_pin, HIGH);
	if(announce) {
		CommManager::printf("<p1 %s>", hdwSettings.track_name);
	}
}

void DCC::powerOff(bool announce, bool overCurrent) volatile {
	digitalWrite(hdwSettings.enable_pin, LOW);
	if(announce) {
		if(overCurrent) {
			CommManager::printf("<p2 %s>", hdwSettings.track_name);
		} else {
			CommManager::printf("<p0 %s>", hdwSettings.track_name);
		}
	}
}

int DCC::getLastRead() volatile {
	return current;
}

void DCC::showStatus() volatile {
	if(digitalRead(hdwSettings.enable_pin) == LOW) {
		CommManager::printf("<p0 %s>", hdwSettings.track_name);
	} else {
		CommManager::printf("<p1 %s>", hdwSettings.track_name);
	}
}

uint8_t DCC::idlePacket[3]={0xFF,0x00,0};                 // always leave extra byte for checksum computation
uint8_t DCC::resetPacket[3]={0x00,0x00,0};