// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "resultCodes.h"

//Проверяет, является ли результат ошибкой
Bool ResultCodeIsError(resultCode_t code) {
  return (code != CRESULT_OK);
}