#ifndef CommParser_h
#define CommParser_h

#include <Arduino.h>
#include "../DCC/DCC.h"

struct StringParser
{
    static DCC *mainTrack, *progTrack;
    static void init(DCC* mainTrack_, DCC* progTrack_);
    static void parse(const char *);
};

#endif