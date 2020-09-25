#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct Symbol Symbol;

struct Symbol {
    char *key;
    int address;
    int line;
    struct Symbol* next;
};

typedef struct HashTable HashTable;

struct HashTable {
  Symbol** symbols;
  int size;
  int count;
};

HashTable* optable;

HashTable* new_table(int size);

 HashTable* new_table(int size) {
   // Creates a new HashTable
   HashTable* table = (HashTable*) malloc (sizeof(HashTable));
   table->size = size;
   table->count = 0;
   table->symbols = (Symbol**) calloc (table->size, sizeof(Symbol*));
   for (int i=0; i<table->size; i++)
       table->symbols[i] = NULL;

   return table;
}

int hashFunction(HashTable* table, char *key);

int hashFunction(HashTable* table, char *key) {
    int sumofchar = 0;
    for(int j=0; j<strlen(key); j++){
      sumofchar += key[j];
    }
    int index = sumofchar % table->size;

    return index;
}

void insert(HashTable* table, char* key, int value, int source_line) {
  // Creates a pointer to a new hash table item
    Symbol* symbol = (Symbol*) malloc (sizeof(Symbol));
    symbol->key = (char*) malloc (strlen(key) + 1);
    strcpy(symbol->key, key);
    symbol->address = value;
    symbol->line = source_line;
    symbol->next = NULL;

    int i = hashFunction(table, key);

    Symbol* curr_item = table->symbols[i];
    if (curr_item == NULL) {
      if (table->count == table->size) {
        // Hash Table Full
        printf("Insert Error: Hash Table is full\n");
        return;
      }

      table->symbols[i] = symbol;
      table->count++;
    }else {
      while(curr_item->next != NULL) {
        curr_item = curr_item->next;
      }
      curr_item->next = symbol;
    }
}

Symbol* get(HashTable* table, char* key);

Symbol* get(HashTable* table, char* key) {
  int index = hashFunction(table, key);

  Symbol* curr_item = table->symbols[index];

  if(curr_item != NULL){
    if(strcmp(curr_item->key, key) == 0){
      return curr_item;
    }else{
      while(curr_item->next != NULL){
        curr_item = curr_item->next;

        if(strcmp(curr_item->key, key) == 0){
          return curr_item;
        }
      }

      return NULL;
    }//end inner ifelse
  }else{
    return NULL;
  }
}

void insertOpcode(HashTable* table, char* name, char* hex);

void insertOpcode(HashTable* table, char* name, char* hex){
  int code = (int)strtol(hex, NULL, 16);
  insert(table, name, code, 0);
  return;
}

void createOpcodeTable() {
  // Creates a new HashTable
  optable = (HashTable*) malloc (sizeof(HashTable));
  optable->size = 127;
  optable->count = 0;
  optable->symbols = (Symbol**) calloc (optable->size, sizeof(Symbol*));

  insertOpcode(optable, "ADD", "18");
  insertOpcode(optable, "ADDF", "58");
  insertOpcode(optable, "ADDF", "58");
  insertOpcode(optable, "ADDR", "90");
  insertOpcode(optable, "AND", "40");
  insertOpcode(optable, "CLEAR", "B4");
  insertOpcode(optable, "COMP", "28");
  insertOpcode(optable, "COMPF", "88");
  insertOpcode(optable, "COMPR", "A0");
  insertOpcode(optable, "DIV", "24");
  insertOpcode(optable, "DIVF", "64");
  insertOpcode(optable, "DIVR", "9C");
  insertOpcode(optable, "FIX", "C4");
  insertOpcode(optable, "FLOAT", "C0");
  insertOpcode(optable, "HIO", "F4");
  insertOpcode(optable, "J", "3C");
  insertOpcode(optable, "JEQ", "30");
  insertOpcode(optable, "JGT", "34");
  insertOpcode(optable, "JLT", "38");
  insertOpcode(optable, "JSUB", "48");
  insertOpcode(optable, "LDA", "00");
  insertOpcode(optable, "LDB", "68");
  insertOpcode(optable, "LDCH", "50");
  insertOpcode(optable, "LDF", "70");
  insertOpcode(optable, "LDL", "08");
  insertOpcode(optable, "LDS", "6C");
  insertOpcode(optable, "LDT", "74");
  insertOpcode(optable, "LDX", "04");
  insertOpcode(optable, "LPS", "D0");
  insertOpcode(optable, "MUL", "20");
  insertOpcode(optable, "MULF", "60");
  insertOpcode(optable, "MULR", "98");
  insertOpcode(optable, "NORM", "44");
  insertOpcode(optable, "OR", "44");
  insertOpcode(optable, "RD", "D8");
  insertOpcode(optable, "RMO", "AC");
  insertOpcode(optable, "RSUB", "4C");
  insertOpcode(optable, "SHIFTL", "A4");
  insertOpcode(optable, "SHIFTR", "A8");
  insertOpcode(optable, "SIO", "FO");
  insertOpcode(optable, "SSK", "EC");
  insertOpcode(optable, "STA", "OC");
  insertOpcode(optable, "STB", "78");
  insertOpcode(optable, "STCH", "54");
  insertOpcode(optable, "STF", "80");
  insertOpcode(optable, "STI", "D4");
  insertOpcode(optable, "STL", "14");
  insertOpcode(optable, "STS", "7C");
  insertOpcode(optable, "STSW", "E8");
  insertOpcode(optable, "STT", "84");
  insertOpcode(optable, "STX", "10");
  insertOpcode(optable, "SUB", "1C");
  insertOpcode(optable, "SUBF", "5C");
  insertOpcode(optable, "SUBR", "94");
  insertOpcode(optable, "SVC", "B0");
  insertOpcode(optable, "TD", "E0");
  insertOpcode(optable, "TIO", "F8");
  insertOpcode(optable, "TIX", "2C");
  insertOpcode(optable, "TIXR", "B8");
  insertOpcode(optable, "WD", "DC");

  return;
}

bool isInstruction(char* key){
  Symbol* opcode = get(optable, key);
  if(opcode != NULL){
    return true;
  }else{
    return false;
  }
}

bool isADirective(char* sym){
  if(strcmp(sym, "START") == 0){
    return true;
  }else if(strcmp(sym, "END")== 0){
    return true;
  }else if(strcmp(sym, "BYTE") == 0){
    return true;
  }else if(strcmp(sym, "WORD") == 0){
    return true;
  }else if(strcmp(sym, "RESB") == 0){
    return true;
  }else if(strcmp(sym, "RESW") == 0){
    return true;
  }else if(strcmp(sym, "RESR") == 0){
    return true;
  }else if(strcmp(sym, "EXPORTS") == 0){
    return true;
  }else{
    return false;
  }
}

void display(HashTable* table){
  for(int i = 0; i<table->size; i++){
    Symbol* symbol = table->symbols[i];

    if(symbol != NULL){
      printf("%d (%s, %x)",i, symbol->key, symbol->address);
      while(symbol->next != NULL){
        symbol = symbol->next;
        printf("->(%s, %x)", symbol->key, symbol->address);
      }
      printf("\n");
    }
  }
}

void displayTable(HashTable* table){
  for(int i = 0; i<table->size; i++){
    Symbol* symbol = table->symbols[i];

    if(symbol != NULL){
      printf("%s\t%x\n", symbol->key, symbol->address);
      while(symbol->next != NULL){
        symbol = symbol->next;
        printf("%s\t%x\n", symbol->key, symbol->address);
      }
    }
  }
}
