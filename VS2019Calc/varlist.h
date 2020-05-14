// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#ifndef VARLIST_H_INCLUDED__
#define VARLIST_H_INCLUDED__
#pragma once

#include "bool.h"

//Начальная длина списка локальных переменных
#define VAR_LIST_BUFF_SIZE 10

//Локальная однобуквенныя переменная, имеет вычисленное и неопределённое состояния и значение
typedef struct var_t {
  double val;
  Bool isCalculated;
  char ch;
}var_t;

//Список локальных переменных с довыделяющейся памятью
typedef struct varList_t {
  var_t* at;
  int len;
  int size;
}varList_t;

//Создает пустой список локильных переменных
varList_t* VarListGet(void);

//Выгружает список локильных переменных из памяти
void VarListFree(varList_t** varList);

//Ищет по букве индекс локальной переменной в списке и возвращает его, или -1 в случае неудачи
int VarListGetIndexByChar(varList_t* varList, char ch);

//Добавляет элемент в конец списка локальных переменных
int VarListAppend(varList_t* varList, char ch);

#endif //VARLIST_H_INCLUDED__