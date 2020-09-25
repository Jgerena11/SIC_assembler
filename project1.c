#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "hash_table.c"

const int MAX_MEM = 32768;

char* extractString(char* src) {
  char buff[256];
  char *subString;
  strcpy(buff, src);
  subString = strtok(buff,"'");
  subString=strtok(NULL,"'");
  return subString;
}

bool isHexValue(char* str){
  bool flag = true;
  for(int i = 0; i<strlen(str); i++){
    if(str[i] >= 48 && str[i] <= 57){
      continue;
    }else if(str[i] >= 65 && str[i] <= 70 ){
      continue;
    }else{
      flag = false;
    }
  }

  return flag;
}

bool isValidCharConstant(char* str){
  bool flag = true;
  char buff[256];
  strcpy(buff, str);
  for(int i = 0; i<strlen(buff); i++){
    if(str[i] >= 65 && str[i] <= 90){
      continue;
    }else{
      flag = false;
    }
  }
  return flag;
}

void flagError(int line_num, char* line, char* desc) {
  printf("ASSEMBLY ERROR: \n");
  printf("%s", line);
  printf("Line: %d::%s\n", line_num, desc);
}

bool isBlankLine(char* line){
  char buff[256];
  strcpy(buff, line);
  for(int i = 0; i<strlen(buff); i++){
    if(isspace(buff[i]))
      continue;
    else
      return false;
  }
  return true;
}

bool isValidSymbol(int count, char* line, char* str){
  char buff[56];
  strcpy(buff, str);

  //check if symbol is a directive
  if(isADirective(str)){
    flagError(count, line, "Symbol matches directive");
    return false;
  }
  //check if symbol starts with A-Z chars
  if(str[0] < 65 || str[0] > 90){
    flagError(count, line, "Symbol must start with alpha character [A-Z]");
    return false;
  }
  //check if symbol is longer than 6 chars
  if(strlen(str) > 6){
    flagError(count, line, "Symbol must not be longer than 6 characters");
    return false;
  }
  //check if symbol contains special characters
  for(int i = 0; i<strlen(buff); i++){
    if(buff[i] < 65 || buff[i] > 90){
      flagError(count, line, "Symbol contains invalid characters");
      return false;
    }
  }

  return true;
}

int main(int argc, char *argv[]){
  int start_add;
  int end_add;
  int count = 0;
  int LOCCTR = 0;
  int programLength = 0;
  bool START = false;
  bool END = false;
  // bool END = true;

  if (argc != 2){
    printf("USAGE: %s <filename>\n", argv[0]);
    return 1;
  }

  //---------open input file----------------
  FILE* inputFile;
  inputFile = fopen(argv[1], "r");

  if (!inputFile){
    printf("ERROR: %s could not be opened for reading.\n", argv[1]);
    return 1;
  }

  //-------------open output file-------------------
  FILE* outputfile = fopen("tmp.txt", "w");

  if (!outputfile){
      printf("ERROR: %s could not be opened for writing.\n", "tmp.txt");
      return 1;
  }

  //create optab Table
  createOpcodeTable();

  //create new hash table.
  HashTable* table = new_table(100);

  //----------Iterate through lines in input file---------------------
  char line[1024];

  //-----------get START symbol --------------------
  while(fgets(line, 1024, inputFile)){
    //---check line for data.----
    int length = strlen(line);
    if(length<1){
      flagError(count, line, "Program cannot contain blank lines");
      return 1;
    }

    if(isBlankLine(line)){
      flagError(count, line, "Program cannot contain blank lines");
      return 1;
    }

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

    if(strlen(sym_key) > 0){
      if(!(isValidSymbol(count, line, sym_key))){
        return 1;
      }
    }

    //check for start.
    if(strcmp(opcode, "START") == 0){
      if(operand == NULL) {
        flagError(count, line, "START missing address");
        return 1;
      }

      if(!(isHexValue(operand))){
        flagError(count, line, "START address must be in hexadecimal values");
        return 1;
      }

      if(START == true){
        flagError(count, line, "duplicate START operations.");
        return 1;
      }
      START = true;
      LOCCTR = (int)strtol(operand, NULL, 16); //set start address
      start_add = LOCCTR; //set current address
      fprintf(outputfile, "%d\t%s", count, line); //print line to output tmp file
      insert(table, sym_key, LOCCTR, count);//add to symbol table
      count++;
      continue;
    }

    //once start is found begin adding to symbol table.
    if(START){
      fprintf(outputfile, "%d\t%s", count, line);

      //-----------check for symbols-----------------
      if ((line[0] >= 65) && (line[0] <= 90)){
        Symbol* symbol = get(table, sym_key);
        if(symbol != NULL) {
          char description[40] = "ERROR: duplicate symbol: ";
          strcat(description, sym_key);
          flagError(count, line, description);
          return 1;
        }else{
          insert(table, sym_key, LOCCTR, count);
        }//end if else
      }//end if symbol stmt

      //if END.
      if(strcmp(opcode, "END") == 0){
        if(END){
          flagError(count , line, "Duplicate END");
          return 1;
        }
        START = false;
        END = true;
        end_add = LOCCTR;
        programLength = end_add - start_add;
        break;
      }//end if END

      //check for Other directives.
      if(isInstruction(opcode)){
        LOCCTR = LOCCTR + 3;
      }else if(strcmp(opcode, "WORD") == 0){
        LOCCTR = LOCCTR + 3;
        int num = (int)strtol(operand, NULL, 10);
        if(num > pow(2, 23) - 1){
          flagError(count, line, "WORD is over maximum value");
          return 1;
        }
      }else if(strcmp(opcode, "RESW") == 0){
        int num = (int)strtol(operand, NULL, 10);
        LOCCTR = LOCCTR + (3*num);
      }else if(strcmp(opcode, "RESB") == 0){
        int num = (int)strtol(operand, NULL, 10);
        LOCCTR = LOCCTR + num;
      }else if(strcmp(opcode, "BYTE") == 0){
        char* str = extractString(line);

        if(operand[0] == 'X'){
          if(isHexValue(str)){
            int num = (int)(strlen(str) / 2);
            LOCCTR = LOCCTR + num;
          }else{
            flagError(count, line, "Operand must be a hexadecimal value");
            return 1;
          }//end if else
        }else if(operand[0] == 'C'){
          if(isValidCharConstant(str)){
            int num = strlen(str);
            LOCCTR = LOCCTR + num;
          }else{
            flagError(count, line, "Invalid char constant");
            return 1;
          }

        }else{
          flagError(count, line, "Must specify operand type 'X' or 'C'.");
          return 1;
        }
      }else{
        printf("opcode: %s\n", opcode);
        flagError(count, line, "opcode not a directive or instruction");
        return 1;
      }

      if(LOCCTR > MAX_MEM){
        flagError(count, line, "program has reach memory capacity.");
        return 1;
      }
    }//when START symbol found and END not reached.

    count++;
  }//end while loop.

  //TODO: check for duplicate END

  if(START){
    flagError(count, line, "Program missing END directive");
    return 1;
  }

  displayTable(table);

  fclose(inputFile);
  fclose(outputfile);

  return 0;
}
