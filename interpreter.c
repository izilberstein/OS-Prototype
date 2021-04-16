// ITAI ZILBERSTEIN

#include<stdbool.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include "shellmemory.h"
#include "shell.h"
#include "kernel.h"
#include "diskdriver.h"

int interpreter (char input[]);
bool inFile = false;

// Displays 'help' menu
static int help() {
  printf("%-30s %s", "COMMAND", "DESCRIPTION\n");
  printf("%-30s %s", "help", "-- display this window\n");
  printf("%-30s %s", "quit", "-- exit the shell\n");
  printf("%-30s %s", "set VAR STRING","-- set an environment variable\n");
  printf("%-30s %s", "print VAR", "-- print an environment variable\n");
  printf("%-30s %s", "run SCRIPT.TXT", "-- execute the file SCRIPT.TXT\n");
  printf("%-30s %s", "exec P1 [P2] [P3]", "-- executes up to 3 concurrent programs\n");
  printf("%-30s %s", "mount NAME BLOCKS BLOCKSIZE", "-- mounts partition with specified size\n");
  printf("%-30s %s", "read FILE VAR", "-- reads data in FILE to VAR\n");
  printf("%-30s %s", "write FILE [DATA]", "-- writes DATA to FILE\n");
  return 0;
}

// Prints "Bye!" and returns exit code if not in file
static int quit() {
  printf("Bye!\n");
  if (!inFile) {
    clean();
    return 10; // 10 is the exit code
  }
  return 0;
}

// Sets an environment variable
static int set(char* name, char* value) {
  return set_var(name, value); // calls the appropriate function in shellmemory.c
}

// Prints an environment variable
static int print(char* name) {
  return print_var(name); // calls the appropriate function in shellmemory.c
}

// Runs the inputted file
static int run(char* file) {
  FILE* script = fopen(file, "r");
  if (script == NULL) {
    return 2;
  }
  bool enterFile = inFile;
  inFile = true;
  int error = 0;
  char array[999];
  fgets(array, 999, script);
  while (!feof(script) && error == 0) {
    error = interpreter(array);
    fgets(array, 999, script);
  }
  fclose(script);
  inFile = enterFile;
  return error;
}

// Executes up to 3 concurrent programs
static int exec(char* p1, char* p2, char* p3){
  int error = 0;
  if (p2 != NULL) { // at least 2 files to execute
    if (p3 != NULL){  // 3 files to execute
      error = myinit(p1);
      if (error) {return error;}
      error = myinit(p2);
      if (error) {return error;}
      error = myinit(p3);
      if (error) {return error;}
    }
    else { // 2 files to execute -- opens them and initializes them, unless errors occur
      error = myinit(p1);
      if (error) {return error;}
      error = myinit(p2);
      if (error) {return error;}
    } 
  }
  else { // 1 file to execute -- opens and initializes it, unless error occurs
    error = myinit(p1);
    if (error) {return error;}
  }
  bool enterFile = inFile;
  inFile = true;
  error = scheduler();
  inFile = enterFile;
  return error;
}

// Creates partition with inputted infromation
static int mount(char* name, int blocks, int size) {
  int error = partitionFS(name, size, blocks);
  if (error == 0)
    error = mountFS(name);
  return error;
}

// Writes to file
static int write(char* filename, char* data) {
  return writeFS(filename, data);
}

// Reads from file and sets shell variable to contents
static int read(char* filename, char* var) {
  char* val = readFS(filename);
  if (val == NULL)
    return set(var, "NULL");;
  return set(var, val);
}

// Interprets a command line input
int interpreter(char input[]){
  int error = 0; //error code to return -- see shell.c for error codes
  char* commands [200]; // array of command arguments                                                                                                      
  parse(input, commands);
  //a masssive switch... 
  //does command format validation before calling subroutine
  if (commands[0] == NULL); // for \n or spacebar
  else if (!strcmp(commands[0], "help")) {
    if (commands[1] == NULL ){ error = help();}
    else { error = 1;}
  }
  else if (!strcmp(commands[0], "quit")) {
    if (commands[1] == NULL ){ error = quit();}
    else { error = 1;}
  }
  else if (!strcmp(commands[0], "set")){
    if (commands[1]  == NULL || commands[2] == NULL){ error = 1;}
    else {
      int l = 3;     // Handles setting a variable that has spaces
      char word[100];
      strcpy(word, commands[2]);
      while(commands[l] != NULL) {
	strcat(word, " ");
	strcat(word, commands[l]);
	l++;
      }
      char* name = strdup(commands[1]);
      char* value = strdup(word);
      error = set(name, value);
    }
  }
  else if (!strcmp(commands[0], "print")){
    if (commands[1]  == NULL || commands[2] != NULL){ error = 1;}
    else { 
      char* name = strdup(commands[1]);
      error = print(name);
    }
  }
  else if (!strcmp(commands[0], "run")){
    if (commands[1]  == NULL || commands[2] != NULL) { error = 1;}
    else { 
      char* file = strdup(commands[1]);
      error = run(file);
    }
  }
  else if (!strcmp(commands[0], "exec")){
    if (commands[1] == NULL) { error = 1;}
    else {
      char* p1 = strdup(commands[1]);
      char* p2 = NULL;
      char* p3 = NULL;
      if (commands[2] != NULL){ 
	p2 = strdup(commands[2]);
	if (commands[3] != NULL) {
	  p3 = strdup(commands[3]);
	  if (commands[4] != NULL) { error = 1;}
	}
      }
      if (error == 0){
	error = exec(p1, p2, p3);
      }
    }
  }
  else if (!strcmp(commands[0], "mount")){
    if (commands[1]  == NULL || commands[2] == NULL || commands[3] == NULL || commands[4] != NULL){ error = 1;}
    else {
      char* name = strdup(commands[1]);
      int blocks = atoi(commands[2]);
      int size = atoi(commands[3]);
      if (!blocks || !size){
        error = 1; // 0 partition or block size
      }
      if (error == 0)
	error = mount(name, blocks, size);    
    }
  }
  else if (!strcmp(commands[0], "write")){
    if (commands[1]  == NULL || commands[2] == NULL){ error = 1;}
    else {
      int l = 3;     // Handles setting a variable that has spaces                                                                              
      char word[100];
      strcpy(word, commands[2]);
      while(commands[l] != NULL) {
        strcat(word, " ");
        strcat(word, commands[l]);
        l++;
      }
      char* name = strdup(commands[1]);
      char* value = strdup(word);
      if (value[0] != '[' || value[strlen(value)-1] != ']')
	error = 1;
      value = value+1; // gets rid of brackets
      value[strlen(value)-1] = '\0';
      if (error == 0)
	error = write(name, value);
    }
  }
  else if (!strcmp(commands[0], "read")){
    if (commands[1]  == NULL || commands[2] == NULL || commands[3] != NULL) { error = 1;}
    else {
      char* file = strdup(commands[1]);
      char* var = strdup(commands[2]);
      if (error == 0)
        error = read(file, var);
    }
  }
  else {			
    error = 1;
  }
  int i = 0;
  while (commands[i] != NULL) { // free the allocaed memory                                                                                      
    free(commands[i]);
    i++;
  }
  return error;
}
