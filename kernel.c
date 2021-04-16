// ITAI ZILBERSTEIN

#include<stdio.h>
#include<stdlib.h>
#include "pcb.h"
#include "ram.h"
#include "shell.h"
#include "cpu.h"
#include "memorymanager.h"
#include "diskdriver.h"

void totalReset();

// FOR TESTING
void display_RQ();

// Ready list NODE
typedef struct NODE {
  PCB* pcb;
  struct NODE* next;
} Node;

// Initializes HEAD and TAIL of ready list 
Node* head = NULL;
Node* tail = NULL;

// Adds a NODE with the inputted PCB to the back of the ready list
void addToReady(PCB* new_pcb) {
  Node* new_node = (Node*) malloc(sizeof(Node));
  new_node->pcb = new_pcb;
  new_node->next = NULL;
  if (tail == NULL) {
    head = new_node;
    tail = new_node;
  }
  else {
    Node* old_tail = tail;
    tail = new_node;
    old_tail->next = tail;
  }
}

// Removes the head of the ready list and returns the NODE's PCB
PCB* removeFromReady() {
  if (head == NULL)
    return NULL;
  Node* oldHead = head;
  head = oldHead->next;
  oldHead->next = NULL;
  if (oldHead == tail)
    tail = head;
  PCB* toReturn = oldHead->pcb;
  free(oldHead);
  return toReturn;
}

// Checks if there exists a victim PCB for the inputted frame
PCB* getVictimPCB(int frame) {
  Node* n = head;
  while(n != NULL){
    if(inPageTable(n->pcb, frame) != -1)
      return n->pcb;
    else
      n = n->next;
  }
  return NULL;
}

// Initializes a file -- opens it and calls launcher() to load into RAM, creates PCB and add to ready list
int myinit(char* filename) {
  FILE* f = fopen(filename, "r");
  if (f == NULL) {
    printf("Error: script %s not found\n", filename);
    totalReset();
    return 99;
  }
  return launcher(f);
}


// Terminates a PCB not in the ready list -- frees RAM and C memory   
void terminate(PCB* p) {
  for(int i=0; i<10; i++) {
    int frame = getFrame(p, i);
    removeFromRAM(frame*4, frame*4 + 3);
  }
  free(p);
}

// Fully resets RAM and ready list
void totalReset() { 
  resetRAM();
  while(head != NULL)
    removeFromReady();
}

// Page fault handler
int handlePageFault(PCB* p) {
  int error = 0;
  int next_page = getPage(p) + 1; // next page with instructions to execute
  int new_offset = 0;
  if (getFrame(p, next_page-1) == -1 && getPC_offset(p) < 4) { // case the process had its page replaced and it was not done with it
    next_page--; 
    new_offset = getPC_offset(p);
  }
  if (next_page >= getPagesMax(p)) { // EOF -- return 10 for 'quit'
    return 10;
  }
  int frame = getFrame(p, next_page); // check if frame for page in RAM
  if (frame == -1) { // next page not loaded -- need to access backing store
    error = diskToRAM(p, next_page);
    frame = getFrame(p,  next_page);
  }
  // update pcb accordingly
  setPage(p, next_page);
  setPC(p, frame*4);
  setPC_offset(p, new_offset);
  return error;    
}

// Executes concurrent programs
int scheduler () {
  int error = 0;
  //int count=1;  // FOR TESTING EXECUTION CYCLES
  //while (cpu.IP != -1); // CPU BUSY, IP = -1 means it is ready
  while (getIP() == -1 && (!error || error == 10)) { // IP = -1 means CPU is available and no errors

    //printf("-------------------BEFORE EXECUTION: %d-------------\n", count);
    //display_RAM();
    //display_RQ();

    PCB* p = removeFromReady(); // process to execute
    if (p == NULL){ // no process to execute (empty ready queue)
      resetRAM(); // reset RAM
      return 0; 
    }
    setIP(getPC(p)); // load PC into CPU
    setOffset(getPC_offset(p)); // load offset into CPU
    if (getFrame(p, getPage(p)) == -1) // case where page to execute not in RAM -- page fault and can't run
      error = -2;
    else
      error = run(getQuanta()); // execute for cpu's quanta
    if (error == -2) { // END OF PAGE / PAGE FAULT
      error = handlePageFault(p); 
    }
    else { // still valid instructions in page
      setPC(p, getIP());  // update PC of process as normal
      setPC_offset(p, getOffset()); // update offset of process as normal
    }
    if (error) // process finished or error occurs
      terminate(p); 
    else
      addToReady(p);
    setIP(-1); // CPU available

    //printf("-------------------AFTER EXECUTION: %d-------------\n", count);
    //display_RAM();
    //display_RQ();
    //count++;
  }
  // Only reached if ERROR occurs
  // Kills all running processes
  totalReset();
  return error;
}

// FOR TESTING
void display_RQ() {
  Node* t = head;
  while (t != NULL){
    display_PCB(t->pcb);
    t = t->next;
  }
}

// Loads kernel and calls shell
int kernel () {
  printf("Kernel 2.0 loaded!\n");
  initCPU();
  return shellUI();
}

// Initializes backing store -- note I assume the BackingStore directory always exists
void boot () {
  initRAM();
  initIO();
  system("rm -r BackingStore");
  system("mkdir BackingStore");
}

int main () {
  int error = 0;
  boot();
  error = kernel();
  return error;
}

