#ifndef COMMANDSTATION_COMMINTERFACE_COMMINTERFACESERIAL_H_
#define COMMANDSTATION_COMMINTERFACE_COMMINTERFACESERIAL_H_

#include "CommInterface.h"
#include <Arduino.h>
#include <HardwareSerial.h>

class SerialInterface : public CommInterface {
public:
	SerialInterface(HardwareSerial &serial, long baud=115200);
	void process();
	void showConfiguration();
	void showInitInfo();
	void send(const char *buf);
	Stream* getStream() { return &serialStream; }
protected:
	HardwareSerial &serialStream;
	long baud;
	String buffer;
	bool inCommandPayload;
};

#endif	// COMMANDSTATION_COMMINTERFACE_COMMINTERFACESERIAL_H_