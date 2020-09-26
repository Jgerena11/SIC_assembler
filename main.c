#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

char* hRecord(char* name, int address, int length){
  char line[20] = "H";
  char programName[6];
  char startAdd[6] = "00";
  char programLength[6] = "00";

  for(int i = 0; i<6; i++){
    if(i >= strlen(name)){
      line[i+1] = ' ';
    }else{
      line[i+1] = name[i];
    }
  }

  char addressbuff[7];
  char lengthbuff[7];

  snprintf(addressbuff, 7, "00%x", address);
  snprintf(lengthbuff, 7, "00%x", length);

  for(int i = 0; i<6; i++){
    line[i+7] = addressbuff[i];
  }

  for(int i = 0; i<6; i++){
    line[i+13] = lengthbuff[i];
  }

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
