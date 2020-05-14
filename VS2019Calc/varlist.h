// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#ifndef VARLIST_H_INCLUDED__
#define VARLIST_H_INCLUDED__
#pragma once

#include "bool.h"

//��������� ����� ������ ��������� ����������
#define VAR_LIST_BUFF_SIZE 10

//��������� ������������� ����������, ����� ����������� � ������������� ��������� � ��������
typedef struct var_t {
  double val;
  Bool isCalculated;
  char ch;
}var_t;

//������ ��������� ���������� � �������������� �������
typedef struct varList_t {
  var_t* at;
  int len;
  int size;
}varList_t;

//������� ������ ������ ��������� ����������
varList_t* VarListGet(void);

//��������� ������ ��������� ���������� �� ������
void VarListFree(varList_t** varList);

//���� �� ����� ������ ��������� ���������� � ������ � ���������� ���, ��� -1 � ������ �������
int VarListGetIndexByChar(varList_t* varList, char ch);

//��������� ������� � ����� ������ ��������� ����������
int VarListAppend(varList_t* varList, char ch);

#endif //VARLIST_H_INCLUDED__