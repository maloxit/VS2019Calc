// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#ifndef RESULT_CODES_H_INCLUDED__
#define RESULT_CODES_H_INCLUDED__
#pragma once

#include "bool.h"

//Коды результатов вычисления, используются внутри модуля
typedef enum resultCode_t {
  CRESULT_OK = 0,
  CRESULT_ERROR_VALUE_FORMAT,
  CRESULT_ERROR_UNKNOWN_LEX,
  CRESULT_ERROR_BRACKET,
  CRESULT_ERROR_MEMORY_LACK,
  CRESULT_ERROR_INVALID_EXPR,
  CRESULT_ERROR_INVALID_ARG_OR_HUGEVAL,
  CRESULT_ERROR
} resultCode_t;

//Проверяет, является ли результат ошибкой
Bool ResultCodeIsError(resultCode_t code);

#endif //RESULT_CODES_H_INCLUDED__