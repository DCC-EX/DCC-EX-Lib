#include <Arduino.h>
#include "CommInterfaceSerial.h"
#include "StringParser.h"
#include "CommManager.h"
#include "../DCC/DCC.h"

SerialInterface::SerialInterface(Serial_ &serial, long baud) : serialStream(serial), baud(baud), buffer(""), inCommandPayload(false) {
	serialStream.begin(baud);
	serialStream.flush();
}

void SerialInterface::process() {
	while(serialStream.available()) {
		char ch = serialStream.read();
		if (ch == '<') {
			inCommandPayload = true;
			buffer = "";
		} else if (ch == '>') {
			StringParser::parse(buffer.c_str());
			buffer = "";
			inCommandPayload = false;
		} else if(inCommandPayload) {
			buffer += ch;
		}
	}
}

void SerialInterface::showConfiguration() {
	serialStream.print("Hardware Serial - Speed:");
	serialStream.println(baud);
}

void SerialInterface::showInitInfo() {
	CommManager::printf("<N0:SERIAL>");
}

void SerialInterface::send(const char *buf) {
	serialStream.print(buf);
}