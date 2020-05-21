#ifndef Outputs_h
#define Outputs_h

#include <Arduino.h>

struct OutputData {
  uint8_t oStatus;
  uint8_t id;
  uint8_t pin; 
  uint8_t iFlag; 
};

struct Output{
  static Output *firstOutput;
  int num;
  struct OutputData data;
  Output *nextOutput;
  void activate(int s);
  static void parse(const char *c);
  static Output* get(int);
  static void remove(int);
  static void load();
  static void store();
  static Output *create(int, int, int, int=0);
  static void show(int=0);
}; // Output
  
#endif