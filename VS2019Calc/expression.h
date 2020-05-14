// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#ifndef EXPRESSION_H_INCLUDED__
#define EXPRESSION_H_INCLUDED__
#pragma once

#include "resultCodes.h"

//�������, ���������� ��������� ����������� ������, ����� ��� � ��������
typedef struct _nodevalue_t nodevalue_t;
typedef struct _node_t {
  nodevalue_t* nodeVal;
  struct _node_t* previous;
  struct _node_t* next;
} node_t;

//���������� ������
typedef struct dblList_t {
  node_t* head;
  node_t* end;
} dblList_t;

dblList_t* DblListCreate(void);

//��������� ���������� ������ �� ������
void DblListFree(dblList_t** list);

//��������� ������� � ����� ����������� ������
node_t* DblListAppend(dblList_t* list, const nodevalue_t* insert);

//�������� �� ������ ��������� ����� �� �������� �������
//������������ ��� ��������� ������������������ ���������, ���������� ����� ';'
resultCode_t DblListLeftSplit(dblList_t** leftList, dblList_t** mainList, node_t* splitPoint);

//�������� ������� ������ ����� ����� ��� ���������� � ���������, �� ����� ���� �������� ������ ����������� �������
//������������ ��� ��������� ���������, ����������� � ������� ������
resultCode_t DblListExtructSubList(dblList_t* list, node_t* start, node_t* close, dblList_t** subList);

//�������� ������� ������ �� ���������� �������� �� ��������� ������������ �� ������������ ���������� �������
//������������ ��� ������ ����������� ��������� �� ���������
void DblListShrinkSubList(dblList_t* list, node_t* start, node_t* end, const nodevalue_t* insert);

//���������� ��������� �� ���������� ������� ������������������
node_t* NodeGetNext(node_t* item);

//���������� ���������� ����������� ������� ������������������
node_t* NodeGetPrevious(node_t* item);

#endif //EXPRESSION_H_INCLUDED__