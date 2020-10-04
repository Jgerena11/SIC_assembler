#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

char* hRecord(char* name, int address, int length){
  char line[20] = "H";
  char addressbuff[7];
  char lengthbuff[7];

  strcat(line, name);
  int sizebuff = 6 - strlen(name);
  for(int i = 0; i<sizebuff; i++){
    strcat(line, " ");
  }

  snprintf(addressbuff, 5, "%x", address);
  snprintf(lengthbuff, 5, "%x", length);

  sizebuff = 6 - strlen(addressbuff);
  for(int i = 0; i<sizebuff; i++){
    strcat(line, "0");
  }
  strcat(line, addressbuff);

  sizebuff = 6 - strlen(lengthbuff);
  for(int i = 0; i<sizebuff; i++){
    strcat(line, "0");
  }
  strcat(line, lengthbuff);

  char* ptr = strdup(line);
  return ptr;
}

int main(void) {
  char* name = "COPY";
  int address = 4096;
  int length = 8367;

  char* value = hRecord(name, address, length);

  printf("%s\n", value);
  return 0;
}
