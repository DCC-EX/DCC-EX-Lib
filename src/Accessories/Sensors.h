#ifndef Sensor_h
#define Sensor_h

#include "Arduino.h"

#define  SENSOR_DECAY  0.03

struct SensorData {
    int snum;
    uint8_t pin;
    uint8_t pullUp;
};

struct Sensor{
    static Sensor *firstSensor;
    SensorData data;
    boolean active;
    float signal;
    Sensor *nextSensor;
    static void load();
    static void store();
    static Sensor *create(int, int, int, int=0);
    static Sensor* get(int);  
    static void remove(int);  
    static void show();
    static void status();
    static void parse(const char *c);
    static void check();   
};

#endif

