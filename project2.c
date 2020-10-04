#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "hash_table.c"

char TobjectCode[59] = "";
int tStart = 0;
int tSize = 0;

char* hRecord(char* name, int address, int length){
  char line[20] = "H";
  char namebuff[8];
  char addressbuff[7];
  char lengthbuff[7];

  snprintf(namebuff, 7, "%-6s", name);
  snprintf(addressbuff, 7, "%06X", address);
  snprintf(lengthbuff, 7, "%06X", length);

  strcat(line, namebuff);
  strcat(line, addressbuff);
  strcat(line, lengthbuff);

  char* ptr = strdup(line);
  return ptr;
}

char* eRecord(int address){
  char line[8] = "E";
  char buff[7];

  snprintf(buff, 7, "%06X", address);

  strcat(line, buff);

  char* ptr = strdup(line);

  return ptr;
}

char* getObjectCode(int opcode, int address){
  char objectcode[68];

  char buff1[3];
  char buff2[5];

  snprintf(buff1, 3, "%X", opcode);
  snprintf(buff2, 5, "%04X", address);

  strcat(objectcode, buff1);
  strcat(objectcode, buff2);

  char* ptr = objectcode;

  return ptr;
}

char* getCharConst(char* str){
  char charConst[100];
  for(int i = 0; i<strlen(str); i++){
    char numbuff[3];
    snprintf(numbuff, 3, "%X", str[i]);
    strcat(charConst, numbuff);
  }//end for

  char* ptr = charConst;

  return ptr;
}

int getHexConstLength(char* str){
  int size;
  int num = (int)(strlen(str) / 2);
  float value = (float)strlen(str)/(float)2;
  if(value > (float)num){
    size = num + 1;
  }else{
    size = num;
  }

  return size;
}

char* getTRecord(){
  char line[69] = "T";

  char buff1[7];
  char buff2[3];

  snprintf(buff1, 7, "%06X", tStart);
  snprintf(buff2, 3, "%02X", tSize);

  strcat(line, buff1);
  strcat(line, buff2);
  strcat(line, TobjectCode);

  char* ptr = line;

  return ptr;
}

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
  //-------------open object file-------------------
  FILE* objectfile = fopen("objectfile.txt", "w");

  if (!objectfile){
      printf("ERROR: %s could not be opened for writing.\n", "objectfile.txt");
      return false;
  }

  int count = 0; //keep count of lines in program
  int LOCCTR = 0;
  char line[1024];

  while(fgets(line, 1024, inputFile)){
    //check for comments
    if (line[0] == 35){
      count++;
      continue;
    }

    char* target = strdup(line);

    char *tokens[3]; //tokens array

    //tokenize line
    char *token = strtok(target, ", \t");
    int i = 0;
    while (token){
      if(token[0] == 35) {
        break;
      }
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

    //assign tokens to variables
    char sym_key[10];
    char *opcode;
    char *operand1;
    char* operand2;
    char *operands[10];
    if(!(isspace(line[0]))){
      strcpy(sym_key, tokens[0]);
      opcode = tokens[1];
      for(int j= 2; j<i; j++){
        if(j > 3){
          flagError(count, line, "To many operands.");
          return false;
        }
        operands[j-2] = tokens[j];
      }//end for
      if(operands[0] != NULL){
        if(i == 3){
          operand1 = operands[0];
          operand2 = operands[1];
        }else{
          operand1 = operands[0];
        }
      }
    }else{
      opcode = tokens[0];
      for(int j = 1; j < i; j++){
        if(j>2){
          flagError(count, line, "To many operands.");
          return false;
        }
        operands[j-1] = tokens[j];
      }//end for

      if(operands[0] != NULL){
        if(i == 2){
          operand1 = operands[0];
          operand2 = operands[1];
        }else{
          operand1 = operands[0];
        }
      }
    }//end if else

    if(strcmp(opcode, "START") == 0){
      char* codeline = hRecord(sym_key, tStart, programLength);
      fprintf(objectfile, "%s\n", codeline);
      count++;
      continue;
    }else if(strcmp(opcode, "END") == 0){
      //print out any t records.
      if(tSize > 0){
        char* codeline = getTRecord(tStart, tSize);
        fprintf(objectfile, "%s\n", codeline);
        emptyTobjectCode();
      }

      //print out end record.
      char* codeline = eRecord(firstExec);
      fprintf(objectfile, "%s\n", codeline);
      continue;
    }else if(isInstruction(opcode)){
      char *objectcode;
      Symbol* operation = get(optable, opcode);

      //set start address of t record if not already set.
      if(tStart == 0)
        tStart = LOCCTR;

      //check if line contains operands.
      if(operand1 != NULL){
        Symbol* sym_instr = get(table, operand1);

        if(sym_instr == NULL){
          flagError(count, line, "symbol does not exist.");
          return false;
        }else{
          int locn;
          if(strcmp(operand2, "X") == 0){
            locn = sym_instr->address + 32768;
          }else{
            locn = sym_instr->address;
          }

          objectcode = getObjectCode(operation->address, locn);
        }//end if else.
      }else{
        objectcode = getObjectCode(operation->address, 0);
      }//end if operand1

      if(strlen(TobjectCode) + strlen(objectcode) > 59 ){
        char* trecord = getTRecord();
        fprintf(objectfile, "%s\n", trecord);
        emptyTobjectCode();
        NewTRecord(LOCCTR, 3, objectcode);
      }else{
        strcat(TobjectCode, objectcode);
        tSize += 3;
      }

      LOCCTR = LOCCTR + 3;
    }else if(isADirective(opcode)){
      char* objectcode;
      int byteSize = 0;
      int incramentValue = 0;

      if(tStart == 0)
        tStart = LOCCTR;

      //make sure it has no more than 1 operand
      if(operand2 != NULL){
        flagError(count, line, "Directive cannot have more than 1 operand");
        return false;
      }

      if(strcmp(opcode, "BYTE") == 0){
        char* str = extractString(line);

        if(operand1[0] == 'X'){
          objectcode = str;
          byteSize = getHexConstLength(str);

          int num = (int)(strlen(str) / 2);
          incramentValue = num;
        }else if(operand1[0] == 'C'){
          char* charConst = getCharConst(str);
          objectcode = charConst;

          int num = strlen(str);
          byteSize = num;
          incramentValue = num;
        }//end if else
      }else if(strcmp(opcode, "WORD") == 0){
        Symbol* symbol = get(table, sym_key);
        byteSize = (int)strtol(operand1, NULL, 10);
        incramentValue = 3;
      }else if(strcmp(opcode, "RESW") == 0){
        int num = (int)strtol(operand1, NULL, 10);
        incramentValue = 3 * num;
      }else if(strcmp(opcode, "RESB") == 0){
        int num = (int)strtol(operand1, NULL, 10);
        incramentValue = num;
      }

      if(strlen(TobjectCode) + strlen(objectcode) > 59 ){
        char* trecord = getTRecord();
        fprintf(objectfile, "%s\n", trecord);
        emptyTobjectCode();
        NewTRecord(LOCCTR, byteSize, objectcode);
      }else{
        strcat(TobjectCode, objectcode);
        tSize += byteSize;
      }

      LOCCTR += incramentValue;
    }//else if directive
    count++;
  }//end while

  fclose(objectfile);
  return true;
}
