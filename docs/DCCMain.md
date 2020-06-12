# DCCMain
## API Documentation
### Constructor 

```cpp
DCCMain(uint8_t numDevices, Hardware hardware, Railcom railcom);
```

This constructor clears the packet queue and sets up the speed table so it is ready to interact with the rest of the program.

```numDevices``` defines the number of device slots (in DCC++ these were called registers) available. The maximum number of devices that can be added before running out of RAM is currently unknown, though it has been tested with 50 slots on an Arduino Uno. Microcontrollers with more RAM can handle more slots. 

!!! note
    **TODO** Test maximum number of devices on each platform.

```hardware``` accepts a ```Hardware``` object, which defines the microcontroller timer, signal, and pin configuation. 

```railcom``` accepts a ```Railcom``` object, which defines pin numbers and serial configuration for railcom. 

#### Hardware
You will need to create an instance of this struct, fill it, and pass it to either ```DCCMain``` or ```DCCService```. The following functions can be called on your instance of the ```Hardware``` struct to set it up:

```cpp
// General config modification
void config_setChannelName(const char *name);
void config_setControlScheme(control_type_t scheme);
void config_setPreambleBits(uint8_t preambleBits);

// Pin config modification
void config_setPinSignalA(uint8_t pin);
void config_setPinSignalB(uint8_t pin);
void config_setDefaultSignalB(bool default_state);
void config_setPinEnable(uint8_t pin);
void config_setPinCurrentSense(uint8_t pin);

// Current config modification
void config_setTriggerValue(int triggerValue);
void config_setMaxValue(int maxValue);
void config_setAmpsPerVolt(float ampsPerVolt);
```

**TODO** Describe these functions

#### Railcom
You will need to create an instance of this struct, fill it, and pass it to either ```DCCMain``` or ```DCCService```. The following functions can be called on your instance of the ```Railcom``` struct to set it up:

```cpp
// Railcom config modification
void config_setEnable(uint8_t isRailcom);
void config_setRxPin(uint8_t pin);
void config_setTxPin(uint8_t pin);

#if defined(ARDUINO_ARCH_SAMD) 
void config_setSerial(Uart* serial);
void config_setSercom(SERCOM* sercom);
void config_setRxMux(EPioType mux);
void config_setRxPad(SercomRXPad pad);
void config_setTxPad(SercomUartTXPad pad);
void config_setDACValue(uint8_t value);
#else
void config_setSerial(HardwareSerial* serial);
#endif

void config_setPOMResponseCallback(void (*_POMResponse)(RailcomPOMResponse));
```

**TODO** Describe these functions

#### Static Factory Constructors
Alternatively, you can use a static factory contructor to set up the ```DCCMain``` object. 

!!! info
    Currently, the following constructors exist. Note that the standard format for the constructor is Create_*MFG*\_*PRODUCT*\_*TRACK* if you plan to contribute another static constructor method.

```cpp
DCCMain* Create_Arduino_L298Shield_Main(uint8_t numDevices);
DCCMain* Create_Pololu_MC33926Shield_Main(uint8_t numDevices);
DCCMain* Create_WSM_SAMCommandStation_Main(uint8_t numDevices);
``` 

### DCCMain::setThrottle

```cpp
uint8_t setThrottle(uint8_t slot, uint16_t addr, uint8_t speed, uint8_t direction, setThrottleResponse& response);
```

Sets the throttle of address ```addr```, held in device slot ```slot```, to value ```speed``` and direction ```direction```. ```speed``` has possible values from 0-127, with 0=stop, and 1=emergency stop. ```direction=0``` sets the device to reverse and ```direction=1``` sets the device forwards.

```response``` is a struct that is passed by reference into the function and filled by the function. 

!!! info
    **Response** You'll see that most of the DCC functions pass data back to the caller this way, instead of using the CommManager to print straight back to the communication buses like in DCC++. This allows the codebase to be more easily extensible to other methods of communication
    
    **Return** Most DCC functions will return an int that indicates the error status. Errors are defined in ```Waveform.h```

### DCCMain::setFunction

```cpp
uint8_t setFunction(uint16_t addr, uint8_t byte1, setFunctionResponse& response);
uint8_t setFunction(uint16_t addr, uint8_t byte1, uint8_t byte2, setFunctionResponse& response);
```

Sets functions of address ```addr``` to values determined by ```byte1```, and optionally ```byte2```.

!!! info
    *   To set functions F0-F4 on (=1) or off (=0):
        * BYTE1:  128 + F1\*1 + F2\*2 + F3\*4 + F4\*8 + F0\*16
        * BYTE2:  omitted

    * To set functions F5-F8 on (=1) or off (=0):
        * BYTE1:  176 + F5\*1 + F6\*2 + F7\*4 + F8\*8
        * BYTE2:  omitted

    * To set functions F9-F12 on (=1) or off (=0):
        * BYTE1:  160 + F9\*1 +F10\*2 + F11\*4 + F12\*8
        * BYTE2:  omitted

    * To set functions F13-F20 on (=1) or off (=0):
        * BYTE1: 222
        * BYTE2: F13\*1 + F14\*2 + F15\*4 + F16\*8 + F17\*16 + F18\*32 + F19\*64 + F20\*128

    * To set functions F21-F28 on (=1) of off (=0):
        * BYTE1: 223
        * BYTE2: F21\*1 + F22\*2 + F23\*4 + F24\*8 + F25\*16 + F26\*32 + F27\*64 + F28\*128

### DCCMain::setAccessory

```cpp
uint8_t setAccessory(uint16_t addr, uint8_t number, bool activate, setAccessoryResponse& response);
```

Turns an accessory (stationary) decoder on or off (```activate```). ```addr``` and ```number``` correspond to address and subaddress, respectively. See the note below for details.

!!! info
    Note that many decoders and controllers combine the ADDRESS and SUBADDRESS into a single number, N, from  1 through a max of 2044, where

    N = (ADDRESS - 1) * 4 + SUBADDRESS + 1, for all ADDRESS>0
    
    OR

    ADDRESS = INT((N - 1) / 4) + 1
    SUBADDRESS = (N - 1) % 4

### DCCMain::writeCVByteMain

```cpp
uint8_t writeCVByteMain(uint16_t addr, uint16_t cv, uint8_t bValue, writeCVByteMainResponse& response, void (*POMCallback)(RailcomPOMResponse));
```

### DCCMain::writeCVBitMain

```cpp
uint8_t writeCVBitMain(uint16_t addr, uint16_t cv, uint8_t bNum, uint8_t bValue, writeCVBitMainResponse& response, void (*POMCallback)(RailcomPOMResponse));
```

### DCCMain::readCVByteMain

```cpp
uint8_t readCVByteMain(uint16_t addr, uint16_t cv, readCVByteMainResponse& response, void (*POMCallback)(RailcomPOMResponse));
```

### DCCMain::readCVBytesMain

```cpp
uint8_t readCVBytesMain(uint16_t addr, uint16_t cv, readCVBytesMainResponse& response, void (*POMCallback)(RailcomPOMResponse));
```

