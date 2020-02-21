// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "calc.h"
#define LEM_LIST_LEN 25
#define CALL_STACK_LEN 10
#define M_PI 3.1415926535897932384626
#define M_E  2.71828182845904523536028747
#define COMP_EPSILON 1e-15

typedef enum lemType_t {
  LEM_TYPE_VALUE = -1,
  LEM_TYPE_COUPLE = -2,

  LEM_TYPE_OPEN = 0,
  LEM_TYPE_CLOSE,
  LEM_TYPE_SQRT,
  LEM_TYPE_SIN,
  LEM_TYPE_COS,
  LEM_TYPE_TG,
  LEM_TYPE_CTG,
  LEM_TYPE_ARCSIN,
  LEM_TYPE_ARCCOS,
  LEM_TYPE_ARCTG,
  LEM_TYPE_LN,
  LEM_TYPE_LOG,
  LEM_TYPE_FLOOR,
  LEM_TYPE_CEIL,
  LEM_TYPE_POW,
  LEM_TYPE_MULTIPLY,
  LEM_TYPE_DIVIDE,
  LEM_TYPE_MINUS,
  LEM_TYPE_PLUS,
  LEM_TYPE_COMMA,
  LEM_TYPE_EQUAL,
  LEM_TYPE_SEMICOLON,
  LEM_TYPE_PI,
  LEM_TYPE_E,
  LEM_TYPE_VAR
} lemType_t;

typedef enum resultCode_t {
  CRESULT_OK = 0,
  CRESULT_ERROR_VALUE_FORMAT,
  CRESULT_ERROR_UNKNOWN_LEM,
  CRESULT_ERROR_BRACKET,
  CRESULT_ERROR_MEMORY_LACK,
  CRESULT_ERROR_INVALID_EXPR,
  CRESULT_ERROR_INVALID_ARG_OR_HUGEVAL,
  CRESULT_ERROR
} resultCode_t;

static calcResult_t errorList[] = {
  {0, "Not an error"},
  {1, "Invalid value format"},
  {1, "Invalid symbol combination"},
  {1, "Missing bracket"},
  {1, "Not enough memory"},
  {1, "Invalid expression"},
  {1, "Invalid argument or huge value"},
  {1, "Unclassified error"}
};
int IsError(resultCode_t code) {
  return (code != CRESULT_OK);
}
void CalcPrintError(calcResult_t err) {
  printf("%s", err.text);
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
  lemType_t type;
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
static node_t* DblListAppend(dblList_t* list, const value_t * value, lemType_t type) {
  node_t* item = (node_t*)malloc(sizeof(node_t));
  if (!item) {
    return NULL;
  }
  item->value = *value;
  item->type = type;
  if (list->head == NULL) {
    item->next = item->previous = NULL;
    list->head = list->end = item;
  }
  else {
    list->end->next = item;
    item->previous = list->end;
    item->next = NULL;
    list->end = item;
  }
  return item;
}

#define VAR_LIST_BUFF_SIZE 10
typedef struct var_t {
  double val;
  int isCalculated;
  char ch;
}var_t;

typedef struct varList_t {
  var_t* at;
  int len;
  int size;
}varList_t;

static varList_t* localVarList;

static varList_t* GetVarList(void) {
  varList_t* varList;
  varList = (varList_t*)malloc(sizeof(varList_t));
  if (!varList)
    return NULL;
  varList->at = (var_t*)malloc(sizeof(var_t) * VAR_LIST_BUFF_SIZE);
  if (!varList->at) {
    free(varList);
    return NULL;
  }
  varList->size = VAR_LIST_BUFF_SIZE;
  varList->len = 0;
  return varList;
}
static void FreeVarList(varList_t** varList) {
  free((*varList)->at);
  free(*varList);
  *varList = NULL;
}
static int GetCharIndexInVarList(varList_t* varList, char ch) {
  int i;
  for (i = 0; i < varList->len; i++)
    if (varList->at[i].ch == ch)
      break;
  if (i == varList->len)
    return -1;
  else
    return i;
}

static int AppendVar(varList_t* varList, char ch) {
  var_t* memTry;
  if (varList->len == varList->size) {
    memTry = (var_t*)realloc(varList->at, sizeof(var_t) * (varList->size + VAR_LIST_BUFF_SIZE));
    if (!memTry)
      return -1;
    else {
      varList->size += VAR_LIST_BUFF_SIZE;
      varList->at = memTry;
    }
  }
  varList->at[varList->len].ch = ch;
  varList->at[varList->len].isCalculated = 0;
  return (varList->len)++;
}
typedef double UnarOp_t(double arg1);

static double ctan(double arg1) {
  return tan(M_PI / 2 - arg1);
}
static double UnarMinus(double arg1) {
  return -arg1;
}
typedef double BinaryOp_t(double arg1, double arg2);
static double Divide(double arg1, double arg2) {
  if (fabs(arg2) < COMP_EPSILON) {
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

static resultCode_t BinaryProcess(BinaryOp_t* operation, node_t* lem, node_t** start, node_t** end, node_t* insert) {
  if (!(lem->previous && lem->next))
    return CRESULT_ERROR_INVALID_EXPR;
  if (!(lem->previous->type == LEM_TYPE_VALUE && lem->next->type == LEM_TYPE_VALUE))
    return CRESULT_ERROR_INVALID_EXPR;
  *start = lem->previous;
  *end = lem->next;
  insert->type = LEM_TYPE_VALUE;
  errno = 0;
  insert->value.single = operation(lem->previous->value.single, lem->next->value.single);
  if (errno != 0) {
    errno = 0;
    return CRESULT_ERROR_INVALID_ARG_OR_HUGEVAL;
  }
  return CRESULT_OK;
}
static resultCode_t UnarProcess(UnarOp_t* operation, node_t* lem, node_t** start, node_t** end, node_t* insert) {
  if (!(lem->next && lem->next->type == LEM_TYPE_VALUE))
    return CRESULT_ERROR_INVALID_EXPR;
  *start = lem;
  *end = lem->next;
  insert->type = LEM_TYPE_VALUE;
  errno = 0;
  insert->value.single = operation(lem->next->value.single);
  if (errno != 0) {
    errno = 0;
    return CRESULT_ERROR_INVALID_ARG_OR_HUGEVAL;
  }
  return CRESULT_OK;
}


typedef resultCode_t LemCalc_t(node_t* lem, node_t** start, node_t** end, node_t* insert);
static resultCode_t SqrtProcess(node_t* lem, node_t** start, node_t** end, node_t* insert) {
  return UnarProcess(sqrt, lem, start, end, insert);
}
static resultCode_t SinProcess(node_t* lem, node_t** start, node_t** end, node_t* insert) {
  return UnarProcess(sin, lem, start, end, insert);
}
static resultCode_t CosProcess(node_t* lem, node_t** start, node_t** end, node_t* insert) {
  return UnarProcess(cos, lem, start, end, insert);
}
static resultCode_t TgProcess(node_t* lem, node_t** start, node_t** end, node_t* insert) {
  return UnarProcess(tan, lem, start, end, insert);
}
static resultCode_t CtgProcess(node_t* lem, node_t** start, node_t** end, node_t* insert) {
  return UnarProcess(ctan, lem, start, end, insert);
}
static resultCode_t ArcsinProcess(node_t* lem, node_t** start, node_t** end, node_t* insert) {
  return UnarProcess(asin, lem, start, end, insert);
}
static resultCode_t ArccosProcess(node_t* lem, node_t** start, node_t** end, node_t* insert) {
  return UnarProcess(acos, lem, start, end, insert);
}
static resultCode_t ArctgProcess(node_t* lem, node_t** start, node_t** end, node_t* insert) {
  return UnarProcess(atan, lem, start, end, insert);
}
static resultCode_t LnProcess(node_t* lem, node_t** start, node_t** end, node_t* insert) {
  return UnarProcess(log, lem, start, end, insert);
}
static resultCode_t LogProcess(node_t* lem, node_t** start, node_t** end, node_t* insert) {
  double buff;
  if (!(lem->next && lem->next->type == LEM_TYPE_COUPLE))
    return CRESULT_ERROR_INVALID_EXPR;
  *start = lem;
  *end = lem->next;
  insert->type = LEM_TYPE_VALUE;
  errno = 0;
  buff = log(lem->next->value.couple.v2);
  if (errno != 0 || fabs(buff) < COMP_EPSILON) {
    errno = 0;
    return CRESULT_ERROR_INVALID_ARG_OR_HUGEVAL;
  }
  insert->value.couple.v1 = log(lem->next->value.couple.v1);
  if (errno != 0) {
    errno = 0;
    return CRESULT_ERROR_INVALID_ARG_OR_HUGEVAL;
  }
  insert->value.couple.v1 /= buff;
  if (errno != 0) {
    errno = 0;
    return CRESULT_ERROR_INVALID_ARG_OR_HUGEVAL;
  }
  return CRESULT_OK;
}
static resultCode_t FloorProcess(node_t* lem, node_t** start, node_t** end, node_t* insert) {
  return UnarProcess(floor, lem, start, end, insert);
}
static resultCode_t CeilProcess(node_t* lem, node_t** start, node_t** end, node_t* insert) {
  return UnarProcess(ceil, lem, start, end, insert);
}
static resultCode_t PowProcess(node_t* lem, node_t** start, node_t** end, node_t* insert) {
  return BinaryProcess(pow, lem, start, end, insert);
}
static resultCode_t MultiplyProcess(node_t* lem, node_t** start, node_t** end, node_t* insert) {
  return BinaryProcess(Multiply, lem, start, end, insert);
}
static resultCode_t DivideProcess(node_t* lem, node_t** start, node_t** end, node_t* insert) {
  return BinaryProcess(Divide, lem, start, end, insert);
}
static resultCode_t MinusProcess(node_t* lem, node_t** start, node_t** end, node_t* insert) {
  node_t* item = lem;
  int i = 1;
  if (!(lem->next && lem->next->type == LEM_TYPE_VALUE)) {
    return CRESULT_ERROR_INVALID_EXPR;
  }
  while (item->previous && item->type == LEM_TYPE_MINUS) {
    item = item->previous;
    i++;
  }
  if (item->type != LEM_TYPE_VALUE) {
    
    if (item->type == LEM_TYPE_MINUS) {
      *start = item;
      i++;
    }
    else {
      *start = item->next;
    }
    if (lem->next->next && lem->next->next->type == LEM_TYPE_POW) {
      *end = lem;
      if (i % 2 == 0)
        insert->type = LEM_TYPE_MINUS;
      else
        insert->type = LEM_TYPE_PLUS;
    }
    else {
      *end = lem->next;
      insert->type = LEM_TYPE_VALUE;
      if (i % 2 == 0)
        insert->value.single = -lem->next->value.single;
      else
        insert->value.single = lem->next->value.single;
    }
  }
  else {
    *end = lem;
    *start = item->next;
    
    insert->type = LEM_TYPE_PLUS;
    if (i % 2 == 0){
      if (lem->next->next && lem->next->next->type == LEM_TYPE_POW) {
        insert->type = LEM_TYPE_MINUS;
      }
      else
        lem->next->value.single *= -1;
    }
  }
  return CRESULT_OK;
}
static resultCode_t PlusProcess(node_t* lem, node_t** start, node_t** end, node_t* insert) {
  if (!(lem->previous && lem->next && lem->previous->type == LEM_TYPE_VALUE))
    return CRESULT_ERROR_INVALID_EXPR;
  if (lem->next->type == LEM_TYPE_VALUE) {
    return BinaryProcess(Plus, lem, start, end, insert);
  }
  else
    return CRESULT_ERROR_INVALID_EXPR;
}
static resultCode_t CommaProcess(node_t* lem, node_t** start, node_t** end, node_t* insert) {
  if (!(lem->previous && lem->next))
    return CRESULT_ERROR_INVALID_EXPR;
  if (!(lem->previous->type == LEM_TYPE_VALUE && lem->next->type == LEM_TYPE_VALUE))
    return CRESULT_ERROR_INVALID_EXPR;
  *start = lem->previous;
  *end = lem->next;
  insert->type = LEM_TYPE_COUPLE;
  insert->value.couple.v1 = lem->previous->value.single;
  insert->value.couple.v2 = lem->next->value.single;

  return CRESULT_OK;

}
static resultCode_t EqualProcess(node_t* lem, node_t** start, node_t** end, node_t* insert) {
  if (!(lem->previous && lem->next))
    return CRESULT_ERROR_INVALID_EXPR;
  if (!(lem->previous->type == LEM_TYPE_VAR && lem->next->type == LEM_TYPE_VALUE))
    return CRESULT_ERROR_INVALID_EXPR;
  *start = lem->previous;
  *end = lem->next;
  insert->type = LEM_TYPE_VALUE;
  insert->value.single = lem->next->value.single;
  localVarList->at[lem->previous->value.varIndex].val = lem->next->value.single;
  localVarList->at[lem->previous->value.varIndex].isCalculated = 1;

  return CRESULT_OK;

}
static resultCode_t VarProcess(node_t* lem, node_t** start, node_t** end, node_t* insert) {
  *start = *end = lem;
  if (localVarList->at[lem->value.varIndex].isCalculated) {
    insert->type = LEM_TYPE_VALUE;
    insert->value.single = localVarList->at[lem->value.varIndex].val;
  }
  else {
    insert->type = LEM_TYPE_VAR;
    insert->value = lem->value;
  }
  return CRESULT_OK;
}

typedef struct lem_t {
  char str[7];
  LemCalc_t* CalcFun;
} lem_t;

static lem_t lemList[LEM_LIST_LEN] = {
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
static resultCode_t ExptLeftSplit(dblList_t** leftList, dblList_t** mainList, node_t* splitPoint) {
  if ((*mainList)->head == splitPoint || (*mainList)->end == splitPoint)
    return CRESULT_ERROR_INVALID_EXPR;
  (*leftList) = DblListCreate();
  if (!(*leftList))
    return CRESULT_ERROR_MEMORY_LACK;
  (*leftList)->head = (*mainList)->head;
  (*leftList)->end = splitPoint->previous;
  (*leftList)->end->next = NULL;
  (*mainList)->head = splitPoint->next;
  (*mainList)->head->previous = NULL;
  free(splitPoint);
  return CRESULT_OK;
}

static resultCode_t ExtructSubExpr(dblList_t* expr, node_t* start, node_t* close, dblList_t** subExpr) {
  *subExpr = DblListCreate();
  if (!(*subExpr))
    return CRESULT_ERROR_MEMORY_LACK;
  if (expr->end == close)
    expr->end = start->previous;
  (*subExpr)->head = start;
  (*subExpr)->end = close->previous;
  start->previous->next = close->next;
  if (close->next)
    close->next->previous = start->previous;
  (*subExpr)->head->previous = NULL;
  (*subExpr)->end->next = NULL;
  free(close);
  return CRESULT_OK;
}
static void ShrinkSubExpr(dblList_t* expr, node_t* start, node_t* end, const node_t * insert) {
  node_t* item, * next;
  if (end->next)
    end->next->previous = start;
  else
    expr->end = start;
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

static node_t* GetNext(node_t* item) {
  return item->next;
}
static node_t* GetPrevious(node_t* item) {
  return item->previous;
}

#define PRIORITY_GROUPS_COUNT 7

typedef int isMember_t(node_t * lem);
int isGoup0Member(node_t* lem) {
  if (lem->type == LEM_TYPE_VAR)
    return 1;
  else
    return 0;
}
int isGoup1Member(node_t* lem) {

  if ((lem->type >= LEM_TYPE_SQRT && lem->type <= LEM_TYPE_CEIL) || lem->type == LEM_TYPE_MINUS) {
    return 1;
  }
  else
    return 0;
}
int isGoup2Member(node_t* lem) {
  if (lem->type == LEM_TYPE_POW)
    return 1;
  else
    return 0;
}

int isGoup3Member(node_t* lem) {
  if (lem->type >= LEM_TYPE_MULTIPLY && lem->type <= LEM_TYPE_DIVIDE)
    return 1;
  else
    return 0;
}
int isGoup4Member(node_t* lem) {
  if (lem->type >= LEM_TYPE_MINUS && lem->type <= LEM_TYPE_PLUS)
    return 1;
  else
    return 0;
}
int isGoup5Member(node_t* lem) {
  if (lem->type == LEM_TYPE_COMMA)
    return 1;
  else
    return 0;
}
int isGoup6Member(node_t* lem) {
  if (lem->type == LEM_TYPE_EQUAL)
    return 1;
  else
    return 0;
}

static resultCode_t SubExprCalc(dblList_t** expr, node_t* outItem) {
  static struct priorityGroups {
    isMember_t* IsMember;
    int isFromLeft;
  } priorityGroups[] = {
    {isGoup0Member, 1},
    {isGoup1Member, 0},
    {isGoup2Member, 0},
    {isGoup3Member, 1},
    {isGoup4Member, 1},
    {isGoup5Member, 1},
    {isGoup6Member, 0}
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
      GetAnother = GetNext;
      item = (*expr)->head;
    }
    else {
      GetAnother = GetPrevious;
      item = (*expr)->end;
    }
    for (; item; item = GetAnother(item)) {
      if (priorityGroups[i].IsMember(item)) {
        localResult = lemList[item->type].CalcFun(item, &start, &end, &insert);
        if (IsError(localResult))
          break;
        ShrinkSubExpr(*expr, start, end, &insert);
        item = start;
      }
    }
    if (IsError(localResult))
      break;
  }
  if (!IsError(localResult)) {
    if ((*expr)->head && (*expr)->head == (*expr)->end) {
      outItem->type = (*expr)->head->type;
      outItem->value = (*expr)->head->value;
    }
    else
      localResult = CRESULT_ERROR_INVALID_EXPR;
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
  resultCode_t localResult;
  int localDepth;
  int depth = 0;
  int stackLen = CALL_STACK_LEN;
  calcState* subExprsCall;
  calcState* memTry = (calcState*)malloc(sizeof(calcState) * stackLen);
  if (!memTry) {
    return CRESULT_ERROR_MEMORY_LACK;
  }
  subExprsCall = memTry;
  subExprsCall[depth].subExpr = *expr;
  subExprsCall[depth].item = (*expr)->head;
  subExprsCall[depth].outItem = &finalOut;
  localResult = CRESULT_OK;
  while (depth >= 0 && !IsError(localResult)) {
    open = close = NULL;
    for (item = subExprsCall[depth].item; item && !IsError(localResult); item = item->next) {
      if (item->type == LEM_TYPE_OPEN) {
        open = item;
        close = open->next;
        localDepth = 1;
        while (close) {
          if (close->type == LEM_TYPE_CLOSE) {
            localDepth--;
            if (localDepth == 0)
              break;
          }
          else if (close->type == LEM_TYPE_OPEN) {
            localDepth++;
          }
          close = close->next;
        }
        if (!close || close == open->next) {
          localResult = CRESULT_ERROR_BRACKET;
        }
        else
          break;
      }
      else if (item->type == LEM_TYPE_CLOSE) {
        localResult = CRESULT_ERROR_BRACKET;
      }
    }
    if (IsError(localResult)) {
      break;
    }
    if (open && close) {
      subExprsCall[depth].item = close->next;
      localResult = ExtructSubExpr(subExprsCall[depth].subExpr, open->next, close, &subExpr);
      if (!IsError(localResult)) {
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
  if (IsError(localResult)) {
    for (; depth >= 0; depth--)
      DblListFree(&(subExprsCall[depth].subExpr));
    free(subExprsCall);
    return localResult;
  }
  else {
    if (finalOut.type == LEM_TYPE_VALUE) {
      *ans = finalOut.value.single;
      free(subExprsCall);
      return localResult;
    }
    else
      free(subExprsCall);
    return CRESULT_ERROR;
  }
}


static resultCode_t LemSplit(const char* const str, dblList_t** expression) {
  int i, k, typeSet, varIndex;
  char* endPtr;
  double singleBuff;
  value_t valBuff;
  localVarList = GetVarList();
  if (!localVarList)
    return CRESULT_ERROR_MEMORY_LACK;
  *expression = DblListCreate();
  if (!(*expression))
    return CRESULT_ERROR_MEMORY_LACK;
  for (i = 0; str[i] != '\0'; i++) {
    while (isspace(str[i]))
      i++;
    if (str[i] == '\0') {
      break;
    }
    if (isdigit(str[i])) {
      singleBuff = strtod(str + i, &endPtr);
      if (endPtr == str + i) {
        DblListFree(expression);
        FreeVarList(&localVarList);
        return CRESULT_ERROR_VALUE_FORMAT;
      }
      else {
        valBuff.single = singleBuff;
        if (!DblListAppend(*expression, &valBuff, LEM_TYPE_VALUE)) {
          DblListFree(expression);
          FreeVarList(&localVarList);
          return CRESULT_ERROR_MEMORY_LACK;
        }
        i = endPtr - str - 1;
      }
    }
    else {
      for (k = 0; k < LEM_LIST_LEN; k++) {
        if (strncmp(str + i, lemList[k].str, strlen(lemList[k].str)) == 0)
          break;
      }
      if (k == LEM_LIST_LEN) {
        if (isalpha(str[i])) {
          varIndex = GetCharIndexInVarList(localVarList, str[i]);
          if (varIndex < 0)
            varIndex = AppendVar(localVarList, str[i]);
          if (varIndex < 0) {
            DblListFree(expression);
            FreeVarList(&localVarList);
            return CRESULT_ERROR_UNKNOWN_LEM;
          }
          else {
            valBuff.varIndex = varIndex;
            if (!DblListAppend(*expression, &valBuff, LEM_TYPE_VAR)) {
              DblListFree(expression);
              FreeVarList(&localVarList);
              return CRESULT_ERROR_MEMORY_LACK;
            }
          }

        }
        else {
          DblListFree(expression);
          FreeVarList(&localVarList);
          return CRESULT_ERROR_UNKNOWN_LEM;
        }
      }
      else {
        singleBuff = 0;
        typeSet = k;
        if (typeSet == LEM_TYPE_PI) {
          singleBuff = M_PI;
          typeSet = LEM_TYPE_VALUE;
        }
        if (typeSet == LEM_TYPE_E) {
          singleBuff = M_E;
          typeSet = LEM_TYPE_VALUE;
        }
        valBuff.single = singleBuff;
        if (!DblListAppend(*expression, &valBuff, (lemType_t)typeSet)) {
          DblListFree(expression);
          FreeVarList(&localVarList);
          return CRESULT_ERROR_MEMORY_LACK;
        }
        i += strlen(lemList[k].str) - 1;
      }
    }
  }
  return CRESULT_OK;
}
static resultCode_t ExprSequenceCalc(dblList_t** exprSequence, double* ans) {
  node_t* item;
  dblList_t* expr;
  resultCode_t localResult;
  double localAns = 0;
  item = (*exprSequence)->head;
  localResult = CRESULT_OK;
  while (item && !IsError(localResult)) {
    if (item->type == LEM_TYPE_SEMICOLON) {
      localResult = ExptLeftSplit(&expr, exprSequence, item);
      if (IsError(localResult)) {
        break;
      }
      else {
        item = (*exprSequence)->head;
        localResult = ExprCalc(&expr, &localAns);
      }
    }
    else
      item = item->next;
  }
  if (!item) {
    localResult = ExprCalc(exprSequence, &localAns);
  }

  *ans = localAns;
  return localResult;

}
calcResult_t StringCalc(const char* const str, double* ans) {
  dblList_t* expr;
  resultCode_t result;
  result = LemSplit(str, &expr);
  if (IsError(result))
    return errorList[result];
  result = ExprSequenceCalc(&expr, ans);
  FreeVarList(&localVarList);
  return errorList[result];
}

