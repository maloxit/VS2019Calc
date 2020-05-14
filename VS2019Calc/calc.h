// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#ifndef CALC_H_INCLUDED__
#define CALC_H_INCLUDED__
#pragma once

#include "bool.h"

//Результат работы модуля
typedef struct calcResult_t {
  Bool isError;     //Является ли ошибкой
  char text[35];    //Текстовое описание разультата
} calcResult_t;


//Выводит в стандартный поток вывода текст результата вычисления (ошибки)
void CalcResultPrint(calcResult_t err);

//Пытается обработать переданную строку как математическое выражение
//Возвращает информацию о результате, и, если попытка удачна, численный ответ
calcResult_t StringCalc(const char* const str, double* ans);

#endif //CALC_H_INCLUDED__