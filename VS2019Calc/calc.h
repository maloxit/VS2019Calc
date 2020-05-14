// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#ifndef CALC_H_INCLUDED__
#define CALC_H_INCLUDED__
#pragma once

#include "bool.h"

//��������� ������ ������
typedef struct calcResult_t {
  Bool isError;     //�������� �� �������
  char text[35];    //��������� �������� ����������
} calcResult_t;


//������� � ����������� ����� ������ ����� ���������� ���������� (������)
void CalcResultPrint(calcResult_t err);

//�������� ���������� ���������� ������ ��� �������������� ���������
//���������� ���������� � ����������, �, ���� ������� ������, ��������� �����
calcResult_t StringCalc(const char* const str, double* ans);

#endif //CALC_H_INCLUDED__