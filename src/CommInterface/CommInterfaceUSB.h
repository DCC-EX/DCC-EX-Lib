#ifndef CommInterfaceUSB_h
#define CommInterfaceUSB_h

#include "CommInterface.h"
#include <Arduino.h>

class USBInterface : public CommInterface {
public:
	USBInterface(Serial_ &serial, long baud=115200);	// Baud doesn't really matter but keep it for consistency.
	void process();
	void showConfiguration();
	void showInitInfo();
	void send(const char *buf);
	Stream* getStream() { return &serialStream; }
protected:
	Serial_ &serialStream;
	long baud;
	String buffer;
	bool inCommandPayload;
};

#endif