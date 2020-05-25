#include "CommManager.h"
#include <Arduino.h>

#if defined (ATSAMD21G)
#include <cstdarg>
#endif

CommInterface *CommManager::interfaces[5] = {NULL, NULL, NULL, NULL, NULL};
int CommManager::nextInterface = 0;

void CommManager::update() {
	for(int i = 0; i < nextInterface; i++) {
		if(interfaces[i] != NULL) {
			interfaces[i]->process();
		}
	}
}

void CommManager::registerInterface(CommInterface *interface) {
	if(nextInterface < 5) {
		interfaces[nextInterface++] = interface;
	}
}

void CommManager::showConfiguration() {
	for(int i = 0; i < nextInterface; i++) {
		if(interfaces[i] != NULL) {
			interfaces[i]->showConfiguration();
		}
	}
}

void CommManager::showInitInfo() {
	for(int i = 0; i < nextInterface; i++) {
		if(interfaces[i] != NULL) {
			interfaces[i]->showInitInfo();
		}
	}
}

void CommManager::printf(const char *fmt, ...) {
	char buf[256] = {0};
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	for(int i = 0; i < nextInterface; i++) {
		if(interfaces[i] != NULL) {
			interfaces[i]->send(buf);
		}
	}
}