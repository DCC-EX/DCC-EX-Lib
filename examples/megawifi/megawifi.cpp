
#include <Arduino.h>
#include <ArduinoTimers.h>
#include "../../src/CommandStation.h"

#ifndef COMMANDSTATION_DCC_CONFIG
#define COMMANDSTATION_DCC_CONFIG

// Choose the motor shield that you want to use.

//#define CONFIG_WSM_FIREBOX
#define CONFIG_ARDUINO_MOTOR_SHIELD
//#define CONFIG_POLOLU_MOTOR_SHIELD

//Define wifi settings here
#define CONFIG_ENABLE_WIFI
#define CONFIG_WIFI_SSID "ExampleWifi"
#define CONFIG_WIFI_PASSWORD "Example"
#define CONFIG_WIFI_PORT 3252
#endif

const uint8_t kIRQmicros = 29;
const uint8_t kNumLocos = 50;

DCCMain *mainTrack = DCCMain::Create_Arduino_L298Shield_Main(kNumLocos);
DCCService *progTrack = DCCService::Create_Arduino_L298Shield_Prog();

void waveform_IrqHandler()
{
  mainTrack->interruptHandler();
  progTrack->interruptHandler();
}

void setup()
{
  mainTrack->setup();
  progTrack->setup();

  // TimerA is TCC0 on SAMD21, Timer1 on MEGA2560, and Timer1 on MEGA328
  // We will fire an interrupt every 29us to generate the signal on the track
  TimerA.initialize();
  TimerA.setPeriod(kIRQmicros);
  TimerA.attachInterrupt(waveform_IrqHandler);
  TimerA.start();

#if defined(ARDUINO_ARCH_SAMD)
  CommManager::registerInterface(new USBInterface(SerialUSB));
  Wire.begin(); // Needed for EEPROM to work
#elif defined(ARDUINO_ARCH_AVR)
  CommManager::registerInterface(new SerialInterface(Serial));
#endif

#if defined(CONFIG_ENABLE_WIFI)
  Serial1.begin(115200);
  WifiInterface::setup(Serial1, F(CONFIG_WIFI_SSID), F(CONFIG_WIFI_PASSWORD), CONFIG_WIFI_PORT);
#endif

  EEStore::init();

  // Set up the string parser to accept commands from the interfaces
  DCCEXParser::init(mainTrack, progTrack);

  CommManager::showInitInfo();
}

void loop()
{
  CommManager::update();
  mainTrack->loop();
  progTrack->loop();
}
