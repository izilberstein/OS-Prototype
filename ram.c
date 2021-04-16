// ITAI ZILBERSTEIN

#include<stdlib.h>
#include<string.h>
#include<stdio.h>

#define RAM_SIZE 40  // RAM size      
char* ram[RAM_SIZE];
int next_free = 0; // index of next free ram cell

// loads a program into ram line at a time   
void addToRAM(FILE* program, int* start, int* end) {
  *start = next_free; // where we start loading into ram
  char buffer[999];
  fgets(buffer, 999, program);
  while(!feof(program)){
    if (next_free > RAM_SIZE - 1 || ram[next_free] != NULL) { *end = -1; return;} // no valid index for end
    ram[next_free] = strdup(buffer);
    next_free++;
    fgets(buffer, 999, program);
  }
  *end = next_free-1;
}

// reset RAM from start to end
void removeFromRAM(int start, int end){
  for (int i=start; i<=end; i++)
    ram[i] = NULL;
}

// gets the string stored at address in RAM
char* getRAM(int address) {
  return ram[address];
}

// reset the next free cell in RAM to 0
void resetRAM() {
  removeFromRAM(0, RAM_SIZE-1);
  next_free = 0;
}

// sets all lines of RAM to NULL
void initRAM() {
  for (int i=0; i<RAM_SIZE; i++)
    ram[i] = NULL;
}

// sets RAM at the inputted address to the value in the buffer
void setRAM(int address, char buffer[]) {
  ram[address] = strdup(buffer);
}

// searches RAM for the first free frame -- returns frame number or -1 if none free
int searchRAM() {
  for(int frame=0; frame<10; frame++){
    if (ram[frame*4] == NULL)
      return frame;
  }
  return -1;
}
// FOR TESTING
void display_RAM() {
  printf("RAM:\n");
  for(int i=0; i<RAM_SIZE; i++){
    if (ram[i] != NULL)
      printf("%-20d : %s\n", i, ram[i]);
  }
}
