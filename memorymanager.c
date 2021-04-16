// ITAI ZILBERSTEIN

#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<time.h>
#include "pcb.h"
#include "ram.h"
#include "kernel.h"

int backingStoreCount = 0; // counts backing store files -- also used for pid

// Copies all lines from src to dest
void copy (FILE *dest, FILE *src) {
  char c = fgetc(src);
  while (c != EOF) {
    fputc(c, dest);
    c = fgetc(src);
  }
  rewind(dest);
}

// Returns number of pages required for program f by counting number of new lines 
int countTotalPages (FILE* f) {
  int lines = 0; 
  char c = fgetc(f);
  while (c != EOF) {
    if (c == '\n')
      lines++;
    c = fgetc(f);
  }
  int pages = lines / 4;
  if (lines % 4 > 0)
    pages++;
  rewind(f);
  return pages;
}

// Loads the pageNumber page of the file f into the frame in RAM
void loadPage (int pageNumber, FILE *f, int frameNumber) {
  char buffer[999];
  fgets(buffer, 999, f);
  // get to correct line in file
  int i=0;
  while(!feof(f) && i<pageNumber*4){
    fgets(buffer, 999, f);
    i++;
  }
  // fill frame in RAM
  int j=0;
  while(!feof(f) && j<4){
    setRAM(frameNumber*4 + j, buffer);
    fgets(buffer, 999, f);
    j++;
  }
  // EOF - not filling full frame so fill rest of frame with NULL
  removeFromRAM(frameNumber*4 + j, frameNumber*4 + 3);
  rewind(f);
}

// Returns first empty frame in RAM
int findFrame () {
  return searchRAM();
}

// Finds a victim frame not in the inputted pcb
int findVictim (PCB *p) {
  // picks a victim frame
  int victimFrame = rand() % 10;
  if (inPageTable(p, victimFrame) == -1)
    return victimFrame;
  // frame in pcb, find one that is not
  for(int k=(victimFrame+1)%10; k != victimFrame; k=(k+1)%10) {
    if(inPageTable(p, k) == -1) {
      return k;
    }
  }
  // all frames in pcb
  return -1;
}

// Updates the page table of a pcb
int updatePageTable(PCB *p, int pageNumber, int frameNumber, int victimFrame) {
  // if victim frame not in p, p is not victim pcb, so update page table
  if (victimFrame == -1)
    setPageTable(p, pageNumber, frameNumber);
  // if victim frame in p, set that to -1 as p is a victim pcb   
  else {
    int victimPage = inPageTable(p, victimFrame);
    if (victimPage != -1)
      setPageTable(p, victimPage, -1);
  }
  return 0;
}

// Selects a victim frame and upadtes its PCB -- returns newly freed frame 
int victimize(PCB* p){
  int free_frame = findVictim(p);
  PCB* vic = getVictimPCB(free_frame);
  if (vic != NULL)
    updatePageTable(vic, -1, -1, free_frame);
  return free_frame;
}

// Simple minimum function
static int min(int x, int y) { if (x<y) {return x;} return y;}

// Returns a string containing the path to the file with given PID in backing store (the path is BackingStore/PID_#)
char* backingStorePath(int pid) {
  char fileNumber[100];
  snprintf(fileNumber, 100, "%d", pid);
  char* bs = "BackingStore/PID_";
  char* path = (char*)  malloc(sizeof(bs)+sizeof(fileNumber));
  strcpy(path, bs);
  strcat(path, fileNumber);
  return path;
}

// Full operation of storing a pcb page from backing store to RAM
int diskToRAM(PCB* p, int page) {
  int free_frame = findFrame();
  if (free_frame == -1) {
    free_frame = victimize(p);
  }
  FILE* p_backingStore = fopen(backingStorePath(getPID(p)), "r");
  loadPage(page, p_backingStore, free_frame);
  fclose(p_backingStore);
  return updatePageTable(p, page, free_frame, -1);
}

// Launches the inputted program by copying to backing store, creating pcb, loading into ram
int launcher (FILE* p) {
  // creating path and new file name for backing store file
  int error = 0;
  int new_pid = backingStoreCount++;
  // backing store file to copy into
  FILE *newFile = fopen(backingStorePath(new_pid), "w+");
  if (newFile == NULL) {
    fclose(p);
    printf("BACKING STORE FAILURE\n");
    return 99;
  }
  copy(newFile, p);
  // close old files
  fclose(p);
  int pages = countTotalPages(newFile);
  PCB* new_pcb = makePCB(pages, new_pid);
  fclose(newFile);
  // load first two pages into ram
  for(int i=0; i<min(pages, 2); i++) {
    error = diskToRAM(new_pcb, i);
  }
  setPC(new_pcb, getFrame(new_pcb,0)*4);
  // put in new pcb in ready queue
  addToReady(new_pcb);
  return error;
}

