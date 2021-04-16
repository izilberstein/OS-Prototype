#ifndef KERNEL_H
#define KERNEL_H
int myinit(char* filename);
int scheduler();
#include "pcb.h"
PCB* getVictimPCB(int frame);
void addToReady(PCB* p);
void totalReset();
// FOR TESTING
void display_RQ();
#endif
