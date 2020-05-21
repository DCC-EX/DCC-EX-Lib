#ifndef CommManager_h
#define CommManager_h

#include "../DCC/DCC.h"
#include "CommInterface.h"

class CommManager {
public:
	static void update();
	static void registerInterface(CommInterface *interface);
	static void showConfiguration();
	static void showInitInfo();
	static void printf(const char *fmt, ...);
private:
	static CommInterface *interfaces[5];
	static int nextInterface;
};

#endif