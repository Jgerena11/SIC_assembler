#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "hash_table.c"







void emptyTobjectCode(){
  tSize = 0;
  tStart = 0;
  TobjectCode[0] = '\0';
}

void NewTRecord(int start, int size, char* objectcode){
  tStart = start;
  tSize += size;
  strcat(TobjectCode, objectcode);
}

bool pass2(FILE* inputFile, int programLength, int start_add, int firstExec, HashTable* table){


  int count = 0; //keep count of lines in program
  int LOCCTR = 0;
  char line[1024];




  return true;
}
