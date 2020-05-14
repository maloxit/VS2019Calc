// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#pragma warning(disable:4996)

#include "bool.h"
#include "rusctype.h"
#include "calc.h"


//Кол-во символов, считываемых за раз
#define READ_BUFF_LEN 20

//Результат попытки чтения
typedef enum inputResult_t {
  RESULT_OK,
  RESULT_ERROR_MEMORY_LACK,
  RESULT_ERROR_INPUT_END,
  RESULT_ERROR
} inputResult_t;

//Пропускает одну строку из стандартного входного потока
void SkipString() {
  scanf("%*[^\n]");
  getchar();
}

//Пытается считать одну строку из стандартного потока ввода и возвращает информацию о результате
inputResult_t ReadLine(char** strOut) {
  char * strBuffer, * strEndPoint, * memTry;
  char firstCharBuff;
  int arrayLength;
  int strLength;
  firstCharBuff = (char)getchar();
  if ((int)firstCharBuff == EOF) {
    return RESULT_ERROR_INPUT_END;
  }
  memTry = (char*)malloc(sizeof(char) * (READ_BUFF_LEN + 1));
  if (!memTry) {
    if (firstCharBuff != '\n') {
      SkipString();
    }
    return RESULT_ERROR_MEMORY_LACK;
  }
  strBuffer = memTry;
  arrayLength = READ_BUFF_LEN + 1;
  
  if (firstCharBuff == '\n') {
    strBuffer[0] = '\0';
    *strOut = strBuffer;
    return RESULT_OK;
  }
  strBuffer[0] = firstCharBuff;
  fgets(strBuffer + 1, READ_BUFF_LEN, stdin);
  strEndPoint = strBuffer;
  while (strlen(strEndPoint) == READ_BUFF_LEN && strEndPoint[READ_BUFF_LEN - 1] != '\n') {
    arrayLength += READ_BUFF_LEN;
    memTry = (char*)realloc(strBuffer, sizeof(char) * arrayLength);
    if (!memTry) {
      SkipString();
      free(strBuffer);
      return RESULT_ERROR_MEMORY_LACK;
    }
    strBuffer = memTry;
    strEndPoint = strBuffer + arrayLength - READ_BUFF_LEN - 1;
    fgets(strEndPoint, READ_BUFF_LEN + 1, stdin);
  }
  strLength = (int)strlen(strBuffer);
  if (strBuffer[strLength - 1] == '\n') {
    strBuffer[strLength - 1] = '\0';
  }
  *strOut = strBuffer;
  return RESULT_OK;
}

//Проверяет, является ли строка выражением (не комментарием)
Bool IsExpression(char* str) {
  int i = 0;
  while (str[i] != '\0' && MyIsSpace(str[i])) {
    i++;
  }
  if (str[i] == '\0') {
    return FALSE;
  }
  else {
    if (str[i] == '/' && str[i + 1] == '/') {
      return FALSE;
    }
    else {
      return TRUE;
    }
  }
}

int main(int argc, char* argv[]) {
  char* str = NULL;
  double ans;
  inputResult_t inputResult;
  calcResult_t err;
  //Смена кодировки, проверка корректности аргументов, попытка открытия файла для чтения
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
  //Последовательное чтение и обработка строк до конца файла
  for (inputResult = ReadLine(&str); inputResult != RESULT_ERROR_INPUT_END; inputResult = ReadLine(&str)) {
    if (inputResult == RESULT_ERROR_MEMORY_LACK) {
      printf("ERROR: Not enough memory\n");
      continue;
    }
    printf("%s", str);
    if (IsExpression(str)) {
      //Вызов вычислительного модуля
      err = StringCalc(str, &ans);
      //Вывод ошибки блока вычисления или ответа
      if (err.isError) {
        printf(" == ERROR: ");
        CalcResultPrint(err);
      }
      else {
        printf(" == %lg", ans);
      }
    }
    printf("\n");
    free(str);
  }
#ifdef _CRTDBG_MAP_ALLOC
  _CrtDumpMemoryLeaks();
#endif
  return 0;
}