#if defined(ARDUINO_ARCH_SAMD)

#include <Arduino.h>
#include "CommInterfaceUSB.h"
#include "DCCEXParser.h"
#include "CommManager.h"
#include "../DCC/DCC.h"

USBInterface::USBInterface(Serial_ &serial, long baud) : serialStream(serial), baud(baud), buffer(""), inCommandPayload(false) {
	serialStream.begin(baud);
	serialStream.flush();
}

void USBInterface::process() {
	while(serialStream.available()) {
		char ch = serialStream.read();
		if (ch == '<') {
			inCommandPayload = true;
			buffer = "";
		} else if (ch == '>') {
			DCCEXParser::parse(buffer.c_str());
			buffer = "";
			inCommandPayload = false;
		} else if(inCommandPayload) {
			buffer += ch;
		}
	}
}

void USBInterface::showConfiguration() {
	serialStream.println("USB");
}

void USBInterface::showInitInfo() {
	CommManager::printf("<N0:SERIAL>");
}

void USBInterface::send(const char *buf) {
	serialStream.print(buf);
}

#endif