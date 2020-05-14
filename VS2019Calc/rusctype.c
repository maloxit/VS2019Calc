// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "rusctype.h"

//Функции работы с символами, аналогичные стандартным, но с поддержкой кирилицы
Bool MyIsSpace(char ch) {
  return (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\v' || ch == '\f' || ch == '\r');
}

Bool MyIsDigit(char ch) {
  return (ch >= '0' && ch <= '9');
}

Bool MyIsAlpha(char ch) {
  return ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'));
}