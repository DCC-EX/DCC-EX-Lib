#ifndef CommInterface_h
#define CommInterface_h

class CommInterface {
public:
	virtual void process() = 0;
	virtual void showConfiguration() = 0;
	virtual void showInitInfo() = 0;
	virtual void send(const char *buf) = 0;
};

#endif