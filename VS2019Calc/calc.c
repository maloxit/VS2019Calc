// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "calc.h"
#define LEX_LIST_LEN 25
#define CALL_STACK_LEN 10
#define M_PI 3.1415926535897932384626
#define M_E  2.71828182845904523536028747
#define COMP_EPSILON 1e-11

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

static calcResult_t resultList[] = {
  {FALSE, "Not an error"},
  {TRUE, "Invalid value format"},
  {TRUE, "Invalid symbol combination"},
  {TRUE, "Missing bracket"},
  {TRUE, "Not enough memory"},
  {TRUE, "Invalid expression"},
  {TRUE, "Invalid argument or huge value"},
  {TRUE, "Unclassified error"}
};

Bool ResultCodeIsError(resultCode_t code) {
  return (code != CRESULT_OK);
}

void CalcResultPrint(calcResult_t result) {
  printf("%s", result.text);
}


Bool MyIsSpace(char ch) {
  return (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\v' || ch == '\f' || ch == '\r');
}

Bool MyIsDigit(char ch) {
  return (ch >= '0' && ch <= '9');
}

Bool MyIsAlpha(char ch) {
  return ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'));
}


typedef struct couple_t {
  double v1;
  double v2;
} couple_t;

typedef union value_t {
  couple_t couple;
  unsigned int varIndex;
  double single;
} value_t;

typedef struct _node_t {
  lexType_t type;
  value_t value;
  struct _node_t* previous;
  struct _node_t* next;
} node_t;

typedef struct dblList_t {
  node_t* head;
  node_t* end;
} dblList_t;

static dblList_t* DblListCreate(void) {
  dblList_t* list = (dblList_t*)malloc(sizeof(dblList_t));
  if (list != NULL) {
    list->head = NULL;
    list->end = NULL;
  }
  return list;
}

static void DblListFree(dblList_t** list) {
  node_t* item = (*list)->head, * nextI = NULL;
  while (item) {
    nextI = item->next;
    free(item);
    item = nextI;
  }
  free(*list);
  *list = NULL;
}

static node_t* DblListAppend(dblList_t* list, const value_t * value, lexType_t type) {
  node_t* item = (node_t*)malloc(sizeof(node_t));
  if (!item) {
    return NULL;
  }
  item->value = *value;
  item->type = type;
  if (list->head == NULL) {
    item->next = NULL;
    item->previous = NULL;
    list->head = item;
    list->end = item;
  }
  else {
    list->end->next = item;
    item->previous = list->end;
    item->next = NULL;
    list->end = item;
  }
  return item;
}

static resultCode_t DblListLeftSplit(dblList_t** leftList, dblList_t** mainList, node_t* splitPoint) {
  if ((*mainList)->head == splitPoint || (*mainList)->end == splitPoint) {
    return CRESULT_ERROR_INVALID_EXPR;
  }
  (*leftList) = DblListCreate();
  if (!(*leftList)) {
    return CRESULT_ERROR_MEMORY_LACK;
  }
  (*leftList)->head = (*mainList)->head;
  (*leftList)->end = splitPoint->previous;
  (*leftList)->end->next = NULL;
  (*mainList)->head = splitPoint->next;
  (*mainList)->head->previous = NULL;
  free(splitPoint);
  return CRESULT_OK;
}

static resultCode_t DblListExtructSubList(dblList_t* list, node_t* start, node_t* close, dblList_t** subList) {
  *subList = DblListCreate();
  if (!(*subList)) {
    return CRESULT_ERROR_MEMORY_LACK;
  }
  if (list->end == close) {
    list->end = start->previous;
  }
  (*subList)->head = start;
  (*subList)->end = close->previous;
  start->previous->next = close->next;
  if (close->next) {
    close->next->previous = start->previous;
  }
  (*subList)->head->previous = NULL;
  (*subList)->end->next = NULL;
  free(close);
  return CRESULT_OK;
}

static void DblListShrinkSubList(dblList_t* list, node_t* start, node_t* end, const node_t* insert) {
  node_t* item, * next;
  if (end->next) {
    end->next->previous = start;
  }
  else {
    list->end = start;
  }
  start->next = end->next;
  item = end;
  while (item != start) {
    next = item->previous;
    free(item);
    item = next;
  }
  start->type = insert->type;
  start->value = insert->value;
}

static node_t* NodeGetNext(node_t* item) {
  return item->next;
}

static node_t* NodeGetPrevious(node_t* item) {
  return item->previous;
}


#define VAR_LIST_BUFF_SIZE 10

typedef struct var_t {
  double val;
  Bool isCalculated;
  char ch;
}var_t;

typedef struct varList_t {
  var_t* at;
  int len;
  int size;
}varList_t;

static varList_t* localVarList;

static varList_t* VarListGet(void) {
  varList_t* varList;
  varList = (varList_t*)malloc(sizeof(varList_t));
  if (!varList) {
    return NULL;
  }
  varList->at = (var_t*)malloc(sizeof(var_t) * VAR_LIST_BUFF_SIZE);
  if (!varList->at) {
    free(varList);
    return NULL;
  }
  varList->size = VAR_LIST_BUFF_SIZE;
  varList->len = 0;
  return varList;
}

static void VarListFree(varList_t** varList) {
  free((*varList)->at);
  free(*varList);
  *varList = NULL;
}

static int VarListGetIndexByChar(varList_t* varList, char ch) {
  int i;
  for (i = 0; i < varList->len; i++) {
    if (varList->at[i].ch == ch) {
      break;
    }
  }
  if (i == varList->len) {
    return -1;
  }
  else {
    return i;
  }
}

static int VarListAppend(varList_t* varList, char ch) {
  var_t* memTry;
  if (varList->len == varList->size) {
    memTry = (var_t*)realloc(varList->at, sizeof(var_t) * (varList->size + VAR_LIST_BUFF_SIZE));
    if (!memTry) {
      return -1;
    }
    else {
      varList->size += VAR_LIST_BUFF_SIZE;
      varList->at = memTry;
    }
  }
  varList->at[varList->len].ch = ch;
  varList->at[varList->len].isCalculated = FALSE;
  return (varList->len)++;
}


typedef double UnarOp_t(double arg1);

static double MyTan(double arg1) {
  if (fabs(cos(arg1)) <= COMP_EPSILON) {
    errno = EDOM;
    return 0;
  }
  return tan(arg1);
}

static double MyCtan(double arg1) {
  return MyTan(M_PI / 2 - arg1);
}

typedef double BinaryOp_t(double arg1, double arg2);

static double Divide(double arg1, double arg2) {
  if (fabs(arg2) <= COMP_EPSILON) {
    errno = EDOM;
    return 0;
  }
  return arg1 / arg2;
}

static double Multiply(double arg1, double arg2) {
  return arg1 * arg2;
}

static double Plus(double arg1, double arg2) {
  return arg1 + arg2;
}

static double BinMinus(double arg1, double arg2) {
  return arg1 - arg2;
}

static resultCode_t BinaryProcess(BinaryOp_t* operation, node_t* lex, node_t** start, node_t** end, node_t* insert) {
  if (!(lex->previous && lex->next)) {
    return CRESULT_ERROR_INVALID_EXPR;
  }
  if (!(lex->previous->type == LEX_TYPE_VALUE && lex->next->type == LEX_TYPE_VALUE)) {
    return CRESULT_ERROR_INVALID_EXPR;
  }
  *start = lex->previous;
  *end = lex->next;
  insert->type = LEX_TYPE_VALUE;
  errno = 0;
  insert->value.single = operation(lex->previous->value.single, lex->next->value.single);
  if (errno != 0) {
    errno = 0;
    return CRESULT_ERROR_INVALID_ARG_OR_HUGEVAL;
  }
  return CRESULT_OK;
}

static resultCode_t UnarProcess(UnarOp_t* operation, node_t* lex, node_t** start, node_t** end, node_t* insert) {
  if (!(lex->next && lex->next->type == LEX_TYPE_VALUE)) {
    return CRESULT_ERROR_INVALID_EXPR;
  }
  *start = lex;
  *end = lex->next;
  insert->type = LEX_TYPE_VALUE;
  errno = 0;
  insert->value.single = operation(lex->next->value.single);
  if (errno != 0) {
    errno = 0;
    return CRESULT_ERROR_INVALID_ARG_OR_HUGEVAL;
  }
  return CRESULT_OK;
}


typedef resultCode_t LemCalc_t(node_t* lex, node_t** start, node_t** end, node_t* insert);

static resultCode_t SqrtProcess(node_t* lex, node_t** start, node_t** end, node_t* insert) {
  return UnarProcess(sqrt, lex, start, end, insert);
}

static resultCode_t SinProcess(node_t* lex, node_t** start, node_t** end, node_t* insert) {
  return UnarProcess(sin, lex, start, end, insert);
}

static resultCode_t CosProcess(node_t* lex, node_t** start, node_t** end, node_t* insert) {
  return UnarProcess(cos, lex, start, end, insert);
}

static resultCode_t TgProcess(node_t* lex, node_t** start, node_t** end, node_t* insert) {
  return UnarProcess(MyTan, lex, start, end, insert);
}

static resultCode_t CtgProcess(node_t* lex, node_t** start, node_t** end, node_t* insert) {
  return UnarProcess(MyCtan, lex, start, end, insert);
}

static resultCode_t ArcsinProcess(node_t* lex, node_t** start, node_t** end, node_t* insert) {
  return UnarProcess(asin, lex, start, end, insert);
}

static resultCode_t ArccosProcess(node_t* lex, node_t** start, node_t** end, node_t* insert) {
  return UnarProcess(acos, lex, start, end, insert);
}

static resultCode_t ArctgProcess(node_t* lex, node_t** start, node_t** end, node_t* insert) {
  return UnarProcess(atan, lex, start, end, insert);
}

static resultCode_t LnProcess(node_t* lex, node_t** start, node_t** end, node_t* insert) {
  return UnarProcess(log, lex, start, end, insert);
}

static resultCode_t LogProcess(node_t* lex, node_t** start, node_t** end, node_t* insert) {
  double buff;
  if (!(lex->next && lex->next->type == LEX_TYPE_COUPLE)) {
    return CRESULT_ERROR_INVALID_EXPR;
  }
  *start = lex;
  *end = lex->next;
  insert->type = LEX_TYPE_VALUE;
  errno = 0;
  buff = log(lex->next->value.couple.v1);
  if (errno != 0 || fabs(buff) <= COMP_EPSILON) {
    errno = 0;
    return CRESULT_ERROR_INVALID_ARG_OR_HUGEVAL;
  }
  insert->value.single = log(lex->next->value.couple.v2);
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

static resultCode_t FloorProcess(node_t* lex, node_t** start, node_t** end, node_t* insert) {
  return UnarProcess(floor, lex, start, end, insert);
}

static resultCode_t CeilProcess(node_t* lex, node_t** start, node_t** end, node_t* insert) {
  return UnarProcess(ceil, lex, start, end, insert);
}

static resultCode_t PowProcess(node_t* lex, node_t** start, node_t** end, node_t* insert) {
  return BinaryProcess(pow, lex, start, end, insert);
}

static resultCode_t MultiplyProcess(node_t* lex, node_t** start, node_t** end, node_t* insert) {
  return BinaryProcess(Multiply, lex, start, end, insert);
}

static resultCode_t DivideProcess(node_t* lex, node_t** start, node_t** end, node_t* insert) {
  return BinaryProcess(Divide, lex, start, end, insert);
}

static resultCode_t MinusProcess(node_t* lex, node_t** start, node_t** end, node_t* insert) {
  node_t* item = lex;
  int i = 1;
  if (!(lex->next && lex->next->type == LEX_TYPE_VALUE)) {
    return CRESULT_ERROR_INVALID_EXPR;
  }
  while (item->previous && item->type == LEX_TYPE_MINUS) {
    item = item->previous;
    i++;
  }
  if (item->type != LEX_TYPE_VALUE) {
    if (item->type == LEX_TYPE_MINUS) {
      *start = item;
      i++;
    }
    else {
      *start = item->next;
    }
    if (lex->next->next && lex->next->next->type == LEX_TYPE_POW) {
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
        insert->value.single = -lex->next->value.single;
      }
      else {
        insert->value.single = lex->next->value.single;
      }
    }
  }
  else {
    *end = lex;
    *start = item->next;
    
    insert->type = LEX_TYPE_PLUS;
    if (i % 2 == 0) {
      if (lex->next->next && lex->next->next->type == LEX_TYPE_POW) {
        insert->type = LEX_TYPE_MINUS;
      }
      else {
        lex->next->value.single *= -1;
      }
    }
  }
  return CRESULT_OK;
}

static resultCode_t PlusProcess(node_t* lex, node_t** start, node_t** end, node_t* insert) {
  if (!(lex->previous && lex->next && lex->previous->type == LEX_TYPE_VALUE)) {
    return CRESULT_ERROR_INVALID_EXPR;
  }
  if (lex->next->type == LEX_TYPE_VALUE) {
    return BinaryProcess(Plus, lex, start, end, insert);
  }
  else {
    return CRESULT_ERROR_INVALID_EXPR;
  }
}

static resultCode_t CommaProcess(node_t* lex, node_t** start, node_t** end, node_t* insert) {
  if (!(lex->previous && lex->next)) {
    return CRESULT_ERROR_INVALID_EXPR;
  }
  if (!(lex->previous->type == LEX_TYPE_VALUE && lex->next->type == LEX_TYPE_VALUE)) {
    return CRESULT_ERROR_INVALID_EXPR;
  }
  *start = lex->previous;
  *end = lex->next;
  insert->type = LEX_TYPE_COUPLE;
  insert->value.couple.v1 = lex->previous->value.single;
  insert->value.couple.v2 = lex->next->value.single;

  return CRESULT_OK;

}

static resultCode_t EqualProcess(node_t* lex, node_t** start, node_t** end, node_t* insert) {
  if (!(lex->previous && lex->next)) {
    return CRESULT_ERROR_INVALID_EXPR;
  }
  if (!(lex->previous->type == LEX_TYPE_VAR && lex->next->type == LEX_TYPE_VALUE)) {
    return CRESULT_ERROR_INVALID_EXPR;
  }
  *start = lex->previous;
  *end = lex->next;
  insert->type = LEX_TYPE_VALUE;
  insert->value.single = lex->next->value.single;
  localVarList->at[lex->previous->value.varIndex].val = lex->next->value.single;
  localVarList->at[lex->previous->value.varIndex].isCalculated = TRUE;

  return CRESULT_OK;

}

static resultCode_t VarProcess(node_t* lex, node_t** start, node_t** end, node_t* insert) {
  *start = lex;
  *end = lex;
  if (localVarList->at[lex->value.varIndex].isCalculated) {
    insert->type = LEX_TYPE_VALUE;
    insert->value.single = localVarList->at[lex->value.varIndex].val;
  }
  else {
    insert->type = LEX_TYPE_VAR;
    insert->value = lex->value;
  }
  return CRESULT_OK;
}


typedef struct lex_t {
  char str[7];
  LemCalc_t* CalcFun;
} lex_t;

static lex_t lexList[LEX_LIST_LEN] = {
  {"(", NULL},
  {")", NULL},
  {"sqrt", SqrtProcess},/*{<- + unar minus*/
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
  {"^", PowProcess},/*}*/
  {"*", MultiplyProcess},/*{->*/
  {"/", DivideProcess},/*}*/
  {"-", MinusProcess},/*{->*/
  {"+", PlusProcess},/*}*/
  {",", CommaProcess}, /*{->}*/
  {"=", EqualProcess},
  {";", NULL},
  {"pi", NULL},
  {"e", NULL},
  {"\n", VarProcess}
};

#define PRIORITY_GROUPS_COUNT 7

typedef Bool isMember_t(node_t * lex);

static Bool IsGoup0Member(node_t* lex) {
  return (lex->type == LEX_TYPE_VAR);
}

static Bool IsGoup1Member(node_t* lex) {
  return ((lex->type >= LEX_TYPE_SQRT && lex->type <= LEX_TYPE_CEIL) || lex->type == LEX_TYPE_MINUS);
}

static Bool IsGoup2Member(node_t* lex) {
  return (lex->type == LEX_TYPE_POW || lex->type == LEX_TYPE_MINUS);
}

static Bool IsGoup3Member(node_t* lex) {
  return (lex->type >= LEX_TYPE_MULTIPLY && lex->type <= LEX_TYPE_DIVIDE);
}

static Bool IsGoup4Member(node_t* lex) {
  return (lex->type >= LEX_TYPE_MINUS && lex->type <= LEX_TYPE_PLUS);
}

static Bool IsGoup5Member(node_t* lex) {
  return (lex->type == LEX_TYPE_COMMA);
}

static Bool IsGoup6Member(node_t* lex) {
  return (lex->type == LEX_TYPE_EQUAL);
}


static resultCode_t SubExprCalc(dblList_t** expr, node_t* outItem) {
  static struct priorityGroups {
    isMember_t* IsMember;
    Bool isFromLeft;
  } priorityGroups[] = {
    {IsGoup0Member, TRUE},
    {IsGoup1Member, FALSE},
    {IsGoup2Member, FALSE},
    {IsGoup3Member, TRUE},
    {IsGoup4Member, TRUE},
    {IsGoup5Member, TRUE},
    {IsGoup6Member, FALSE}
  };
  int i;
  node_t* item;
  node_t* start;
  node_t* end;
  node_t insert;
  resultCode_t localResult = CRESULT_OK;

  node_t* (*GetAnother) (node_t*);
  for (i = 0; i < PRIORITY_GROUPS_COUNT; i++) {
    if (priorityGroups[i].isFromLeft) {
      GetAnother = NodeGetNext;
      item = (*expr)->head;
    }
    else {
      GetAnother = NodeGetPrevious;
      item = (*expr)->end;
    }
    for (; item; item = GetAnother(item)) {
      if (priorityGroups[i].IsMember(item)) {
        localResult = lexList[item->type].CalcFun(item, &start, &end, &insert);
        if (ResultCodeIsError(localResult)) {
          break;
        }
        DblListShrinkSubList(*expr, start, end, &insert);
        item = start;
      }
    }
    if (ResultCodeIsError(localResult)) {
      break;
    }
  }
  if (!ResultCodeIsError(localResult)) {
    if ((*expr)->head && (*expr)->head == (*expr)->end) {
      outItem->type = (*expr)->head->type;
      outItem->value = (*expr)->head->value;
    }
    else {
      localResult = CRESULT_ERROR_INVALID_EXPR;
    }
  }
  DblListFree(expr);
  return localResult;
}

static resultCode_t ExprCalc(dblList_t** expr, double* ans) {
  typedef struct calcState {
    dblList_t* subExpr;
    node_t* item;
    node_t* outItem;
  }calcState;
  dblList_t* subExpr;
  node_t* item;
  node_t* open;
  node_t* close;
  node_t finalOut;
  resultCode_t localResult = CRESULT_OK;
  int localDepth;
  int depth = 0;
  int stackLen = CALL_STACK_LEN;
  calcState* subExprsCall;
  calcState* memTry = (calcState*)malloc(sizeof(calcState) * stackLen);
  if (!memTry) {
    DblListFree(expr);
    return CRESULT_ERROR_MEMORY_LACK;
  }
  subExprsCall = memTry;
  subExprsCall[depth].subExpr = *expr;
  subExprsCall[depth].item = (*expr)->head;
  subExprsCall[depth].outItem = &finalOut;
  while (depth >= 0 && !ResultCodeIsError(localResult)) {
    open = NULL;
    close = NULL;
    for (item = subExprsCall[depth].item; item && !ResultCodeIsError(localResult); item = item->next) {
      if (item->type == LEX_TYPE_OPEN) {
        open = item;
        close = open->next;
        localDepth = 1;
        while (close) {
          if (close->type == LEX_TYPE_CLOSE) {
            localDepth--;
            if (localDepth == 0) {
              break;
            }
          }
          else {
            if (close->type == LEX_TYPE_OPEN) {
              localDepth++;
            }
          }
          close = close->next;
        }
        if (!close || close == open->next) {
          localResult = CRESULT_ERROR_BRACKET;
        }
        else {
          break;
        }
      }
      else {
        if (item->type == LEX_TYPE_CLOSE) {
          localResult = CRESULT_ERROR_BRACKET;
        }
      }
    }
    if (ResultCodeIsError(localResult)) {
      break;
    }
    if (open && close) {
      subExprsCall[depth].item = close->next;
      localResult = DblListExtructSubList(subExprsCall[depth].subExpr, open->next, close, &subExpr);
      if (!ResultCodeIsError(localResult)) {
        depth++;
        if (depth < stackLen) {
          subExprsCall[depth].subExpr = subExpr;
          subExprsCall[depth].item = subExpr->head;
          subExprsCall[depth].outItem = open;
        }
        else {
          stackLen += CALL_STACK_LEN;
          memTry = (calcState*)realloc(subExprsCall, sizeof(calcState) * stackLen);
          if (!memTry) {
            depth--;
            DblListFree(&subExpr);
            localResult = CRESULT_ERROR_MEMORY_LACK;
          }
          else {
            subExprsCall = memTry;
            subExprsCall[depth].subExpr = subExpr;
            subExprsCall[depth].item = subExpr->head;
            subExprsCall[depth].outItem = open;
          }
        }
      }
    }
    else {
      localResult = SubExprCalc(&(subExprsCall[depth].subExpr), subExprsCall[depth].outItem);
      depth--;
    }
  }
  if (ResultCodeIsError(localResult)) {
    for (; depth >= 0; depth--) {
      DblListFree(&(subExprsCall[depth].subExpr));
    }
    free(subExprsCall);
    *expr = NULL;
    return localResult;
  }
  else {
    if (finalOut.type == LEX_TYPE_VALUE) {
      *ans = finalOut.value.single;
      free(subExprsCall);
      *expr = NULL;
      return localResult;
    }
    else {
      free(subExprsCall);
    }
    *expr = NULL;
    return CRESULT_ERROR;
  }
}

static resultCode_t ExprSequenceCalc(dblList_t** exprSequence, double* ans) {
  node_t* item = (*exprSequence)->head;
  dblList_t* expr = NULL;
  resultCode_t localResult = CRESULT_OK;
  double localAns = 0;
  while (item && !ResultCodeIsError(localResult)) {
    if (item->type == LEX_TYPE_SEMICOLON) {
      localResult = DblListLeftSplit(&expr, exprSequence, item);
      if (ResultCodeIsError(localResult)) {
        break;
      }
      else {
        item = (*exprSequence)->head;
        localResult = ExprCalc(&expr, &localAns);
      }
    }
    else {
      item = item->next;
    }
  }
  if (!item) {
    localResult = ExprCalc(exprSequence, &localAns);
  }
  if ((*exprSequence)) {
    DblListFree(exprSequence);
  }
  if (expr) {
    DblListFree(&expr);
  }
  *ans = localAns;
  return localResult;

}

static resultCode_t LexSplit(const char* const str, dblList_t** expression) {
  int i, k, typeSet, varIndex;
  char* endPtr;
  double singleBuff;
  value_t valBuff;
  localVarList = VarListGet();
  if (!localVarList) {
    return CRESULT_ERROR_MEMORY_LACK;
  }
  *expression = DblListCreate();
  if (!(*expression)) {
    return CRESULT_ERROR_MEMORY_LACK;
  }
  for (i = 0; str[i] != '\0'; i++) {
    if (MyIsSpace(str[i])) {
      continue;
    }
    if (MyIsDigit(str[i])) {
      singleBuff = strtod(str + i, &endPtr);
      if (endPtr == str + i) {
        DblListFree(expression);
        VarListFree(&localVarList);
        return CRESULT_ERROR_VALUE_FORMAT;
      }
      else {
        valBuff.single = singleBuff;
        if (!DblListAppend(*expression, &valBuff, LEX_TYPE_VALUE)) {
          DblListFree(expression);
          VarListFree(&localVarList);
          return CRESULT_ERROR_MEMORY_LACK;
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
          varIndex = VarListGetIndexByChar(localVarList, str[i]);
          if (varIndex < 0) {
            varIndex = VarListAppend(localVarList, str[i]);
          }
          if (varIndex < 0) {
            DblListFree(expression);
            VarListFree(&localVarList);
            return CRESULT_ERROR_UNKNOWN_LEX;
          }
          else {
            valBuff.varIndex = varIndex;
            if (!DblListAppend(*expression, &valBuff, LEX_TYPE_VAR)) {
              DblListFree(expression);
              VarListFree(&localVarList);
              return CRESULT_ERROR_MEMORY_LACK;
            }
          }

        }
        else {
          DblListFree(expression);
          VarListFree(&localVarList);
          return CRESULT_ERROR_UNKNOWN_LEX;
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
        valBuff.single = singleBuff;
        if (!DblListAppend(*expression, &valBuff, typeSet)) {
          DblListFree(expression);
          VarListFree(&localVarList);
          return CRESULT_ERROR_MEMORY_LACK;
        }
        i += (int)strlen(lexList[k].str) - 1;
      }
    }
  }
  return CRESULT_OK;
}

calcResult_t StringCalc(const char* const str, double* ans) {
  dblList_t* expr;
  resultCode_t result;
  result = LexSplit(str, &expr);
  if (ResultCodeIsError(result)) {
    return resultList[result];
  }
  result = ExprSequenceCalc(&expr, ans);
  VarListFree(&localVarList);
  return resultList[result];
}