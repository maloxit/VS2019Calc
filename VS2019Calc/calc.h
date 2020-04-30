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
  Bool isError;     //Является ошибкой
  char text[35];    //Текстовое описание разультата
} calcResult_t;

Bool MyIsSpace(char ch);

Bool MyIsDigit(char ch);

Bool MyIsAlpha(char ch);

void CalcResultPrint(calcResult_t err);

calcResult_t StringCalc(const char* const str, double* ans);

#endif //CALC_H_INCLUDED__