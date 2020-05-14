// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "lexems.h"
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "rusctype.h"
//Локальная глобальный указатель на текущий список локальных переменных
//Нужен для уменьшения кол-ва аргументов функций обработки лексем
varList_t* localVarList;

//Прототип унарной математической функции 
typedef double UnarOp_t(double arg1);

//Тангенс с проверкой деления на ноль, использует errno
static double MyTan(double arg1) {
  if (fabs(cos(arg1)) <= COMP_EPSILON) {
    errno = EDOM;
    return 0;
  }
  else {
    return tan(arg1);
  }
}

//Котангенс с проверкой деления на ноль, использует errno
static double MyCtan(double arg1) {
  return MyTan(M_PI / 2 - arg1);
}

//Прототип бинарной математической функции 
typedef double BinaryOp_t(double arg1, double arg2);

//Деление с проверкой деления на ноль, использует errno
static double Divide(double arg1, double arg2) {
  if (fabs(arg2) <= COMP_EPSILON) {
    errno = EDOM;
    return 0;
  }
  return arg1 / arg2;
}

//Умножение вещественных чисел
static double Multiply(double arg1, double arg2) {
  return arg1 * arg2;
}

//Сложение вещественных чисел
static double Plus(double arg1, double arg2) {
  return arg1 + arg2;
}

//Обобщающий процесс обработки лексемы бинарной операции
static resultCode_t BinaryProcess(BinaryOp_t* operation, node_t* lex, node_t** start, node_t** end, nodevalue_t* insert) {
  if (!(lex->previous && lex->next)) {
    return CRESULT_ERROR_INVALID_EXPR;
  }
  if (!(lex->previous->nodeVal->type == LEX_TYPE_VALUE && lex->next->nodeVal->type == LEX_TYPE_VALUE)) {
    return CRESULT_ERROR_INVALID_EXPR;
  }
  *start = lex->previous;
  *end = lex->next;
  insert->type = LEX_TYPE_VALUE;
  errno = 0;
  insert->value.single = operation(lex->previous->nodeVal->value.single, lex->next->nodeVal->value.single);
  if (errno != 0) {
    errno = 0;
    return CRESULT_ERROR_INVALID_ARG_OR_HUGEVAL;
  }
  return CRESULT_OK;
}

//Обобщающий процесс обработки лексемы унарной операции
static resultCode_t UnarProcess(UnarOp_t* operation, node_t* lex, node_t** start, node_t** end, nodevalue_t* insert) {
  if (!(lex->next && lex->next->nodeVal->type == LEX_TYPE_VALUE)) {
    return CRESULT_ERROR_INVALID_EXPR;
  }
  *start = lex;
  *end = lex->next;
  insert->type = LEX_TYPE_VALUE;
  errno = 0;
  insert->value.single = operation(lex->next->nodeVal->value.single);
  if (errno != 0) {
    errno = 0;
    return CRESULT_ERROR_INVALID_ARG_OR_HUGEVAL;
  }
  return CRESULT_OK;
}

//Однотипные функции обработки лексем
/*/*/
static resultCode_t SqrtProcess(node_t* lex, node_t** start, node_t** end, nodevalue_t* insert) {
  return UnarProcess(sqrt, lex, start, end, insert);
}

static resultCode_t SinProcess(node_t* lex, node_t** start, node_t** end, nodevalue_t* insert) {
  return UnarProcess(sin, lex, start, end, insert);
}

static resultCode_t CosProcess(node_t* lex, node_t** start, node_t** end, nodevalue_t* insert) {
  return UnarProcess(cos, lex, start, end, insert);
}

static resultCode_t TgProcess(node_t* lex, node_t** start, node_t** end, nodevalue_t* insert) {
  return UnarProcess(MyTan, lex, start, end, insert);
}

static resultCode_t CtgProcess(node_t* lex, node_t** start, node_t** end, nodevalue_t* insert) {
  return UnarProcess(MyCtan, lex, start, end, insert);
}

static resultCode_t ArcsinProcess(node_t* lex, node_t** start, node_t** end, nodevalue_t* insert) {
  return UnarProcess(asin, lex, start, end, insert);
}

static resultCode_t ArccosProcess(node_t* lex, node_t** start, node_t** end, nodevalue_t* insert) {
  return UnarProcess(acos, lex, start, end, insert);
}

static resultCode_t ArctgProcess(node_t* lex, node_t** start, node_t** end, nodevalue_t* insert) {
  return UnarProcess(atan, lex, start, end, insert);
}

static resultCode_t LnProcess(node_t* lex, node_t** start, node_t** end, nodevalue_t* insert) {
  return UnarProcess(log, lex, start, end, insert);
}

static resultCode_t FloorProcess(node_t* lex, node_t** start, node_t** end, nodevalue_t* insert) {
  return UnarProcess(floor, lex, start, end, insert);
}

static resultCode_t CeilProcess(node_t* lex, node_t** start, node_t** end, nodevalue_t* insert) {
  return UnarProcess(ceil, lex, start, end, insert);
}

static resultCode_t PowProcess(node_t* lex, node_t** start, node_t** end, nodevalue_t* insert) {
  return BinaryProcess(pow, lex, start, end, insert);
}

static resultCode_t MultiplyProcess(node_t* lex, node_t** start, node_t** end, nodevalue_t* insert) {
  return BinaryProcess(Multiply, lex, start, end, insert);
}

static resultCode_t DivideProcess(node_t* lex, node_t** start, node_t** end, nodevalue_t* insert) {
  return BinaryProcess(Divide, lex, start, end, insert);
}
/*/*/

//Обработка лексемы логарифма от перы значений (по произвольному основанию) через отношение нормальных логарифмов
static resultCode_t LogProcess(node_t* lex, node_t** start, node_t** end, nodevalue_t* insert) {
  double buff;
  if (!(lex->next && lex->next->nodeVal->type == LEX_TYPE_COUPLE)) {
    return CRESULT_ERROR_INVALID_EXPR;
  }
  *start = lex;
  *end = lex->next;
  insert->type = LEX_TYPE_VALUE;
  errno = 0;
  buff = log(lex->next->nodeVal->value.couple.v1);
  if (errno != 0 || fabs(buff) <= COMP_EPSILON) {
    errno = 0;
    return CRESULT_ERROR_INVALID_ARG_OR_HUGEVAL;
  }
  insert->value.single = log(lex->next->nodeVal->value.couple.v2);
  if (errno != 0) {
    errno = 0;
    return CRESULT_ERROR_INVALID_ARG_OR_HUGEVAL;
  }
  insert->value.single /= buff;
  if (errno != 0) {
    errno = 0;
    return CRESULT_ERROR_INVALID_ARG_OR_HUGEVAL;
  }
  return CRESULT_OK;
}

//Обработка лексемы умнарного и бинарного минуса, и также повторяющихся минусов
static resultCode_t MinusProcess(node_t* lex, node_t** start, node_t** end, nodevalue_t* insert) {
  node_t* item = lex;
  int i = 1;
  if (!(lex->next && lex->next->nodeVal->type == LEX_TYPE_VALUE)) {
    return CRESULT_ERROR_INVALID_EXPR;
  }
  while (item->previous && item->nodeVal->type == LEX_TYPE_MINUS) {
    item = item->previous;
    i++;
  }
  if (item->nodeVal->type != LEX_TYPE_VALUE) {
    if (item->nodeVal->type == LEX_TYPE_MINUS) {
      *start = item;
      i++;
    }
    else {
      *start = item->next;
    }
    if (lex->next->next && lex->next->next->nodeVal->type == LEX_TYPE_POW) {
      *end = lex;
      if (i % 2 == 0) {
        insert->type = LEX_TYPE_MINUS;
      }
      else {
        insert->type = LEX_TYPE_PLUS;
      }
    }
    else {
      *end = lex->next;
      insert->type = LEX_TYPE_VALUE;
      if (i % 2 == 0) {
        insert->value.single = -lex->next->nodeVal->value.single;
      }
      else {
        insert->value.single = lex->next->nodeVal->value.single;
      }
    }
  }
  else {
    *end = lex;
    *start = item->next;

    insert->type = LEX_TYPE_PLUS;
    if (i % 2 == 0) {
      if (lex->next->next && lex->next->next->nodeVal->type == LEX_TYPE_POW) {
        insert->type = LEX_TYPE_MINUS;
      }
      else {
        lex->next->nodeVal->value.single *= -1;
      }
    }
  }
  return CRESULT_OK;
}

//Обработка лексемы бинарного минуса
static resultCode_t PlusProcess(node_t* lex, node_t** start, node_t** end, nodevalue_t* insert) {
  if (!(lex->previous && lex->next && lex->previous->nodeVal->type == LEX_TYPE_VALUE)) {
    return CRESULT_ERROR_INVALID_EXPR;
  }
  if (lex->next->nodeVal->type == LEX_TYPE_VALUE) {
    return BinaryProcess(Plus, lex, start, end, insert);
  }
  else {
    return CRESULT_ERROR_INVALID_EXPR;
  }
}

//Обработка лексемы запятой (скобки при логарифме). Объединяет зоседние значения в пару
static resultCode_t CommaProcess(node_t* lex, node_t** start, node_t** end, nodevalue_t* insert) {
  if (!(lex->previous && lex->next)) {
    return CRESULT_ERROR_INVALID_EXPR;
  }
  if (!(lex->previous->nodeVal->type == LEX_TYPE_VALUE && lex->next->nodeVal->type == LEX_TYPE_VALUE)) {
    return CRESULT_ERROR_INVALID_EXPR;
  }
  *start = lex->previous;
  *end = lex->next;
  insert->type = LEX_TYPE_COUPLE;
  insert->value.couple.v1 = lex->previous->nodeVal->value.single;
  insert->value.couple.v2 = lex->next->nodeVal->value.single;

  return CRESULT_OK;
}

//Обработка лексемы присваивания значения временной переменной
static resultCode_t EqualProcess(node_t* lex, node_t** start, node_t** end, nodevalue_t* insert) {
  if (!(lex->previous && lex->next)) {
    return CRESULT_ERROR_INVALID_EXPR;
  }
  if (!(lex->previous->nodeVal->type == LEX_TYPE_VAR && lex->next->nodeVal->type == LEX_TYPE_VALUE)) {
    return CRESULT_ERROR_INVALID_EXPR;
  }
  *start = lex->previous;
  *end = lex->next;
  insert->type = LEX_TYPE_VALUE;
  insert->value.single = lex->next->nodeVal->value.single;
  localVarList->at[lex->previous->nodeVal->value.varIndex].val = lex->next->nodeVal->value.single;
  localVarList->at[lex->previous->nodeVal->value.varIndex].isCalculated = TRUE;

  return CRESULT_OK;
}

//Обработка локальной переменной. Если значение уже вычислено, замна на это значение, иначе пропускается
static resultCode_t VarProcess(node_t* lex, node_t** start, node_t** end, nodevalue_t* insert) {
  *start = lex;
  *end = lex;
  if (localVarList->at[lex->nodeVal->value.varIndex].isCalculated) {
    insert->type = LEX_TYPE_VALUE;
    insert->value.single = localVarList->at[lex->nodeVal->value.varIndex].val;
  }
  else {
    insert->type = LEX_TYPE_VAR;
    insert->value = lex->nodeVal->value;
  }
  return CRESULT_OK;
}

//Список символьных обозначений лексем, и указателей на фукуцию для их обработки
lex_t lexList[LEX_LIST_LEN] = {
  {"(", NULL},
  {")", NULL},
  {"sqrt", SqrtProcess},
  {"sin", SinProcess},
  {"cos", CosProcess},
  {"tg", TgProcess},
  {"ctg", CtgProcess},
  {"arcsin", ArcsinProcess},
  {"arccos", ArccosProcess},
  {"arctg", ArctgProcess},
  {"ln", LnProcess},
  {"log", LogProcess},
  {"floor", FloorProcess},
  {"ceil", CeilProcess},
  {"^", PowProcess},
  {"*", MultiplyProcess},
  {"/", DivideProcess},
  {"-", MinusProcess},
  {"+", PlusProcess},
  {",", CommaProcess},
  {"=", EqualProcess},
  {";", NULL},
  {"pi", NULL},
  {"e", NULL},
  {"\n", VarProcess}
};

//Однотипные функции, определяющие, принадлежит ли лексема группе приоритета операций
/*/*/

static Bool IsGoup0Member(nodevalue_t* lex) {
  return (lex->type == LEX_TYPE_VAR);
}

static Bool IsGoup1Member(nodevalue_t* lex) {
  return ((lex->type >= LEX_TYPE_SQRT && lex->type <= LEX_TYPE_CEIL) || lex->type == LEX_TYPE_MINUS);
}

static Bool IsGoup2Member(nodevalue_t* lex) {
  return (lex->type == LEX_TYPE_POW || lex->type == LEX_TYPE_MINUS);
}

static Bool IsGoup3Member(nodevalue_t* lex) {
  return (lex->type >= LEX_TYPE_MULTIPLY && lex->type <= LEX_TYPE_DIVIDE);
}

static Bool IsGoup4Member(nodevalue_t* lex) {
  return (lex->type >= LEX_TYPE_MINUS && lex->type <= LEX_TYPE_PLUS);
}

static Bool IsGoup5Member(nodevalue_t* lex) {
  return (lex->type == LEX_TYPE_COMMA);
}

static Bool IsGoup6Member(nodevalue_t* lex) {
  return (lex->type == LEX_TYPE_EQUAL);
}
/*/*/
struct priorityGroups priorityGroups[PRIORITY_GROUPS_COUNT] = {
 {IsGoup0Member, TRUE},
 {IsGoup1Member, FALSE},
 {IsGoup2Member, FALSE},
 {IsGoup3Member, TRUE},
 {IsGoup4Member, TRUE},
 {IsGoup5Member, TRUE},
 {IsGoup6Member, FALSE}
};

//Преобразует символьную строку в последовательность лексем, записаных в двусвязный список
//Также распознает однобуквенные локальные переменные
resultCode_t LexSplit(const char* const str, dblList_t** expression, varList_t** varList) {
  int i, k, typeSet, varIndex;
  char* endPtr = NULL;
  double singleBuff;
  nodevalue_t valBuff;
  resultCode_t localResult = CRESULT_OK;
  (*varList) = VarListGet();
  if (!(*varList)) {
    return CRESULT_ERROR_MEMORY_LACK;
  }
  *expression = DblListCreate();
  if (!(*expression)) {
    VarListFree(varList);
    return CRESULT_ERROR_MEMORY_LACK;
  }
  for (i = 0; !ResultCodeIsError(localResult) && str[i] != '\0'; i++) {
    if (MyIsSpace(str[i])) {
      continue;
    }
    if (MyIsDigit(str[i])) {
      singleBuff = strtod(str + i, &endPtr);
      if (endPtr == str + i) {
        localResult = CRESULT_ERROR_VALUE_FORMAT;
      }
      else {
        valBuff.value.single = singleBuff;
        valBuff.type = LEX_TYPE_VALUE;
        if (!DblListAppend(*expression, &valBuff)) {
          localResult = CRESULT_ERROR_MEMORY_LACK;
        }
        i = (int)(endPtr - str - 1);
      }
    }
    else {
      for (k = 0; k < LEX_LIST_LEN; k++) {
        if (strncmp(str + i, lexList[k].str, strlen(lexList[k].str)) == FALSE) {
          break;
        }
      }
      if (k == LEX_LIST_LEN) {
        if (MyIsAlpha(str[i])) {
          varIndex = VarListGetIndexByChar((*varList), str[i]);
          if (varIndex < 0) {
            varIndex = VarListAppend((*varList), str[i]);
          }
          if (varIndex < 0) {
            localResult = CRESULT_ERROR_UNKNOWN_LEX;
          }
          else {
            valBuff.value.varIndex = varIndex;
            valBuff.type = LEX_TYPE_VAR;
            if (!DblListAppend(*expression, &valBuff)) {
              localResult = CRESULT_ERROR_MEMORY_LACK;
            }
          }

        }
        else {
          localResult = CRESULT_ERROR_UNKNOWN_LEX;
        }
      }
      else {
        singleBuff = 0;
        typeSet = k;
        if (typeSet == LEX_TYPE_PI) {
          singleBuff = M_PI;
          typeSet = LEX_TYPE_VALUE;
        }
        if (typeSet == LEX_TYPE_E) {
          singleBuff = M_E;
          typeSet = LEX_TYPE_VALUE;
        }
        valBuff.value.single = singleBuff;
        valBuff.type = typeSet;
        if (!DblListAppend(*expression, &valBuff)) {
          localResult = CRESULT_ERROR_MEMORY_LACK;
        }
        i += (int)strlen(lexList[k].str) - 1;
      }
    }
  }
  if (localResult != CRESULT_OK) {
    DblListFree(expression);
    VarListFree(varList);
  }
  return localResult;
}