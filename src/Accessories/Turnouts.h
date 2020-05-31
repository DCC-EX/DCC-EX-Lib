#ifndef Turnouts_h
#define Turnouts_h

#include <Arduino.h>
#include "../DCC/DCC.h"

struct TurnoutData {
    uint8_t tStatus;
    uint8_t subAddress;
    int id;
    int address;  
};

struct Turnout{
    static Turnout *firstTurnout;
    int num;
    struct TurnoutData data;
    Turnout *nextTurnout;
    void activate(int s, DCC* track);
    static Turnout* get(int);
    static void remove(int);
    static void load();
    static void store();
    static Turnout *create(int, int, int, int=0);
    static void show(int=0);
};
  
#endif