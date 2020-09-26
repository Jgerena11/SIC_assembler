#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "hash_table.c"

char* hRecord(char* name, int address, int length){
  char line[20] = "H";
  char addressbuff[7];
  char lengthbuff[7];

  for(int i = 0; i<6; i++){
    if(i >= strlen(name)){
      line[i+1] = ' ';
    }else{
      line[i+1] = name[i];
    }
  }

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

char* eRecord(int address){
  char line[8] = "E";
  char buff[7];

  snprintf(buff, 7, "00%x", address);

  strcat(line, buff);

  char* ptr = strdup(line);

  return ptr;
}

void pass2(FILE* inputFile, int programLength, int start_add, HashTable* table){
  //-------------open object file-------------------
  FILE* objectfile = fopen("object_file.txt", "w");

  if (!object_file){
      printf("ERROR: %s could not be opened for writing.\n", "object_file.txt");
      return 1;
  }

  while(fgets(line, 1024, inputFile)){
    //check for comments
    if (line[0] == 35){
      count++;
      continue;
    }

    char* target = strdup(line);

    char *tokens[3]; //tokens array

    //tokenize line
    char *token = strtok(target, " \t");
    int i = 0;
    while (token && i<3){
      if(token[strlen(token) - 1] == 10){
        token[strlen(token) - 1] = '\0';
      }
      if(token[strlen(token) - 1] == 13){
        token[strlen(token) - 1] = '\0';
      }
      tokens[i] = token;
      token = strtok(NULL, " \t");
      i++;
    }

    char sym_key[10];
    char *opcode;
    char *operand;
    if(!(isspace(line[0]))){
      strcpy(sym_key, tokens[0]);
      opcode = tokens[1];
      operand = tokens[2];
    }else{
      opcode = tokens[0];
      operand = tokens[1];
    }

    if(strcmp(opcode, "START") == 0){
      fprintf(object_file, "H%s ", sym_key);
      fprintf("0%x", get(sym_key)->address);
    }
  }
}
