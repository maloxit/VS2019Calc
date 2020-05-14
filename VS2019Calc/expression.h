// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#ifndef EXPRESSION_H_INCLUDED__
#define EXPRESSION_H_INCLUDED__
#pragma once

#include "resultCodes.h"

//Лексема, являющаяся элементом двусвязного списка, имеет тип и значение
typedef struct _nodevalue_t nodevalue_t;
typedef struct _node_t {
  nodevalue_t* nodeVal;
  struct _node_t* previous;
  struct _node_t* next;
} node_t;

//Двусвязный список
typedef struct dblList_t {
  node_t* head;
  node_t* end;
} dblList_t;

dblList_t* DblListCreate(void);

//Выгружает двусвязный список из памяти
void DblListFree(dblList_t** list);

//Добавляет элемент в конец двусвязного списка
node_t* DblListAppend(dblList_t* list, const nodevalue_t* insert);

//Отсекает от списка подсписок слева от элемента деления
//Используется для обработки последовательности выражений, записанных через ';'
resultCode_t DblListLeftSplit(dblList_t** leftList, dblList_t** mainList, node_t* splitPoint);

//Выделяет участок списка между двумя его элементами в подсписок, на месте него остается полько открываущий элемент
//Используется для выделения выражений, заключенных в круглые скобки
resultCode_t DblListExtructSubList(dblList_t* list, node_t* start, node_t* close, dblList_t** subList);

//Заменяет участок списка от начального элемента до конечного включительно на единственный переданный элемент
//Используется для замены вычисленных выражений на результат
void DblListShrinkSubList(dblList_t* list, node_t* start, node_t* end, const nodevalue_t* insert);

//Возвращает следующий за переданным элемент последовательности
node_t* NodeGetNext(node_t* item);

//Возвращает предыдущий переданному элемент последовательности
node_t* NodeGetPrevious(node_t* item);

#endif //EXPRESSION_H_INCLUDED__