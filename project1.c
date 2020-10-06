#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include "hash_table.c"

const int MAX_MEM = 32768;

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

void printObjectCodeAddition(char* str, char* line, int locn){
  printf("%s <== %X ::\t%s", str, locn, line);
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
  char objectcode[59] = "";

  char buff1[3];
  char buff2[5];

  snprintf(buff1, 3, "%X", opcode);
  snprintf(buff2, 5, "%04X", address);

  strcat(objectcode, buff1);
  strcat(objectcode, buff2);

  // printf("constructed object: %s\n", objectcode);
  char* ptr = strdup(objectcode);

  return ptr;
}

char* getCharConst(char* str){
  char charConst[100] = "";
  // printf("charConst: %s\n", charConst);
  for(int i = 0; i<strlen(str); i++){
    char numbuff[3];
    snprintf(numbuff, 3, "%X", str[i]);
    strcat(charConst, numbuff);
  }//end for

  char* ptr = strdup(charConst);

  return ptr;
}

int getHexConstLength(char* str){
  //every two chars in a hex constant is a byte.
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

char* getTRecord(int tStart, int tSize, char* object_code){
  char line[200] = "";

  int pry = 1;
  if(strlen(object_code) > 69){
    printf("objectcode length: %ld\n", strlen(object_code));
  }

  snprintf(line, 200, "T%06X%02X%s",tStart, tSize, object_code);

  if(strlen(line) > 69){
    printf("line to long!!");
  }

  char* ptr = strndup(line, strlen(line));

  return ptr;
}

void writeTRecord(FILE* file, char* record){
  // printf("size of written record: %ld\n", strlen(record));
  fprintf(file, "%s\n", record);
}

int main(int argc, char *argv[]){
  int start_add;
  int end_add;
  int count = 0;
  int LOCCTR = 0;
  int programLength = 0;
  bool START = false;
  bool END = false;
  int firstExec = 0;
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
        if(firstExec == 0){
          firstExec = LOCCTR;
        }
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

  if(START){
    flagError(count, line, "Program missing END directive");
    return 1;
  }

  fclose(outputfile);

  //reset locctr and count.
  LOCCTR = 0;
  count = 0;

  //set pointer to beginning of file
  fseek(inputFile, 0, SEEK_SET);

  //-------------open object file-------------------
  FILE* objectfile = fopen("objectfile.obj", "w");

  if (!objectfile){
      printf("ERROR: %s could not be opened for writing.\n", "objectfile.txt");
      return 1;
  }

  char TobjectCode[59] = "";
  int tStart = 0;
  int tSize = 0;

  //------------PASS 2---------------------
  while(fgets(line, 1024, inputFile)){
    //check for comments
    if (line[0] == 35){
      count++;
      continue;
    }

    char linebuff[1024];
    strcpy(linebuff, line);
    char* constant = strtok(linebuff, "'");
    constant = strtok(NULL, "'");

    char* target = strdup(linebuff);

    char *tokens[10] = {"", "", "", "", "", "", "", "", "", ""}; //tokens array

    //tokenize line
    char *token = strtok(target, ", \t");
    int i = 0;
    while (token && i < 10){
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
      token = strtok(NULL, ", \t");
      i++;
    }

    //assign tokens to variables
    char sym_key[10];
  char *opcode;
  char *operand1 = "";
  char* operand2 = "";

  if(!(isspace(linebuff[0]))){
    strcpy(sym_key, tokens[0]);
    opcode = tokens[1];
    if(tokens[2][0] != '\0')
      operand1 = tokens[2];
    if(tokens[3][0] != '\0')
      operand2 = tokens[3];
    if(tokens[4][0] != '\0'){
      flagError(count, line, "Too many operands");
      return 1;
    }
   }else{
    opcode = tokens[0];
    if(tokens[1][0] != '\0')
      operand1 = tokens[1];
    if(tokens[2][0] != '\0')
      operand2 = tokens[2];
    if(tokens[3][0] != '\0'){
      flagError(count, line, "Too many operands");
      return 1;
    }
  }//end if else

    if(strcmp(opcode, "START") == 0){
      LOCCTR = (int)strtol(operand1, NULL, 16);
      Symbol* sym = get(table, sym_key);
      char* hrecord = hRecord(sym_key, sym->address, programLength);
      fprintf(objectfile, "%s\n", hrecord);
    }else if(strcmp(opcode, "END") == 0){
      //print out any t records.
      if(tSize > 0){
        char* record = getTRecord(tStart, tSize, TobjectCode);
        writeTRecord(objectfile, record);
        tSize = 0;
        tStart = 0;
        TobjectCode[0] = '\0';
      }

      //print out end record.
      char* codeline = eRecord(firstExec);
      fprintf(objectfile, "%s\n", codeline);
    }else if(isInstruction(opcode)){
      char *objectCode = "";
      // printf("objectcode: %s\n", objectcode);
      Symbol* operation = get(optable, opcode);
      //set start address of t record if not already set.
      if(tStart == 0)
        tStart = LOCCTR;

      //check if line contains operands.
      if(operand1 && operand1[0]){
        Symbol* sym_instr = get(table, operand1);
        if(sym_instr == NULL){
          flagError(count, line, "symbol does not exist.");
          return 1;
        }else{
          int locn;
          if(operand2 && operand2[0]){
            if(strcmp(operand2, "X") == 0)
              locn = sym_instr->address + 32768;

          }else{
            locn = sym_instr->address;
          }

          objectCode = getObjectCode(operation->address, locn);
          if(strlen(objectCode) > 10){
            printf("objectcode length: %ld\n", strlen(objectCode));
          }
        }//end if else.
      }else{
        objectCode = getObjectCode(operation->address, 0);
        if(strlen(objectCode) > 10){
          printf("objectcode length: %ld\n", strlen(objectCode));
        }
      }//end if operand1

      if(strlen(TobjectCode) + strlen(objectCode) > 59 ){
        char* record = getTRecord(tStart, tSize, TobjectCode); //bug here?
        if(strlen(objectCode) > 10){
          printf("(2)objectcode length: %ld\n", strlen(objectCode));
        }
        writeTRecord(objectfile, record);
        tSize = 0;
        tStart = 0;
        TobjectCode[0] = '\0';
        tStart = LOCCTR;
        tSize = 3;
        printObjectCodeAddition(objectCode, line, LOCCTR);
        strcat(TobjectCode, objectCode);
      }else{
        printObjectCodeAddition(objectCode, line, LOCCTR);
        strcat(TobjectCode, objectCode);
        tSize += 3;
      }

      LOCCTR = LOCCTR + 3;
    }else if(isADirective(opcode)){
      int byteSize = 0;
      int incramentValue = 0;

      //make sure it has no more than 1 operand
      if(operand2[0] != '\0'){
        flagError(count, line, "Directive cannot have more than 1 operand");
        return 1;
      }

      if(strcmp(opcode, "BYTE") == 0){
        if(tStart == 0)
          tStart = LOCCTR;
        if(operand1[0] == 'X'){
          int num = (int)(strlen(constant) / 2);

          if(constant){
            if(strlen(TobjectCode) + strlen(constant) > 59){
              char* trecord = getTRecord(tStart, tSize, TobjectCode);
              writeTRecord(objectfile, trecord);
              tSize = 0;
              tStart = 0;
              TobjectCode[0] = '\0';
              tStart = LOCCTR;
              tSize += num;
              printObjectCodeAddition(constant, line, LOCCTR);
              strcat(TobjectCode, constant);
            }else{
              printObjectCodeAddition(constant, line, LOCCTR);
              tSize += num;
              strcat(TobjectCode, constant);
            }
          }

          LOCCTR += num;
        }else if(operand1[0] == 'C'){
          int charConstSize;
          if(constant){
            charConstSize = strlen(constant);
            char objectcode[59] = "";

            for(int i = 0; i<strlen(constant); i++){
              char buff[3];
              snprintf(buff,3, "%X", constant[i]);

              if(strlen(TobjectCode) + strlen(buff) > 59){
                printObjectCodeAddition(objectcode, line, LOCCTR);
                objectcode[0] == '\0';

                char* trecord = getTRecord(tStart, tSize, TobjectCode);
                writeTRecord(objectfile, trecord);
                tSize = 0;
                tStart = 0;
                TobjectCode[0] = '\0';
                tStart = LOCCTR;
                tSize += constant[i];
                strcat(TobjectCode, buff);
                strcat(objectcode, buff);
              }else{
                strcat(objectcode, buff);
                strcat(TobjectCode, buff);
                tSize += constant[i];
              }//end if else
            }//end for loop

            printObjectCodeAddition(objectcode, line, LOCCTR);
          }//end if constant
          LOCCTR += charConstSize;
        }//end if else
      }else if(strcmp(opcode, "WORD") == 0){
        if(tStart == 0)
          tStart = LOCCTR;
        char* objectcode = "";
        byteSize = (int)strtol(operand1, NULL, 10);
        char buff[10] = "";
        printf("BYTESIZE: %d\n", byteSize);
        snprintf(buff, 7, "%06X", byteSize);
        objectcode = buff;

        if(strlen(TobjectCode) + strlen(objectcode) > 59 ){
          char* record = getTRecord(tStart, tSize, TobjectCode);
          writeTRecord(objectfile, record);
          tSize = 0;
          tStart = 0;
          TobjectCode[0] = '\0';
          tStart = LOCCTR;
          tSize = 3;
          printObjectCodeAddition(objectcode, line, LOCCTR);
          strcat(TobjectCode, objectcode);
        }else{
          printObjectCodeAddition(objectcode, line, LOCCTR);
          strcat(TobjectCode, objectcode);
          tSize += byteSize;
        }

        LOCCTR += 3;
      }else if(strcmp(opcode, "RESW") == 0){
        if(tSize > 0){
          char* record = getTRecord(tStart, tSize, TobjectCode);
          writeTRecord(objectfile, record);
          tSize = 0;
          tStart = 0;
          TobjectCode[0] = '\0';
        }
        printObjectCodeAddition("\t", line, LOCCTR);
        int num = (int)strtol(operand1, NULL, 10);
        LOCCTR += (3 * num);
      }else if(strcmp(opcode, "RESB") == 0){
        if(tSize > 0){
          char* record = getTRecord(tStart, tSize, TobjectCode);
          writeTRecord(objectfile, record);
          tSize = 0;
          tStart = 0;
          TobjectCode[0] = '\0';
        }
        printObjectCodeAddition("\t", line, LOCCTR);
        int num = (int)strtol(operand1, NULL, 10);
        LOCCTR += num;
      }
    }//else if directive
    count++;
  }//end while

  // displayTable(table);
  fclose(objectfile);
  fclose(inputFile);


  return 0;
}
