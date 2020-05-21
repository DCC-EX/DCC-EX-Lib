#ifndef CommParser_h
#define CommParser_h

#include <Arduino.h>
#include "../DCC/DCC.h"

struct CommParser
{
    static volatile DCC *mainTrack, *progTrack;
    static void init(volatile DCC*, volatile DCC*);
    static void parse(const char *);
};

#endif