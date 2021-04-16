#include <stdio.h>
#ifndef RAM_H
#define RAM_H
void addToRAM(FILE* program, int* start, int* end);
void removeFromRAM(int start, int end);
char* getRAM(int address);
void resetRAM();
void initRAM();
void setRAM(int address, char buffer[]);
int searchRAM();
// FOR TESTING
void display_RAM();
#endif

