// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <windows.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#pragma warning(disable:4996)
#include "calc.h"
#define BUFF_LEN 20

typedef enum inputResult_t {
  RESULT_OK,
  RESULT_ERROR_MEMORY_LACK,
  RESULT_ERROR_INPUT_END,
  RESULT_ERROR
} inputResult_t;

void SkipString() {
  scanf("%*[^\n]");
  getchar();
}

inputResult_t ReadLine(char** strOut) {
  char* strBuffer, * strEndPoint, * memTry;
  unsigned int arrayLength;
  if (feof(stdin))
    return RESULT_ERROR_INPUT_END;
  memTry = (char*)malloc(sizeof(char) * (BUFF_LEN + 1));
  if (!memTry) {
    SkipString();
    return RESULT_ERROR_MEMORY_LACK;
  }
  strBuffer = memTry;
  arrayLength = BUFF_LEN + 1;
  strEndPoint = strBuffer;
  while (fgets(strEndPoint, BUFF_LEN + 1, stdin) != NULL) {
    if (strlen(strEndPoint) == BUFF_LEN && strEndPoint[BUFF_LEN - 1] != '\n') {
      arrayLength += BUFF_LEN;
      memTry = (char*)realloc(strBuffer, sizeof(char) * arrayLength);
      if (!memTry) {
        SkipString();
        free(strBuffer);
        return RESULT_ERROR_MEMORY_LACK;
      }
      strBuffer = memTry;
      strEndPoint = strBuffer + arrayLength - BUFF_LEN - 1;
    }
    else {
      unsigned int strLength = strlen(strBuffer);
      if (strBuffer[strLength - 1] == '\n')
        strBuffer[strLength - 1] = '\0';
      *strOut = strBuffer;
      return RESULT_OK;
    }
  }
  free(strBuffer);
  return RESULT_ERROR_INPUT_END;
}

int IsExpression(char* str) {
  int i = 0;
  while (str[i] != '\0' && MyIsSpace(str[i])) {
    i++;
  }
  if (str[i] == '\0')
    return 0;
  else {
    if (str[i] == '/' && str[i + 1] == '/')
      return 0;
    else
      return 1;
  }
}

int main(int argc, char* argv[]) {
  char* str = NULL;
  double ans;
  inputResult_t inputResult;
  calcResult_t err;
  SetConsoleOutputCP(1251);
  if (argc > 2) {
    printf("ERROR: invalid number of arguments\n");
    return -1;
  }
  if (argc == 2 && !freopen(argv[1], "r", stdin)) {
    printf("ERROR: invalid file name\n");
    return -1;
  }
  SetConsoleCP(1251);
  while ((inputResult = ReadLine(&str)) != RESULT_ERROR_INPUT_END) {
    if (inputResult == RESULT_ERROR_MEMORY_LACK) {
      printf("ERROR: Not enough memory\n");
      continue;
    }
    printf("%s", str);
    if (IsExpression(str)) {
      err = StringCalc(str, &ans);
      if (err.isError) {
        printf(" == ERROR: ");
        CalcPrintError(err);
      }
      else
        printf(" == %lg", ans);
    }
    printf("\n");
    free(str);
  }
#ifdef _CRTDBG_MAP_ALLOC
  _CrtDumpMemoryLeaks();
#endif
  return 0;
}