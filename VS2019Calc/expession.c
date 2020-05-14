// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include <stdlib.h>
#include "expression.h"
#include "lexems.h"

//������� ���������� ������
dblList_t* DblListCreate(void) {
  dblList_t* list = (dblList_t*)malloc(sizeof(dblList_t));
  if (list != NULL) {
    list->head = NULL;
    list->end = NULL;
  }
  return list;
}

//��������� ���������� ������ �� ������
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

//��������� ������� � ����� ����������� ������
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

//�������� �� ������ ��������� ����� �� �������� �������
//������������ ��� ��������� ������������������ ���������, ���������� ����� ';'
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

//�������� ������� ������ ����� ����� ��� ���������� � ���������, �� ����� ���� �������� ������ ����������� �������
//������������ ��� ��������� ���������, ����������� � ������� ������
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

//�������� ������� ������ �� ���������� �������� �� ��������� ������������ �� ������������ ���������� �������
//������������ ��� ������ ����������� ��������� �� ���������
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

//���������� ��������� �� ���������� ������� ������������������
node_t* NodeGetNext(node_t* item) {
  return item->next;
}

//���������� ���������� ����������� ������� ������������������
node_t* NodeGetPrevious(node_t* item) {
  return item->previous;
}
