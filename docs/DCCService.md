# DCCService
## API Documentation
### Constructor 

```cpp
DCCService(Hardware hardware);
```

```hardware``` accepts a ```Hardware``` object, which defines the microcontroller timer, signal, and pin configuation. 

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

#### Static Factory Constructors
Alternatively, you can use a static factory contructor to set up the ```DCCService``` object. 

!!! info
    Currently, the following constructors exist. Note that the standard format for the constructor is Create_*MFG*\_*PRODUCT*\_*TRACK* if you plan to contribute another static constructor method.

```cpp
DCCService* Create_Arduino_L298Shield_Prog();
DCCService* Create_Pololu_MC33926Shield_Prog();
DCCService* Create_WSM_SAMCommandStation_Prog();
``` 

### DCCService::writeCVByte
```cpp
uint8_t writeCVByte(uint16_t cv, uint8_t bValue, uint16_t callback, uint16_t callbackSub, void(*callbackFunc)(serviceModeResponse));
```

### DCCService::writeCVBit
```cpp
uint8_t writeCVBit(uint16_t cv, uint8_t bNum, uint8_t bValue, uint16_t callback, uint16_t callbackSub, void(*callbackFunc)(serviceModeResponse));
```

### DCCService::readCV
```cpp
uint8_t readCV(uint16_t cv, uint16_t callback, uint16_t callbackSub, void(*callbackFunc)(serviceModeResponse));
```
