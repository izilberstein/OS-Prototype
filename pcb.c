// ITAI ZILBERSTEIN

#include<stdlib.h>
#include<stdio.h>

// PCB structure
typedef struct {
  int PC;
  int pageTable[10];
  int PC_page;
  int PC_offset;
  int pages_max;
  int pid; // to keep track of backing store file
} PCB;

// Mallocs a PCB with inputted number of pages and file -- returns pointer to PCB
PCB* makePCB(int pages, int pid) {
  PCB* new_pcb = (PCB*) malloc(sizeof(PCB));
  // set fields
  new_pcb->PC = 0;
  new_pcb->PC_page = 0;
  new_pcb->PC_offset = 0;
  new_pcb->pages_max = pages;
  new_pcb->pid = pid;
  // initialize page table (-1 in page table means page not loaded in RAM)
  for(int i=0; i<10; i++)
    new_pcb->pageTable[i] = -1;
  return new_pcb; 
}

// Returns the PC of inputted PCB
int getPC(PCB* p){
  return p->PC;
}

// Sets the PC of inputted PCB
void setPC(PCB* p, int pc){
  p->PC = pc;
}

// Returns the pid of the pcb
int getPID(PCB* p) {
  return p->pid;
}

// Returns the current page of p
int getPage(PCB* p) {
  return p->PC_page;
}

// Sets the page of inputted pcb to page
void setPage(PCB* p, int page) {
  p->PC_page = page;
}

// Returns the max pages of the inputted pcb
int getPagesMax(PCB* p) {
  return p->pages_max;
}

// Returns the frame of the inputted page
int getFrame(PCB* p, int page) {
  return p->pageTable[page % 10];
  // the % 10 allows for more than 10 pages
}

// Returns PC offset of inputted pcb
int getPC_offset(PCB* p) {
  return p->PC_offset;
}

// Sets offset of inputted pcb
void setPC_offset(PCB* p, int o) {
  p->PC_offset = o;
}

// Sets an entry in the given pcb's page table
void setPageTable(PCB* p, int pageNumber, int frameNumber) {
  p->pageTable[pageNumber % 10] = frameNumber;
  // the % 10 allows for more than 10 pages
}

// Chekcs if a frame is used by a pcb -- returns page number if yes, -1 if no
int inPageTable(PCB* p, int frame) {
  for (int i=0; i<10; i++) {
    if (p->pageTable[i] == frame)
      return i;
  }
  return -1;
}

// FOR TESTING
void display_PCB(PCB* pcb){
  printf("PID: %d with %d pages\n", pcb->pid,  pcb->pages_max);
  for(int i=0; i<10; i++)
    printf("(%d,%d)", i, pcb->pageTable[i]);
  printf("\n");
}

