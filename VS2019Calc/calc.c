// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include <stdlib.h>
#include <stdio.h>
#include "calc.h"
#include "varlist.h"
#include "lexems.h"
#include "expression.h"
#include "resultCodes.h"

//Подробная информация о результатах вычислений, используется вне модуля
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

//Выводит в стандартный поток вывода текст результата вычисления (ошибки)
void CalcResultPrint(calcResult_t result) {
  printf("%s", result.text);
}

//Функция вычисления подвыражения (без скобок и ';')
//В случае успеха записывает ответ во внешний элемент из выражения уровнем выше
//Иначе освобождает память и возвращает код ошибки
//Вычисление производится упрощением выражения последовательными проходами по операциям из групп приоритетов
static resultCode_t SubExprCalc(dblList_t** expr, nodevalue_t* outItem) {
  int i;
  node_t* item;
  node_t* start;
  node_t* end;
  nodevalue_t insert;
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
      if (priorityGroups[i].IsMember(item->nodeVal)) {
        localResult = lexList[item->nodeVal->type].CalcFun(item, &start, &end, &insert);
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
      *(outItem) = *((*expr)->head->nodeVal);
    }
    else {
      localResult = CRESULT_ERROR_INVALID_EXPR;
    }
  }
  DblListFree(expr);
  return localResult;
}

//Начальная длина стека вызова
#define CALL_STACK_LEN 10

//Функция вычисления выражения (без ';')
//В случае успеха возвращает численный ответ
//Функция рекурсивная, но с собственным стеком вызова с динамическим выделением памяти
//Оброжит выражение в глубину по скобкам, до тех пор, пока в подвыражении их не останется, 
//затем вычисляет его значение функцией SubExprCalc и подставляет в то место, откуда было извлечено это подвыражение
//При возникновении ошибки на любом из этапов освобождает память и возвращает её код
static resultCode_t ExprCalc(dblList_t** expr, double* ans) {
  typedef struct calcState {
    dblList_t* subExpr;
    node_t* item;
    nodevalue_t* outItem;
  }calcState;
  dblList_t* subExpr;
  node_t* item;
  node_t* open;
  node_t* close;
  nodevalue_t finalOut;
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
      if (item->nodeVal->type == LEX_TYPE_OPEN) {
        open = item;
        close = open->next;
        localDepth = 1;
        while (close) {
          if (close->nodeVal->type == LEX_TYPE_CLOSE) {
            localDepth--;
            if (localDepth == 0) {
              break;
            }
          }
          else {
            if (close->nodeVal->type == LEX_TYPE_OPEN) {
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
        if (item->nodeVal->type == LEX_TYPE_CLOSE) {
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
          subExprsCall[depth].outItem = open->nodeVal;
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
            subExprsCall[depth].outItem = open->nodeVal;
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

//Функция вычисления последовательности выражений разделенных ';'
//В случае успеха возвращает численный ответ последнего выражения
//Иначе освобождает память и возвращает код ошибки
static resultCode_t ExprSequenceCalc(dblList_t** exprSequence, double* ans) {
  node_t* item = (*exprSequence)->head;
  dblList_t* expr = NULL;
  resultCode_t localResult = CRESULT_OK;
  double localAns = 0;
  while (item && !ResultCodeIsError(localResult)) {
    if (item->nodeVal->type == LEX_TYPE_SEMICOLON) {
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
  if ((*exprSequence) != NULL) {
    DblListFree(exprSequence);
  }
  if (expr != NULL) {
    DblListFree(&expr);
  }
  *ans = localAns;
  return localResult;

}

//Пытается обработать переданную строку как математическое выражение
//Возвращает информацию о результате, и, если попытка удачна, численный ответ
calcResult_t StringCalc(const char* const str, double* ans) {
  dblList_t* expr;
  resultCode_t result;
  result = LexSplit(str, &expr, &localVarList);
  if (ResultCodeIsError(result)) {
    return resultList[result];
  }
  result = ExprSequenceCalc(&expr, ans);
  VarListFree(&localVarList);
  return resultList[result];
}