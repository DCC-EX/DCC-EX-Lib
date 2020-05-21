#ifndef CommInterfaceSerial_h
#define CommInterfaceSerial_h

#include "CommInterface.h"
#include <Arduino.h>

class SerialInterface : public CommInterface {
public:
	SerialInterface(Serial_ &serial, long baud=115200);
	void process();
	void showConfiguration();
	void showInitInfo();
	void send(const char *buf);
protected:
	Serial_ &serialStream;
	long baud;
	String buffer;
	bool inCommandPayload;
};

#endif /* COMMINTERFACESERIAL_H_ */