#ifndef JMRIParser_h
#define JMRIParser_h

#include <Arduino.h>
#include "../DCC/DCC.h"

struct DCCEXParser
{
    static DCC *mainTrack, *progTrack;
    static void init(DCC* mainTrack_, DCC* progTrack_);
    static void parse(const char *);
    static void cvResponse(serviceModeResponse response);
private:
    static int stringParser(const char * com, int result[]);
    static const int MAX_PARAMS=10; 
    static int p[MAX_PARAMS];
};

#endif