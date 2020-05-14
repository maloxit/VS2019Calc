// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#ifndef RUSCTYPE_H_INCLUDED__
#define RUSCTYPE_H_INCLUDED__
#pragma once
#include "bool.h"

//Функции работы с символами, аналогичные стандартным, но с поддержкой кирилицы
Bool MyIsSpace(char ch);
Bool MyIsDigit(char ch);
Bool MyIsAlpha(char ch);


#endif //RUSCTYPE_H_INCLUDED__