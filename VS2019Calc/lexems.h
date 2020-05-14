// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#ifndef LEXEMS_H_INCLUDED__
#define LEXEMS_H_INCLUDED__
#pragma once

#include "bool.h"
#include "expression.h"
#include "varlist.h"
#include "resultCodes.h"
#define LEX_LIST_LEN 25

#define M_PI 3.1415926535897932384626
#define M_E  2.71828182845904523536028747
#define COMP_EPSILON 1e-11

//Перчисление типов лексем
typedef enum lexType_t {
  LEX_TYPE_VALUE = -1,
  LEX_TYPE_COUPLE = -2,

  LEX_TYPE_OPEN = 0,
  LEX_TYPE_CLOSE,
  LEX_TYPE_SQRT,
  LEX_TYPE_SIN,
  LEX_TYPE_COS,
  LEX_TYPE_TG,
  LEX_TYPE_CTG,
  LEX_TYPE_ARCSIN,
  LEX_TYPE_ARCCOS,
  LEX_TYPE_ARCTG,
  LEX_TYPE_LN,
  LEX_TYPE_LOG,
  LEX_TYPE_FLOOR,
  LEX_TYPE_CEIL,
  LEX_TYPE_POW,
  LEX_TYPE_MULTIPLY,
  LEX_TYPE_DIVIDE,
  LEX_TYPE_MINUS,
  LEX_TYPE_PLUS,
  LEX_TYPE_COMMA,
  LEX_TYPE_EQUAL,
  LEX_TYPE_SEMICOLON,
  LEX_TYPE_PI,
  LEX_TYPE_E,
  LEX_TYPE_VAR
} lexType_t;


//Упорядоченная пара вещественных значений
typedef struct couple_t {
  double v1;
  double v2;
} couple_t;

//Значение лексемы, которое может быть парой, индексом локальной перемонной или одним числом
typedef union value_t {
  couple_t couple;
  unsigned int varIndex;
  double single;
} value_t;

struct _nodevalue_t {
  lexType_t type;
  value_t value;
};

//Прототип функции обработки лемы
typedef resultCode_t LemCalc_t(node_t* lex, node_t** start, node_t** end, nodevalue_t* insert);

//Символьное обозначение лексемы, и указатель на фукуцию для её обработки
typedef struct lex_t {
  char str[7];
  LemCalc_t* CalcFun;
}lex_t;

//Список символьных обозначений лексем, и указателей на фукуцию для их обработки
extern lex_t lexList[LEX_LIST_LEN];

//Кол-во групп операций с равным приоритетом выполнения
#define PRIORITY_GROUPS_COUNT 7

typedef Bool isMember_t(nodevalue_t * lex);

extern struct priorityGroups {
  isMember_t* IsMember;
  Bool isFromLeft;
} priorityGroups[PRIORITY_GROUPS_COUNT];


//Преобразует символьную строку в последовательность лексем, записаных в двусвязный список
//Также распознает однобуквенные локальные переменные
resultCode_t LexSplit(const char* const str, dblList_t** expression, varList_t** varList);

//Локальная глобальный указатель на текущий список локальных переменных
//Нужен для уменьшения кол-ва аргументов функций обработки лексем
extern varList_t* localVarList;

#endif //LEXEMS_H_INCLUDED__