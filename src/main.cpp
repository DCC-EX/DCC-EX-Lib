#include <Arduino.h>
#include "CommandStation.h"

volatile DCC* mainTrack = DCC::Create_WSM_SAMCommandStation_Main(10);
volatile DCC* progTrack = DCC::Create_WSM_SAMCommandStation_Prog(2);

void setup() {
	CommManager::registerInterface(new SerialInterface(SerialUSB));
	CommParser::init(mainTrack, progTrack);
	CommManager::showInitInfo();
}

void loop() {
  	CommManager::update();
	mainTrack->check();
	progTrack->check();
}

void TCC0_Handler() {
	mainTrack->interrupt_handler();
}

void TCC1_Handler() {
	progTrack->interrupt_handler();
}