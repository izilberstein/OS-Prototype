#ifndef PCB_H
#define PCB_H
typedef struct PCB PCB;
PCB* makePCB(int pages, int pid);
int getPC(PCB* p);
void setPC(PCB* p, int pc);
int getPID(PCB* p);
int getPage(PCB* p);
void setPage(PCB* p, int page);
int getPagesMax(PCB* p);
int getFrame(PCB* p, int page);
int getPC_offset(PCB* p);
void setPC_offset(PCB* p, int o);
void setPageTable(PCB* p, int pageNo, int frameNo);
int inPageTable(PCB* p, int frame);
// FOR TESTING
void display_PCB(PCB* pcb);
#endif
