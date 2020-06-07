#ifndef COMMANDSTATION_COMMINTERFACE_COMMINTERFACE_H_
#define COMMANDSTATION_COMMINTERFACE_COMMINTERFACE_H_

class CommInterface {
public:
	virtual void process() = 0;
	virtual void showConfiguration() = 0;
	virtual void showInitInfo() = 0;
	virtual void send(const char *buf) = 0;
	virtual Stream* getStream() = 0;
};

#endif	// COMMANDSTATION_COMMINTERFACE_COMMINTERFACE_H_