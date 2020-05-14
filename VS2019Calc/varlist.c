// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include <stdlib.h>
#include "varlist.h"

//��������� ����� ������ ��������� ����������
#define VAR_LIST_BUFF_SIZE 10

//������� ������ ������ ��������� ����������
varList_t* VarListGet(void) {
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

//��������� ������ ��������� ���������� �� ������
void VarListFree(varList_t** varList) {
  free((*varList)->at);
  free(*varList);
  *varList = NULL;
}

//���� �� ����� ������ ��������� ���������� � ������ � ���������� ���, ��� -1 � ������ �������
int VarListGetIndexByChar(varList_t* varList, char ch) {
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

//��������� ������� � ����� ������ ��������� ����������
int VarListAppend(varList_t* varList, char ch) {
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