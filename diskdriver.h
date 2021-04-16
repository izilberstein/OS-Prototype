#ifndef DISKDRIVER_H
#define DISKDRIVER_H
void initIO();
int partitionFS(char* filename, int blocks,  int size);
int mountFS(char* filename);
int writeFS(char* filename, char* data);
char* readFS(char* filename);
void clean();
#endif
