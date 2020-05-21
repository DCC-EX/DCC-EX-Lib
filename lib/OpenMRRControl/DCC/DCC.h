#ifndef DCC_h
#define DCC_h

#include <Arduino.h>
#include "DCChdw.h"
#include "Device.h"

#define ERR_OK 0
#define ERR_OUT_OF_RANGE -1

// Define constants used for reading CVs from the Programming Track

#define  ACK_BASE_COUNT            100      // number of analogRead samples to take before each CV verify to establish a baseline current
#define  ACK_SAMPLE_COUNT          500      // number of analogRead samples to take when monitoring current after a CV verify (bit or byte) has been sent 
#define  ACK_SAMPLE_SMOOTHING      0.2      // exponential smoothing to use in processing the analogRead samples after a CV verify (bit or byte) has been sent
#define  ACK_SAMPLE_THRESHOLD       30      // the threshold that the exponentially-smoothed analogRead samples (after subtracting the baseline current) must cross to establish ACKNOWLEDGEMENT

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

    DCChdw hdwSettings;
    
    /// Constructor for DCC class.
    /// @param hdwSettings Pin numbers, timing, etc needed for DCC signal generation. Use static 
    ///     factory constructors to generate or roll your own with the DCChdw struct.
    DCC(int numDev, DCChdw hdwSettings);

    void init_timers();
    void interrupt_handler() volatile;
    void loadPacket(int, uint8_t *, uint8_t, uint8_t) volatile;
    
    int setThrottle(uint8_t, uint16_t, uint8_t, bool, setThrottleResponse&) volatile;
    int setFunction(uint16_t, uint8_t, setFunctionResponse&) volatile;
    int setFunction(uint16_t, uint8_t, uint8_t, setFunctionResponse&) volatile;
    int setAccessory(uint16_t, uint8_t, bool, setAccessoryResponse&) volatile;
    int writeCVByteMain(uint16_t, uint16_t, uint8_t, writeCVByteMainResponse&) volatile;
    int writeCVBitMain(uint16_t, uint16_t, uint8_t, uint8_t, writeCVBitMainResponse&) volatile;
    int writeCVByte(uint16_t, uint8_t, uint16_t, uint16_t, writeCVByteResponse&) volatile;
    int writeCVBit(uint16_t, uint8_t, uint8_t, uint16_t, uint16_t, writeCVBitResponse&) volatile;
    int readCV(uint16_t, uint16_t, uint16_t, readCVResponse&) volatile;

    void check() volatile;
    void powerOn() volatile;
    void powerOff() volatile;
    int getLastRead() volatile;
    void showStatus() volatile;

    int numDev;

    /// Device list (pointer to first) and map
    Device* dev;
    Device** devMap;
    
    Device* maxLoadedDev;
    Device* currentDev;
    Device* nextDev;
    Device* lastDev;        // Needed for railcom implementation
    
    Packet* tempPacket;
    Packet* constructionPacket;
    
    uint8_t currentBit;
    uint8_t nRepeat;
    
    int* speedTable;

    static uint8_t idlePacket[];
    static uint8_t resetPacket[];
    static uint8_t bitMask[];

    float reading;
	float current;
	bool tripped;
	long int lastCheckTime;
    long int lastTripTime;
};



#endif