/* 
Program: Instruction
Creator: Jessica Nguyen
Date: 2023-11-18

Parse tree using ASCII graphics
   - Original NCurses code from "Game Programming in C with the Ncurses Library"
   https://www.viget.com/articles/game-programming-in-c-with-the-ncurses-library/
   and from "NCURSES Programming HOWTO"
   http://tldp.org/HOWTO/NCURSES-Programming-HOWTO/
*/ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef NOGRAPHICS
   #include <unistd.h>
   #include <ncurses.h>
#endif

// declaring defines
#define SCREENSIZE 200
#define STRINGSIZE 10
#define DATASIZE 1000

// declaring structs
typedef struct dataList {
   int lineNumber;
   char command[STRINGSIZE+1], argOne[STRINGSIZE+1], argTwo[STRINGSIZE+1], argThree[STRINGSIZE+1];
} dataList;
typedef struct variableList {
   char name[STRINGSIZE+1];
   char value[STRINGSIZE+1];
} variableList;

// declaring functions
void set(dataList *data, variableList *variable, int dataIndex, int variableSize);
void arithmetic(dataList *data, variableList *variable, int dataIndex, int variableSize);
int iff(dataList *data, variableList *variable, int dataIndex, int variableSize);
int go(dataList *data, int specifiedLineNum, int dataSize);
void printData(dataList *data, variableList *variable, int dataIndex, int variableSize);
#ifndef NOGRAPHICS
   // curses output
   // row indicates in which row the output will start - larger numbers move down
   // col indicates in which column the output will start - larger numbers move to the right
   // when row,col == 0,0 it is the upper left hand corner of the window
   void print(int row, int col, char *str) {
      mvprintw(row, col, str);
   }
#endif

int main(int argc, char *argv[]) {
   // checks if an input was obtained from the command line
   if (argc == 2) {
      // declaring variables
      // i represents the pointer counter
      int c, i, dataSize, variableSize, pcData, pcVariable, wordCount;
      char *word, fileLine[DATASIZE+1];
      dataList *data;
      variableList *variable;
      FILE *file;

      // initializing variables
      dataSize = 0;
      variableSize = 0;
      pcData = 0;
      pcVariable = 0;
      data = (dataList*)malloc((DATASIZE+1)*sizeof(dataList));
      variable = (variableList*)malloc((DATASIZE+1)*sizeof(variableList));
      file = NULL;

      #ifndef NOGRAPHICS
         // initialize ncurses
         initscr();
         noecho();
         cbreak();
         timeout(0);
         curs_set(FALSE);
      #endif

      /* read and interpret the file starting here */
      file = fopen(argv[1], "r");
      
      // dummy proof: checks if the file doesn't exist
      if (file == NULL){
         printf("\n%s does not exist.\n\n", argv[1]);

         // free malloced variables
         free(data);
         free(variable);

         return 0;
      }
      
      // moves to the start of the file
      rewind (file);

      // obtains one row of text from the file
      while (fgets(fileLine, DATASIZE, file)) {
         // removes the newline at the end of each fgets()
         if (fileLine[strlen(fileLine)-1] == '\n') {
            fileLine[strlen(fileLine)-1] = '\0';
         }

         // parses the text to the line number, command, and the 3 arguments
         wordCount = 0;
         word = strtok(fileLine, " \n\r\t");
         data[dataSize].lineNumber = atoi(word);
         wordCount++;
         word = strtok(NULL, " \n\r\t");
         while (word != NULL && wordCount <= 4) {
            if (wordCount == 1) {
               strcpy(data[dataSize].command, word);
            } else if (wordCount == 2) {
               strcpy(data[dataSize].argOne, word);
            } else if (wordCount == 3) {
               strcpy(data[dataSize].argTwo, word);
            } else if (wordCount == 4) {
               strcpy(data[dataSize].argThree, word);
            }
            wordCount++;
            word = strtok(NULL, " \n\r\t");
         }

         // go to the next row of the file
         dataSize++;
      }
        
      // close the file
      fclose(file);

      // start interpreting the given commands
      // no need to do anything for "begin" or "end"
      for (i = 0; i < dataSize; i++) {
         if (strcmp(data[i].command, "int") == 0) {
            strcpy(variable[variableSize++].name, data[i].argOne);
         } else if (strcmp(data[i].command, "set") == 0) {
            set(data, variable, i, variableSize);
         } else if (strcmp(data[i].command, "add") == 0 || strcmp(data[i].command, "sub") == 0 || strcmp(data[i].command, "mult") == 0 || strcmp(data[i].command, "div") == 0) {
            arithmetic(data, variable, i, variableSize);
         } else if (strcmp(data[i].command, "print") == 0) {
            printData(data, variable, i, variableSize);
         } else if (strcmp(data[i].command, "goto") == 0) {
            // change the pointer counter based on the line number
            i = go(data, atoi(data[i].argOne), dataSize);
         } else if (strcmp(data[i].command, "if") == 0) {
            // if the statement is false, jump over the next line
            if (iff(data, variable, i, variableSize) == 0){
               i++;
            }
         }
      }
      printf("\n\n");

      #ifndef NOGRAPHICS
         /* loop until the 'q' key is pressed */
         while(1) {
            c = getch();
            if (c == 'q') break;
         }
         // shut down ncurses
         endwin();
      #endif

      // free malloced variables
      free(data);
      free(variable);
   } else if (argc > 2) {
      // if more than 1 file was given, output this message
      printf("Too many files were given in the command line.\n");
   } else {
      // if no files were given, output this message
      printf ("No files were given in the command line.\n");
   }
   return 0;
}

// set() sets variable values to the data arguments
void set(dataList *data, variableList *variable, int dataIndex, int variableSize) {
   // declaring variable
   int i;

   // finds the matching variable and set its value 
   for (i = 0; i < variableSize; i++) {
      if (strcmp(data[dataIndex].argOne, variable[i].name) == 0) {
         strcpy(variable[i].value, data[dataIndex].argTwo);
         i = variableSize;
      }
   }
}

// arithmetic() does an arithmetic operation on the stated variable
void arithmetic(dataList *data, variableList *variable, int dataIndex, int variableSize) {
   // declaring variables
   int i, num;
   
   // finds the matching variable and does the arithmetic
   for (i = 0; i < variableSize; i++) {
      if (strcmp(data[dataIndex].argOne, variable[i].name) == 0) {
         // if the variable doesn't have a value already, default its value to zero
         if (strlen(variable[i].value) == 0) {
            strcpy(variable[i].value, "0");
         }

         // add/sub/mult/div the new number into the variable value
         if (strcmp(data[dataIndex].command, "add") == 0) {
            num = atoi(variable[i].value) + atoi(data[dataIndex].argTwo);
         } else if (strcmp(data[dataIndex].command, "sub") == 0) {
            num = atoi(variable[i].value) - atoi(data[dataIndex].argTwo);
         } else if (strcmp(data[dataIndex].command, "mult") == 0) {
            num = atoi(variable[i].value) * atoi(data[dataIndex].argTwo);
         } else if (strcmp(data[dataIndex].command, "div") == 0) {
            num = atoi(variable[i].value) / atoi(data[dataIndex].argTwo);
         }

         // put that number back into the variable struct
         sprintf(variable[i].value, "%d", num);

         // end the for loop
         i = variableSize;
      }
   }
}

// iff() returns true or false of a given if statement
int iff(dataList *data, variableList *variable, int dataIndex, int variableSize) {
   // declaring variables
   int i;
   char varA[STRINGSIZE+1], varB[STRINGSIZE+1];

   // initializing variables
   for (i = 0; i < variableSize; i++) {
      if (strcmp(data[dataIndex].argOne, variable[i].name) == 0) {
         strcpy(varA, variable[i].value);
      }
   }
   for (i = 0; i < variableSize; i++) {
      if (strcmp(data[dataIndex].argThree, variable[i].name) == 0) {
         strcpy(varB, variable[i].value);
      }
   }

   // compare values and return true (1) or false (0)
   if (strcmp(data[dataIndex].argTwo, "eq") == 0) {
      return atoi(varA) == atoi(varB);
   } else if (strcmp(data[dataIndex].argTwo, "ne") == 0) {
      return atoi(varA) != atoi(varB);
   } else if (strcmp(data[dataIndex].argTwo, "gt") == 0) {
      return atoi(varA) > atoi(varB);
   } else if (strcmp(data[dataIndex].argTwo, "gte") == 0) {
      return atoi(varA) >= atoi(varB);
   } else if (strcmp(data[dataIndex].argTwo, "lt") == 0) {
      return atoi(varA) < atoi(varB);
   } else if (strcmp(data[dataIndex].argTwo, "lte") == 0) {
      return atoi(varA) <= atoi(varB);
   }

   return 0;
}

// go() finds the next location for the pointer counter
int go(dataList *data, int specifiedLineNum, int dataSize) {
   // declaring variable
   int i;

   // finds the matching line number and sets the pointer counter to the new index value
   for (i = 0; i < dataSize; i++) {
      if (data[i].lineNumber == specifiedLineNum) {
         // -1 because the for loop in main() will +1
         return i-1;
      }
   }

   return i-1;
}

// printData() will print what the file wants to be printed
void printData(dataList *data, variableList *variable, int dataIndex, int variableSize) {
   // declaring variables
   int i;
   char varA[STRINGSIZE+1], varB[STRINGSIZE+1];

   // initializing variables
   for (i = 0; i < variableSize; i++) {
      if (strcmp(data[dataIndex].argOne, variable[i].name) == 0) {
         strcpy(varA, variable[i].value);
      }
   }
   for (i = 0; i < variableSize; i++) {
      if (strcmp(data[dataIndex].argTwo, variable[i].name) == 0) {
         strcpy(varB, variable[i].value);
      }
   }

   // print with no graphics
   printf("\n%s %s %s", varA, varB, data[dataIndex].argThree);

   // print with graphics
   #ifndef NOGRAPHICS
      print(atoi(varA), atoi(varB), data[dataIndex].argThree);
   #endif
}