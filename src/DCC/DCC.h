#ifndef DCC_h
#define DCC_h

#include <Arduino.h>
#include "Hardware.h"
#include "Queue.h"

#if defined(ARDUINO_ARCH_SAMD)
extern Uart mainRailcomUART;
#endif

#define ERR_OK 0
#define ERR_OUT_OF_RANGE -1

#define DCC_PACKET_MAX_SIZE 6 



// Todo: re-add noise cancelling on ACK
// Define constants used for reading CVs from the Programming Track
const int ACK_BASE_COUNT = 100;      // number of analogRead samples to take before each CV verify to establish a baseline current
const int ACK_SAMPLE_COUNT = 500;      // number of analogRead samples to take when monitoring current after a CV verify (bit or byte) has been sent 
const float ACK_SAMPLE_SMOOTHING = 0.2;      // exponential smoothing to use in processing the analogRead samples after a CV verify (bit or byte) has been sent
const int ACK_SAMPLE_THRESHOLD = 30;      // the threshold that the exponentially-smoothed analogRead samples (after subtracting the baseline current) must cross to establish ACKNOWLEDGEMENT

const byte idlePacket[] = {0xFF, 0x00, 0xFF};
const byte resetPacket[] = {0x00, 0x00, 0x00};
const byte bitMask[] = {0x00,0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};

struct setThrottleResponse {
    int device;
    int speed;
    int direction;
    uint32_t transactionID;
};

struct setFunctionResponse {
    uint32_t transactionID;
};

struct setAccessoryResponse {
    uint32_t transactionID;
};

struct writeCVByteMainResponse {
    uint32_t transactionID;
};

struct writeCVBitMainResponse {
    uint32_t transactionID;
};

struct readCVMainResponse {
    int callback;
    int callbackSub;
    int cv;
    int bValue;
    uint32_t transactionID;
};

struct writeCVByteResponse {
    int callback;
    int callbackSub;
    int cv;
    int bValue;
    // Doesn't need a transaction ID because this is for programming track
};

struct writeCVBitResponse {
    int callback;
    int callbackSub;
    int cv;
    int bNum;
    int bValue;
    // Doesn't need a transaction ID because this is for programming track
};

struct readCVResponse {
    int callback;
    int callbackSub;
    int cv;
    int bValue;
    // Doesn't need a transaction ID because this is for programming track
};

class DCC {
public:
    static DCC* Create_Arduino_L298Shield_Main(int numDev);
    static DCC* Create_Arduino_L298Shield_Prog(int numDev);
    
    static DCC* Create_Pololu_MC33926Shield_Main(int numDev);
    static DCC* Create_Pololu_MC33926Shield_Prog(int numDev);

    static DCC* Create_WSM_SAMCommandStation_Main(int numDev);
    static DCC* Create_WSM_SAMCommandStation_Prog(int numDev);

    DCC(int numDev, Hardware settings);

    // Call this function every 58us from the main code
    void interruptHandler();

    // Call this function every loop
    void loop() {
        updateSpeed();
        hdw.checkCurrent();
        if(hdw.enable_railcom) {
            readRailcomData();
        }
    }

    int setThrottle(uint8_t nDev, uint16_t cab, uint8_t tSpeed, bool tDirection, setThrottleResponse& response);
    int setFunction(uint16_t cab, uint8_t byte1, setFunctionResponse& response);
    int setFunction(uint16_t cab, uint8_t byte1, uint8_t byte2, setFunctionResponse& response);
    int setAccessory(uint16_t address, uint8_t number, bool activate, setAccessoryResponse& response);
    int writeCVByteMain(uint16_t cab, uint16_t cv, uint8_t bValue, writeCVByteMainResponse& response);
    int writeCVBitMain(uint16_t cab, uint16_t cv, uint8_t bNum, uint8_t bValue, writeCVBitMainResponse& response);
    int writeCVByte(uint16_t cv, uint8_t bValue, uint16_t callback, uint16_t callbackSub, writeCVByteResponse& response);
    int writeCVBit(uint16_t cv, uint8_t bNum, uint8_t bValue, uint16_t callback, uint16_t callbackSub, writeCVBitResponse& response);
    int readCV(uint16_t cv, uint16_t callback, uint16_t callbackSub, readCVResponse& response);

    int numDev;

    struct Speed {
        uint16_t cab;
        uint8_t speed;
        bool forward;           // Todo: move to a uint8_t because bool takes up 2 bytes
    };

    Speed* speedTable;

    Hardware hdw;

private:
    void updateSpeed();
    void readRailcomData();

    int nextDev;

    struct Packet {
        uint8_t payload[DCC_PACKET_MAX_SIZE];
        uint8_t length;
        uint8_t repeats;
        uint64_t transmitID;
    };

    Queue<Packet> packetQueue = Queue<Packet>(20);

    // Data for the currently transmitted packet
    uint8_t bits_sent;
    uint8_t bytes_sent;
    bool currentBit;
    uint8_t transmitRepeats;
    uint8_t remainingPreambles;
    bool generateStartBit;  
    uint8_t transmitPacket[DCC_PACKET_MAX_SIZE];
    uint8_t transmitLength;
    uint32_t transmitID;

    // The ID of the last DCC packet to get processed (for railcom)
    uint32_t lastID;

    static uint32_t counterID;
    static void incrementCounterID() { 
        counterID++;
        if(counterID == 0) counterID = 1;
    }

    // Waveform generator state
    uint8_t state;

    // Railcom cutout stuff
    volatile bool generateRailcomCutout;
    volatile bool inRailcomCutout;
    volatile bool railcomData;
    
    // Interrupt segments, called in interrupt_handler
    bool interrupt1();
    void interrupt2();
    void signal(bool pinA, bool pinB);

    // Loads buffer into the pending packet slot once it is empty.
    void schedulePacket(const uint8_t buffer[], uint8_t byteCount, uint8_t repeats, uint32_t identifier);
};

#endif