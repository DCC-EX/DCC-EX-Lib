#ifndef DCC_h
#define DCC_h

#include <Arduino.h>
#include "DCChdw.h"
#include <ArduinoTimers.h>

#define ERR_OK 0
#define ERR_OUT_OF_RANGE -1

// Define constants used for reading CVs from the Programming Track

#define  ACK_BASE_COUNT            100      // number of analogRead samples to take before each CV verify to establish a baseline current
#define  ACK_SAMPLE_COUNT          500      // number of analogRead samples to take when monitoring current after a CV verify (bit or byte) has been sent 
#define  ACK_SAMPLE_SMOOTHING      0.2      // exponential smoothing to use in processing the analogRead samples after a CV verify (bit or byte) has been sent
#define  ACK_SAMPLE_THRESHOLD       30      // the threshold that the exponentially-smoothed analogRead samples (after subtracting the baseline current) must cross to establish ACKNOWLEDGEMENT

const int DCC_ZERO_BIT_TOTAL_DURATION = 2400;
const int DCC_ZERO_BIT_PULSE_DURATION = 1200;
const int DCC_ONE_BIT_TOTAL_DURATION = 1392;
const int DCC_ONE_BIT_PULSE_DURATION = 696;

const int RC_PULSE_DURATION = 348; // 29us
const int RC_CUTOUT_DURATION = 5220; // 435us

const byte idlePacket[] = {0xFF, 0x00, 0xFF};
const byte resetPacket[] = {0x00, 0x00, 0x00};
const byte bitMask[] = {0x00,0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};

#define DCC_PACKET_MAX_SIZE 6 

struct setThrottleResponse {
    int device;
    int speed;
    int direction;
};

struct writeCVByteResponse {
    int callback;
    int callbackSub;
    int cv;
    int bValue;
};

struct writeCVBitResponse {
    int callback;
    int callbackSub;
    int cv;
    int bNum;
    int bValue;
};

struct readCVResponse {
    int callback;
    int callbackSub;
    int cv;
    int bValue;
};

struct setFunctionResponse {
    // Fill later
};

struct setAccessoryResponse {
    // Fill later
};

struct writeCVByteMainResponse {
    // Fill later
};

struct writeCVBitMainResponse {
    // Fill later
};

class DCC {
public:
    static DCC* Create_WSM_SAMCommandStation_Main(int numDev);
    static DCC* Create_WSM_SAMCommandStation_Prog(int numDev);

    DCChdw hdw;
    
    /// Constructor for DCC class.
    /// @param hdwSettings Pin numbers, timing, etc needed for DCC signal generation. Use static 
    ///     factory constructors to generate or roll your own with the DCChdw struct.
    DCC(int numDev, DCChdw hdwSettings, Timer *int_timer);

    void init_timers();
    void interrupt_handler();

    bool interrupt1();
    void interrupt2();
    void schedulePacket(const uint8_t buffer[], uint8_t byteCount, uint8_t repeats);
    
    int setThrottle(uint8_t nDev, uint16_t cab, uint8_t tSpeed, bool tDirection, setThrottleResponse& response);
    int setFunction(uint16_t cab, uint8_t byte1, setFunctionResponse& response);
    int setFunction(uint16_t cab, uint8_t byte1, uint8_t byte2, setFunctionResponse& response);
    int setAccessory(uint16_t address, uint8_t number, bool activate, setAccessoryResponse& response);
    int writeCVByteMain(uint16_t cab, uint16_t cv, uint8_t bValue, writeCVByteMainResponse& response);
    int writeCVBitMain(uint16_t cab, uint16_t cv, uint8_t bNum, uint8_t bValue, writeCVBitMainResponse& response);
    int writeCVByte(uint16_t cv, uint8_t bValue, uint16_t callback, uint16_t callbackSub, writeCVByteResponse& response);
    int writeCVBit(uint16_t cv, uint8_t bNum, uint8_t bValue, uint16_t callback, uint16_t callbackSub, writeCVBitResponse& response);
    int readCV(uint16_t cv, uint16_t callback, uint16_t callbackSub, readCVResponse& response);

    void check();
    void powerOn();
    void powerOff();
    int getLastRead();
    void showStatus();

    int numDev;
    
    uint8_t bits_sent;
    uint8_t bytes_sent;
    bool currentBit;
    uint8_t transmitRepeats;
    uint8_t remainingPreambles;
    bool generateStartBit;  
    uint8_t transmitPacket[DCC_PACKET_MAX_SIZE];
    uint8_t transmitLength;
    
    bool packetPending;
    uint8_t pendingLength;
    uint8_t pendingPacket[DCC_PACKET_MAX_SIZE];
    uint8_t pendingRepeats;

    int* speedTable;

    float reading;
	float current;
	bool tripped;
	long int lastCheckTime;
    long int lastTripTime;

    uint8_t state;

    Timer* int_timer;
};



#endif