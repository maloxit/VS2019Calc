// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#ifndef CALC_H_INCLUDED__
#define CALC_H_INCLUDED__
#pragma once

#define TRUE 1
#define FALSE 0
typedef int Bool;

//Результат работы модуля
typedef struct calcResult_t {
  Bool isError;     //Является ли ошибкой
  char text[35];    //Текстовое описание разультата
} calcResult_t;

//Функции работы с символами, аналогичные стандартным, но с поддержкой кирилицы
Bool MyIsSpace(char ch);
Bool MyIsDigit(char ch);
Bool MyIsAlpha(char ch);

//Выводит в стандартный поток вывода текст результата вычисления (ошибки)
void CalcResultPrint(calcResult_t err);

//Пытается обработать переданную строку как математическое выражение
//Возвращает информацию о результате, и, если попытка удачна, численный ответ
calcResult_t StringCalc(const char* const str, double* ans);

#endif //CALC_H_INCLUDED__