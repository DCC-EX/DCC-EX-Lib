#ifndef COMMANDSTATION_COMMINTERFACE_COMMMANAGER_H_
#define COMMANDSTATION_COMMINTERFACE_COMMMANAGER_H_

#include "../DCC/DCC.h"
#include "CommInterface.h"

class CommManager {
public:
	static void update();
	static void registerInterface(CommInterface *interface);
	static void showConfiguration();
	static void showInitInfo();
	static void printf(const char *fmt, ...);
	static void printf(const __FlashStringHelper* fmt, ...);
private:
	static CommInterface *interfaces[5];
	static int nextInterface;
};

#endif	// COMMANDSTATION_COMMINTERFACE_COMMMANAGER_H_