// ITAI ZILBERSTEIN

#include<stdlib.h>
#include<stdio.h>
#include<string.h>

// Partition data
struct PARTITION {
      int total_blocks;
      int block_size;
} partition;

// File Allocation Table
typedef struct FAT {
       char *filename;
       int file_length;
       int blockPtrs[10];
       int current_location;
} FAT;
// FAT represented as 20 FAT pointers
FAT* fat[20];
void display_FAT();
int readBlock(int file);
int writeBlock(int file, char* data);
int activate(int findex);

// Directory for active files - index i holds index of corresponding FAT for active file i in active file table
int active_directory[5];
// Buffer to read/write
char* block_buffer;
// Active file table
FILE* active_file_table[5];
// Active partition
char* active_partition = NULL;
// index of start of data region
#define data_start 200
// index of next free block in partition -- used for non-contiguous allocation
int next_free;



// Intializes the data structures
void initIO() {
  block_buffer = NULL;
  for (int i=0; i<19; i++)
    fat[i] = NULL;
  for(int i=0; i<5; i++){
    active_directory[i]=-1; //not in use
  }
}

// Returns a string corresponding to the path name of the inputted string
char* getPartitionPath (char* name) {
  char* par = "PARTITION/";
  char* path = (char*) malloc(sizeof(par)+sizeof(name));
  strcpy(path, par);
  strcat(path, name);
  return path;
}

// Returns index of FAT entry with filename 'name' or -1 if not in FAT
int getFATIndex(char* file) {
  for (int i=0; i<20;i++){
    if (fat[i] !=NULL) {
      if (!strcmp(file, fat[i]->filename)) {
	return i;
      }
    }
  }
  return -1;
}

// Returns 1 if the file is open (in active_file_table), 0 if the file is not
int isOpen(char* file) {
  int findex = getFATIndex(file);
  if (findex == -1)
    return 0;
  for (int j=0; j<5; j++) {
    if (active_directory[j] == findex)
      return 1;
  }
  return 0;
}

// Returns the index of the next free block in the partition 
int nextFreeBlock() {
  int block = 0;
  FILE* data = fopen(getPartitionPath(active_partition), "r");
  fseek(data, data_start, 0);
  char buff[partition.block_size];
  fread(buff, partition.block_size, 1, data);
  while(buff[0] != '0' && block < partition.total_blocks) {
    fread(buff, partition.block_size, 1, data);
    block++;
  }
  fclose(data);
  if (block < partition.total_blocks)
    return block;
  else
    return -1;
}

// Makes a new FAT entry with the inputted information -- returns index if success, -1 if no space
int makeFATEntry(char* name, int length, int ptrs[], int pos) {
  int fentry = -1;
  for(int i=19; i>=0; i--){
    if(fat[i] == NULL)
      fentry = i;
  }
  if (fentry == -1) // FAT full                                                                                                                 
    return -1;
  FAT* newEntry = (FAT*) malloc(sizeof(FAT));
  newEntry->filename = strdup(name);
  newEntry->file_length = length;
  for (int j=0; j<10; j++)
    newEntry->blockPtrs[j] = ptrs[j];
  newEntry->current_location = pos;
  fat[fentry] = newEntry;
  return fentry;
}

// Resets the FAT table and active file table
void clean (){
  for(int i=0; i<20; i++){
    if (fat[i] != NULL) {
      free(fat[i]);
      fat[i] = NULL;
    }
  }
  for(int i=0; i<5; i++){
    active_directory[i] = -1;
    active_file_table[i] = NULL;
  }
}

// Writes the meta data stored in partition and FAT to the metadata region of the active partition
int writeThrough () {
  int error = 0;
  // array to hold info to write
  char metadata[data_start];
  // write partition info to array
  snprintf(metadata, data_start, "%d,%d:", partition.block_size,partition.total_blocks);
  // write fat table info to array
  for (int i=0; i<20; i++) {
    char table[200];
    if (fat[i]) {
      char array[100];
      array[0] = '{';
      array[1] = '\0';
      for(int j=0; j<10; j++) {
	char minibuff[20];
	snprintf(minibuff, 20, "%d,", fat[i]->blockPtrs[j]);
	strcat(array, minibuff);
      }
      int end = strlen(array);
      array[end-1] = '}';
      array[end] = '\0';
      snprintf(table, 200, "[%s,%d,%s,%d]", fat[i]->filename, fat[i]->file_length, array, fat[i]->current_location);
      if (strlen(metadata) + strlen(table) < data_start)
	strcat(metadata, table);
      // meta data region overflow
      else
	error = 99;
    }
  }
  // fill array with empty data
  for(int i=strlen(metadata); i<data_start-1; i++){
    metadata[i] = '-';
  }
  strcat(metadata, ":");
  // write to disk
  FILE* meta_data = fopen(getPartitionPath(active_partition), "r+");
  fwrite(metadata, 1, data_start, meta_data);
  fclose(meta_data);
  return error;
}

// Intializes partition -- checks if name exists as partition -- if not make and populate with info
int partitionFS(char* name, int blocksize, int totalblocks) {
  // make PARTITION directory if one does not exist  
  system("mkdir -p PARTITION");
  char* path = getPartitionPath(name);
  FILE* check;
  // if partition exists, return
  check = fopen(path, "r");
  if (check) {
    fclose(check);
    return 0;
  }
  // make partition
  char* command = "touch ";
  char* syscall = (char*)  malloc(sizeof(command)+sizeof(path));
  strcpy(syscall, command);
  strcat(syscall, strdup(path));
  system(syscall);
  // open new partition

  // if it already exists, exit, else write the meta data
  // file does not exist -- create it and write meta data
  FILE* newPar = fopen(path, "w");
  free(syscall);
  free(path);
  // writes metadata
  char metadata[data_start];
  snprintf(metadata, data_start, "%d,%d:", blocksize, totalblocks);
  for(int i=strlen(metadata); i<data_start-1;i++)
    metadata[i]='-';
  metadata[data_start-1] = ':';
  fwrite(metadata, 1, data_start, newPar); 
  // writes 0s in data region
  for(int i=0; i<totalblocks*blocksize; i++)
    fputc('0', newPar);
  fclose(newPar);
  return 0;
}

// Returns an int that is made up of the chars between start and ctrl in buffer   
int getIntFromBuffer(int* start, char ctrl, char buffer[]) {
  char integer[20];
  int i = 0;
  for(char c=buffer[(*start)++]; c!= ctrl; c =buffer[(*start)++])
    integer[i++] = c;
  integer[i] = '\0';
  return atoi(integer);
}

// Returns a string that is made up of the chars between start and ctrl in buffer
char* getStringFromBuffer(int* start, char ctrl, char buffer[]) {
  char string[20];
  int i=0;
  for(char c=buffer[(*start)++]; c!= ctrl; c =buffer[(*start)++])
    string[i++] = c;
  string[i] = '\0';
  return strdup(string);
}

// Sets information of all data structures and variables
int mountFS(char* name) {
  int error = 0;
  // sets active partition name:
  active_partition = strdup(name);
  // erase old data in data structures
  clean();
  // load metadata into array
  char metadata[data_start];
  FILE* fp = fopen(getPartitionPath(active_partition), "r");
  fread(metadata, data_start, 1, fp);
  fclose(fp);
  // m is current index in metadata array
  int* m = (int*) malloc(sizeof(int));
  *m = 0;
  // get partition info
  partition.block_size = getIntFromBuffer(m, ',', metadata);
  partition.total_blocks = getIntFromBuffer(m, ':', metadata);
  // load data into fat[] using control characters
  while(metadata[(*m)] != ':' && metadata[(*m)] != '-') {
    // buffers to load chars into
    (*m)++; // skip '['
    char* name = getStringFromBuffer(m, ',', metadata);
    int length = getIntFromBuffer(m, ',', metadata);  
    int bps[10];
    (*m)++; // skip ','
    for(int i=0; i<9;i++){
      bps[i] = getIntFromBuffer(m, ',', metadata); 
    }
    bps[9] = getIntFromBuffer(m, '}', metadata);
    (*m)++; // skip ','
    int pos = getIntFromBuffer(m, ']', metadata);
    // RESETS position to 0 -- or use pos to keep position
    error = makeFATEntry(name, length, bps, 0);
    if (error == -1)
      return error;
  }
  // set next free
  next_free = nextFreeBlock();
  // set block buffer
  free(block_buffer);
  // create null terminated block buffer
  block_buffer = (char*) malloc(sizeof(partition.block_size+1));
  block_buffer[partition.block_size] ='\0';
  return 0;
}

// Opens the inputted file
int openfile(char* name) {
  // check if in FAT
  int findex = getFATIndex(name);
  // if in FAT, add to active file table
  if (findex > -1) {
    activate(findex);
    return findex;
  }
  // make FAT entry
  else {
    int emptyPointer[10];
    for(int i=0; i<10;i++)
      emptyPointer[i] = -1;
    return makeFATEntry(name, 0, emptyPointer, 0); 
  }
}

// Activate a file and put it in the active file table -- 0 on success, 1 on failure
int activate(int findex) {
  // find space in active file table                                                                                                            
  int aindex = -1;
  for(int i=4; i>=0; i--) {
    if (active_file_table[i] == NULL)
        aindex = i;
  }
  // if no space, error                                                                                                                         
  if (aindex == -1)
    return 1; // error                                                                                                                        
  // add it to the table     
  FILE* newFile = fopen(getPartitionPath(active_partition), "r+");
  fseek(newFile, (partition.block_size*fat[findex]->blockPtrs[0]) + data_start, 0);
  active_file_table[aindex] = newFile;
  active_directory[aindex] = findex;
  return 0;
}

// Returns the file pointer corresponding to the FAT index file or NULL if none
FILE* getFP(int file) {
  for (int i=0; i<5; i++) {
    if (active_directory[i] == file)
      return active_file_table[i];
  }
  return NULL;
}

// File system read -- returns string of read on sucess or null on fail 
char* readFS(char* file) {
  if (active_partition == NULL) {
    printf("ERROR: No partition mounted\n");
    return NULL;
  }
  int findex;
  // check if file is open
  if (isOpen(file))
    findex = getFATIndex(file);
  // if not, open file
  else
    findex = openfile(file);
  // check if active
  FILE* fp = getFP(findex);
  if (fp == NULL) {
    printf("ERROR: File not found\n");
    return NULL;
  }
  // empty file or EOF
  if (fat[findex]->file_length == 0 || fat[findex]->current_location == fat[findex]->file_length) {
    return NULL;
  }
  // string to return
  char* head = (char*) malloc(sizeof(partition.block_size*fat[findex]->file_length + 1));
  char* tail = (char*) malloc(sizeof(partition.block_size+1));
  // read block by block
  while(fat[findex]->current_location < fat[findex]->file_length) {
    if(readBlock(findex)) {
      return NULL;
    }
    strcpy(tail, block_buffer);
    tail[partition.block_size] = '\0';
    strcat(head, tail);
  }
  free(tail);
  head[(partition.block_size)*(fat[findex]->file_length)] = '\0';
  // write metadata back to disk
  if(writeThrough())
    printf("ERROR: Disk metadata overflow\n");
  return head;
}
 
// File system write -- returns 0 on success
int writeFS(char* file, char* string) {
  if (active_partition == NULL) {
    printf("ERROR: No partition mounted\n");
    return 99;
  }
  int findex;
  // check if file is open
  if (isOpen(file))
    findex = getFATIndex(file);
  // if not, create file
  else
    findex = openfile(file);
  FILE* fp = getFP(findex);
  if (fp == NULL) {
    if(activate(findex)) {
      printf("ERROR: Active file table full\n");
      return 99;
    }
  }
  // inedx of start of string we want to write
  int start = 0;
  // write to disk block at a time
  while (start < strlen(string) && fat[findex]->current_location < 10) {
    // need to allocate block on disk
    if (fat[findex]->blockPtrs[fat[findex]->current_location] == -1) {
      if (next_free >= partition.total_blocks || next_free == -1){
	printf("ERROR: Disk is full\n");
	return 99;
      }
      // update FAT data
      fat[findex]->blockPtrs[fat[findex]->current_location] = next_free;
      next_free++;
      fat[findex]->file_length++;
    }
    // write meta data to disk before performing write
    if (writeThrough()) {
      printf("ERROR: Partition meta data overflow\n");
      return 99;
    }
    int error = writeBlock(findex, &(string[start])); 
    if (error){
      printf("ERROR: Disk write failed\n");
      return 99;
    }
    start += partition.block_size;
  }
  fflush(fp);
  return 0;
}

// Read block from disk
int readBlock(int file) {
  if(file < 0 || file > 19) // check index
    return 99;
  FAT* entry = fat[file];
  FILE* fp = getFP(file);
  int index = entry->blockPtrs[entry->current_location]*(partition.block_size) + data_start;
  // Read block from current location
  fseek(fp, index, 0);
  fread(block_buffer, partition.block_size, 1, fp);
  entry->current_location++;
  return 0;
}

// Simple min
int min(int x, int y) {if (x<y) return x; return y;}

// Write block to disk
int writeBlock(int file, char* data) {
  if(file < 0 || file > 19) // check index
    return 99;
  FAT* entry = fat[file];
  FILE* fp = getFP(file);
  // Index to write to
  int index = entry->blockPtrs[entry->current_location]*(partition.block_size) + data_start;
  fseek(fp, index, 0);
  // Copy what we will write to the buffer
  memset(block_buffer, '0', partition.block_size);
  strncpy(block_buffer, data, min(partition.block_size, strlen(data)));
  // Write the block
  fwrite(block_buffer, 1, partition.block_size, fp);
  // Update FAT
  entry->current_location++;
  return 0;
}

// FOR TESTING
void display_FAT() {
  for(int i=0; i<20; i++){
    if(fat[i] !=NULL) {
      printf("-------------%d-------------\n", i);
      printf("%s, %d, %d\n", fat[i]->filename, fat[i]->file_length, fat[i]->current_location);
      for(int j=0; j<10;j++)
        printf("%d ", fat[i]->blockPtrs[j]);
      printf("\n");
    }
  }
}

// FOR TESTING
void display_AFT() {
  for (int i=0; i<5; i++){
    if (active_file_table[i] != NULL)
      printf("(%d, %d),", i, active_directory[i]);
    else
      printf("(NULL),");
  }
  printf("\n");
}
