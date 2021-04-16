// ITAI ZILBERSTEIN

#include<stdlib.h>
#include<string.h>
#include "interpreter.h"
#include "ram.h"

typedef struct {
  int IP;
  char IR[1000];
  int quanta;
  int offset;
} CPU;

// Our single core CPU
CPU* cpu;

// Runs whatever is loaded into the cpu for quanta lines
// Return code -2 signals that the page fault handler needed
int run (int quanta) {
  for(int i=0; i<quanta; i++) {
    if (cpu->offset == 4) // page fault
      return -2;
    char* instruction = getRAM(cpu->IP + cpu->offset);
    if (instruction == NULL) // EOF
      return -2;
    // no error -- line to execute
    strcpy(cpu->IR, instruction);
    cpu->offset += 1;
    int error = interpreter(cpu->IR);
    if (error != 0)
      return error;
  }
  return 0;
}

// Sets the IP of the CPU
void setIP(int ip) {
  cpu->IP = ip;
}

// Returns IP in CPU
int getIP() {
  return cpu->IP;
}

// Returns IP in CPU
int getQuanta() {
  return cpu->quanta;
}

// Set the offset of the CPU
void setOffset(int o){
  cpu->offset = o;
}

// Get the offset in CPU
int getOffset() {
  return cpu->offset;
}

// Initializes cpu
void initCPU(){
  cpu = (CPU*) malloc(sizeof(CPU));
  cpu->quanta = 2; // change CPU quanta
  cpu->IP = -1; // available CPU
  cpu->offset = 0;
}

