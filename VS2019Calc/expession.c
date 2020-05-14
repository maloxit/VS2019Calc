// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include <stdlib.h>
#include "expression.h"
#include "lexems.h"

//Создает двусвязный список
dblList_t* DblListCreate(void) {
  dblList_t* list = (dblList_t*)malloc(sizeof(dblList_t));
  if (list != NULL) {
    list->head = NULL;
    list->end = NULL;
  }
  return list;
}

//Выгружает двусвязный список из памяти
void DblListFree(dblList_t** list) {
  node_t* item = (*list)->head, * nextI = NULL;
  while (item) {
    nextI = item->next;
    free(item->nodeVal);
    free(item);
    item = nextI;
  }
  free(*list);
  *list = NULL;
}

//Добавляет элемент в конец двусвязного списка
node_t* DblListAppend(dblList_t* list, const nodevalue_t* insert) {
  node_t* item = (node_t*)malloc(sizeof(node_t));
  if (!item) {
    return NULL;
  }
  item->nodeVal = (nodevalue_t*)malloc(sizeof(nodevalue_t));
  if ((item->nodeVal) != NULL) {
    *(item->nodeVal) = *insert;
  }
  else {
    free(item);
    return NULL;
  }
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

//Отсекает от списка подсписок слева от элемента деления
//Используется для обработки последовательности выражений, записанных через ';'
resultCode_t DblListLeftSplit(dblList_t** leftList, dblList_t** mainList, node_t* splitPoint) {
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
  free(splitPoint->nodeVal);
  free(splitPoint);
  return CRESULT_OK;
}

//Выделяет участок списка между двумя его элементами в подсписок, на месте него остается полько открываущий элемент
//Используется для выделения выражений, заключенных в круглые скобки
resultCode_t DblListExtructSubList(dblList_t* list, node_t* start, node_t* close, dblList_t** subList) {
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
  free(close->nodeVal);
  free(close);
  return CRESULT_OK;
}

//Заменяет участок списка от начального элемента до конечного включительно на единственный переданный элемент
//Используется для замены вычисленных выражений на результат
void DblListShrinkSubList(dblList_t* list, node_t* start, node_t* end, const nodevalue_t* insert) {
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
    free(item->nodeVal);
    free(item);
    item = next;
  }
  *(start->nodeVal) = *insert;
}

//Возвращает следующий за переданным элемент последовательности
node_t* NodeGetNext(node_t* item) {
  return item->next;
}

//Возвращает предыдущий переданному элемент последовательности
node_t* NodeGetPrevious(node_t* item) {
  return item->previous;
}
