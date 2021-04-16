// ITAI ZILBERSTEIN

#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include "interpreter.h"

// Parses the input[] into seperate words and stores in commands[]
void parse(char input[], char* commands[]){
  int i = 0; // commands index
  int s = 0;
  for(;*(input+s) == ' '; s++); // credit prof Vybihal for this beautiful line
  
  const char* delim = " \n";
  char *buffer = strtok(input, delim);
  
  while ( buffer != NULL ){ // parses the input into commands[]
    commands[i] = (char*)malloc(strlen(buffer)); // allocate memory in the array
    strcpy(commands[i], buffer);
    i++;
    buffer = strtok(NULL, delim);
  }
  commands[i] = NULL;
}

// Handles error codes -- code 0 is no error, 10 is exit, 99 is handled error
void handleError(int code) {
  if (code == 0 || code == 10 || code == 99) // display no message
    return;
  printf("ERROR: ");
  switch(code){ // for message
  case 1 :
    printf("Unknown command\n");
    break;;
  case 2 :
    printf("File not found\n");
    break;;
  case 3 :
    printf("Shell memory overflow\n");
    break;;
  case 4 :
    printf("Not enough RAM to add program\n");
    break;;
  case 5 :
    printf("No such variable in memory\n");
    break;;
  }
}

// Shell comand line loop
int shellUI (){
  printf("Welcome to Itai's shell!\n");
  printf("Version 4.0 Updated April, 2021\n");
  int error = 0;  // boolean to continue
  while (error != 10 ){
    if (isatty(STDIN_FILENO)) {// check if STDIN is the terminal
      putchar('$'); putchar(' ');} // display command prompt
    char input [200];
    fgets(input, 200, stdin); 
    error = interpreter(input);
    handleError(error);
  }
  return 0;
}

