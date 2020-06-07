#ifndef COMMANDSTATION_DCC_DCC_H_
#define COMMANDSTATION_DCC_DCC_H_

#include <Arduino.h>

#include "Hardware.h"
#include "Queue.h"

#if defined(ARDUINO_ARCH_SAMD)
extern Uart mainRailcomUART;
#endif

#define ERR_OK 0
#define ERR_OUT_OF_RANGE -1
#define ERR_BUSY -2

#define DCC_PACKET_MAX_SIZE 6 

// Define constants used for reading CVs from the Programming Track
const int ACK_SAMPLE_THRESHOLD = 30;    // the threshold (in mA) that the analogRead samples (after subtracting the baseline current) must cross to ACK

const uint8_t queueSize = 50;           // Optimal size is 50 for the programming track, 30 may be workable.

const uint8_t idlePacket[] = {0xFF, 0x00, 0xFF};
const uint8_t resetPacket[] = {0x00, 0x00, 0x00};
const uint8_t bitMask[] = {0x00,0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};

struct setThrottleResponse {
    int device;
    int speed;
    int direction;
    uint16_t transactionID;
};

struct setFunctionResponse {
    uint16_t transactionID;
};

struct setAccessoryResponse {
    uint16_t transactionID;
};

struct writeCVByteMainResponse {
    uint16_t transactionID;
};

struct writeCVBitMainResponse {
    uint16_t transactionID;
};

struct readCVMainResponse {
    int callback;
    int callbackSub;
    int cv;
    int bValue;
    uint16_t transactionID;
};

enum cv_edit_type {
    READCV,
    WRITECV,
    WRITECVBIT,
};

struct serviceModeResponse {
    cv_edit_type type;
    int callback;
    int callbackSub;
    int cv;
    int cvBitNum;
    int cvValue;
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

    // Call this function every 29us from the main code
    void interruptHandler();

    // Call this function every loop
    void loop() {
        updateSpeed();
        hdw.checkCurrent();
        if(hdw.getRailcomEnable()) {
            readRailcomData();
        }
        if(hdw.getIsProgTrack()) {
            checkAck();
        }
    }

    int setThrottle(uint8_t nDev, uint16_t cab, uint8_t tSpeed, bool tDirection, setThrottleResponse& response);
    int setFunction(uint16_t cab, uint8_t byte1, setFunctionResponse& response);
    int setFunction(uint16_t cab, uint8_t byte1, uint8_t byte2, setFunctionResponse& response);
    int setAccessory(uint16_t address, uint8_t number, bool activate, setAccessoryResponse& response);
    int writeCVByteMain(uint16_t cab, uint16_t cv, uint8_t bValue, writeCVByteMainResponse& response);
    int writeCVBitMain(uint16_t cab, uint16_t cv, uint8_t bNum, uint8_t bValue, writeCVBitMainResponse& response);
    int writeCVByte(uint16_t cv, uint8_t bValue, uint16_t callback, uint16_t callbackSub, void(*callbackFunc)(serviceModeResponse));
    int writeCVBit(uint16_t cv, uint8_t bNum, uint8_t bValue, uint16_t callback, uint16_t callbackSub, void(*callbackFunc)(serviceModeResponse));
    int readCV(uint16_t cv, uint16_t callback, uint16_t callbackSub, void(*callbackFunc)(serviceModeResponse));

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
    void checkAck();

    int nextDev;

    struct Packet {
        uint8_t payload[DCC_PACKET_MAX_SIZE];
        uint8_t length;
        uint8_t repeats;
        uint16_t transmitID;
    };

    
    Queue<Packet> packetQueue = Queue<Packet>(queueSize);

    // Data for the currently transmitted packet
    uint8_t bits_sent;
    uint8_t bytes_sent;
    bool currentBit;
    uint8_t transmitRepeats;
    uint8_t remainingPreambles;
    bool generateStartBit;  
    uint8_t transmitPacket[DCC_PACKET_MAX_SIZE];
    uint8_t transmitLength;
    uint16_t transmitID;

    // The ID of the last DCC packet to get processed (for railcom)
    uint16_t lastID;

    static uint16_t counterID;
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
    void schedulePacket(const uint8_t buffer[], uint8_t byteCount, uint8_t repeats, uint16_t identifier);

    // ACK detection stuff
    serviceModeResponse cvState;
    uint8_t ackBuffer;          // Keeps track of what the ack values are. 1 = ack 0 = nack
    uint8_t ackNeeded;          // Individual bits denote where we still need an ack.
    uint16_t ackPacketID[8];    // What packet IDs correspond to the ACKs
    uint8_t verifyPayload[4];   // Packet sent after acks are done to confirm CV read/write. [0-1] are set in 
                                // the caller function, [2] is modified in the checkAck function
    bool inVerify;              // Are we verifying something previously read/written?
    bool backToIdle;            // Have we gone back to idle packets after setting a CV instruction?
    void (*cvResponse)(serviceModeResponse);    // Callback function that returns response to comms API. Registered in CV functions.
};

#endif  // COMMANDSTATION_DCC_DCC_H_