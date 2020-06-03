#ifndef CommInterfaceSerial_h
#define CommInterfaceSerial_h

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

#endif