# DCC
The DCC layer provides functions for interacting with devices on the bus/track. Compared to the original DCC++ code, it does not perform string parsing operations and instead exposes an API with parameterized functions for each functionality in the original DCC++ code.
## API Documentation
### DCC Class
#### Constructor 

```cpp
DCC::DCC(int numDev, DCChdw hdw)
```

Creates an instance of the DCC class, and accepts parameters numDev and hdw. numDev defines the number of device slots (in DCC++ these were called registers) available to fill with devices. The maximum number of devices that can be added before running out of RAM is currently unknown.

!!! note
    **TODO** Test maximum number of devices on the SAMD21 platform

This constructor initializes the enable pin and sets it low. It then allocates memory for the device table and device map, as well as the speed table. It initializes the device table and contained packets so they are ready for interaction with the rest of the program. Finally, it loads the idle packet into device 1 and intitializes the timer that will generate the DCC signal (see ```DCC::init_timers()```).

##### DCChdw Struct
You will need to fill this struct and pass it to the DCC constructor, or use a static factory constructor as explained below.

```cpp
struct DCChdw {
    char * track_name;
    bool is_prog_track;
    bool use_dual_signal;
    uint8_t timer_num, gclk_num;
    uint8_t signal_a_pin, signal_a_group, signal_a_timer_bit, signal_a_timer_mux;
    uint8_t signal_b_pin, signal_b_group, signal_b_timer_bit, signal_b_timer_mux;
    uint8_t enable_pin;
    uint8_t current_sense_pin;
    int trigger_value;
};
```
A brief explanation of the parameters in the DCChdw struct:

- ```bool is_prog_track``` - set true if the track will be current limited to 250mA and function as a programming track. Note that currently CommManager and JMRI only support one programming track and one main track.
- ```bool use_dual_signal``` - if true, the dcc signal will be generate on two pins instead of one, 180 degrees out of phase with each other. Useful for chips like the DRV8876
- ```uint8_t timer_num``` - selects the instance of TCC that will be used to denerate the clock signal. Currently only TCC0-2 are supported by the CMSIS core for Arduino, TCC3 still needs to be implemented. 
- ```uint8_t gclk_num``` - selects the instance of GCLK (generic clock) that will be used to feed the TCC module. Typically clocks 4 and 5 are safe. Note that one clock can feet both TCC0 and TCC1, and a separate clock is needed for TCC2.
- ```uint8_t signal_a/b_pin``` - selects the SAMD21 pin numberused for DCC signal generation. Please refer to a SAMD21 datasheet to confirm that the pin you want to use has a TCC peripheral attached to it. If you are using the dual signal feature, both pins must be on different bits of the same TCC timer. Please also note that this is the SAMD21 pin, not a corresponding arduino pin number. signal_b_pin's value will be disregarded if the dual_signal feature is disabled.
- ```uint8_t signal_a/b_group``` - selects the port that the output pin number refers to. Zero is for port A and one refers to port B. No other options are available.
- ```uint8_t signal_a/b_timer_bit``` - defines which bit of the TCC peripheral the pin number corresponds to. In the pinmux table in the datasheet, this number is x in TCCy/WO[x]
- ```uint8_t signal_b_timer_mux``` - refer to the example static constructors for how to set this.
- ```uint8_t enable_pin``` - selects the Arduino pin used for enabling/disabling the motor shield. Note that unlike the signal pins, this is an Arduino pin and not a SAMD21 pin number
- ```uint8_t current_sense_pin``` - selects the Arduino pin used for current detection. Note that unlike the signal pins, this is an Arduino pin and not a SAMD21 pin number
- ```uint8_t trigger_value``` - selects the current value (in mA) at which the motor driver will be turned off

#### Static Factory Constructors
CommandStation implements a method of setting up the hardware that uses static factory constructors. This allows the user to select either a pre-configured hardware setup from the main.cpp file, or to roll their own with the DCChdw struct.

!!! info
    Currently, the following constructors exist. Note that the standard format for the constructor is Create_*MFG*\_*PRODUCT*\_*TRACK*.

```cpp
DCC* DCC::Create_WSM_SAMCommandStation_Main(int numDev)
DCC* DCC::Create_WSM_SAMCommandStation_Prog(int numDev);
```

#### DCC::loadPacket

```cpp
void DCC::loadPacket(int nDev, uint8_t *b, uint8_t nBytes, uint8_t nRepeat) volatile
```

loadPacket formats and inserts an incoming packet into the specified device (nDev) and sets it to be loaded by the interrupt on the next available opportunity. 

#### DCC::setThrottle

```cpp
int DCC::setThrottle(uint8_t nDev, uint16_t cab, uint8_t tSpeed, bool tDirection, setThrottleResponse& response) volatile
```

Sets the throttle of address ```cab```, held in device slot ```nDev```, to value ```tSpeed``` and direction tDirection. ```tSpeed``` has possible values from 0-127, with 0=>stop, and 1=>emergency stop. ```tDirection=0``` sets the device to reverse and ```tDirection=1``` sets the device forwards.

```response``` is a struct that is passed by reference into the function and filled by the function. 

!!! info
    **Response** You'll see that most of the DCC functions pass data back to the caller this way, instead of using the CommManager to print straight back to the communication buses like in DCC++. This allows the codebase to be more easily extensible to other methods of communication
    
    **Return** Most DCC functions will return an int that indicates the error status. Errors are defined in ```DCC.h```

!!! note
    **Other Applications** You probably won't need to set the direction or speed of most devices on the bus if you're using this library for home automation or other things. You can safely ignore this function and just use the other methods.

#### DCC::setFunction

```cpp
int DCC::setFunction(uint16_t cab, uint8_t byte1, setFunctionResponse& response) volatile
int DCC::setFunction(uint16_t cab, uint8_t byte1, uint8_t byte2, setFunctionResponse& response) volatile 
```

Sets functions of address ```cab``` to values determined by ```byte1```, and optionally ```byte2```.

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

#### DCC::setAccessory

```cpp
int DCC::setAccessory(uint16_t address, uint8_t number, bool activate, setAccessoryResponse& response) volatile
```

Turns an accessory (stationary) decoder on or off (```activate```). ```address``` and ```number``` correspond to address and subaddress. See the note below for details.

!!! info
    Note that many decoders and controllers combine the ADDRESS and SUBADDRESS into a single number, N, from  1 through a max of 2044, where

    N = (ADDRESS - 1) * 4 + SUBADDRESS + 1, for all ADDRESS>0
    
    OR

    ADDRESS = INT((N - 1) / 4) + 1
    SUBADDRESS = (N - 1) % 4

#### DCC::writeCVByteMain

```cpp
int DCC::writeCVByteMain(uint16_t cab, uint16_t cv, uint8_t bValue, writeCVByteMainResponse& response) volatile
```

#### DCC::writeCVBitMain

```cpp
int DCC::writeCVBitMain(uint16_t cab, uint16_t cv, uint8_t bNum, uint8_t bValue, writeCVBitMainResponse& response) volatile
```

#### DCC::writeCVByte

```cpp
int DCC::writeCVByte(uint16_t cv, uint8_t bValue, uint16_t callback, uint16_t callbackSub, writeCVByteResponse& response) volatile
```

#### DCC::writeCVBit

```cpp
int DCC::writeCVBit(uint16_t cv, uint8_t bNum, uint8_t bValue, uint16_t callback, uint16_t callbackSub, writeCVBitResponse& response) volatile
```

#### DCC::readCV

```cpp
int DCC::readCV(uint16_t cv, uint16_t callback, uint16_t callbackSub, readCVResponse& response) volatile
```

#### DCC::check

```cpp
void DCC::check() volatile
```

#### DCC::powerOn

```cpp
void DCC::powerOn() volatile
```

#### DCC::powerOff

```cpp
void DCC::powerOff() volatile
```

#### DCC::getLastRead

```cpp
int DCC::getLastRead() volatile
```