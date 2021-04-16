// ITAI ZILBERSTEIN

#include<stdlib.h>
#include<string.h>
#include<stdio.h>

struct MEM {
  char* var;
  char* value;
};

struct MEM* memory [999]; // array of shell memory pointers
int size = 0; // size of memory array (number of shell variables)

// Returns a pointer to the memory with var = key or NULL if none 
static struct MEM* get (char* key) {
  for (int i=0; i<size; i++){
    if (!strcmp(memory[i]->var, key))
      return memory[i];
  }
  return NULL;
}

// Prints the value associated with the key     
int print_var (char* key) { 
  struct MEM* to_print = get(key);
  if (to_print == NULL)
    return 5;
  else {
    printf("%s\n", to_print->value);
    return 0;
  }
}

// Sets the key value pair in the mem array    
int set_var (char* key, char* value) { 
  struct MEM* to_set = get(key);
  if (to_set == NULL){ // does not exist yet
    if (size > 998) { return 3;} // no more memory 
    memory[size] = (struct MEM*) malloc(sizeof(struct MEM));
    if (memory[size] == NULL) { return 3;} // no more memory
    memory[size]->var = strdup(key);
    memory[size]->value = strdup(value);
    size++; // increase the size
    return 0;
  }
  else { // updates existing
    to_set->value = strdup(value);
    return 0;
  }
}
