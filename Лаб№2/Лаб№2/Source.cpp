/*  1 вариант  -  ¬ычитание матриц  */


#include <iostream>
#include <conio.h>
#include <stdlib.h>
#include <time.h>
#include <Windows.h>
#define column  4
#define row  4

int main()
{
	int sum = 0;
	/*int matrix1[row][column] = { {10,9,8,7},{19,18,17,16} };        //= (int **)malloc(row * sizeof(int*));
	int matrix2[row][column] = { { 1,2,3,4 },{ 5,6,7,8 } };         //= (int **)malloc(row * sizeof(int*));
	int matrix3[row][column] = {  };                               //= (int **)malloc(row * sizeof(int*));*/
	int matrix4[row][column] = {};               //{ { 100,99,98,97 },{ 96,95,94,93 },{ 92,91,90,89 },{ 88,87,86,85 } };        //= (int **)malloc(row * sizeof(int*));
	int matrix5[row][column] = {};               //{ { 17,18,19,20 },{ 21,22,23,24 },{ 25,26,27,28 },{ 29,30,31,32 } };         //= (int **)malloc(row * sizeof(int*));
	int matrix6[row][column] = {};                                                                                              //= (int **)malloc(row * sizeof(int*));
																																/*int matrix7[row][column] = { { 3,4,5,6 },{ 7,8,9,10 } };       //= (int **)malloc(row * sizeof(int*));
																																int matrix8[row][column] = { { 1,1,1,1 },{ 1,1,1,1 } };       //= (int **)malloc(row * sizeof(int*));
																																int matrix9[row][column] = {};                                //= (int **)malloc(row * sizeof(int*));*/

	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < column; j++)
		{
			matrix4[i][j] = rand() % 20;
		}
	}

	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < column; j++)
		{
			matrix5[i][j] = rand() % 15;
		}
	}



	//clock_t start, end;
	LARGE_INTEGER timerFrequency, timerStart, timerStop;
	double time;
	QueryPerformanceFrequency(&timerFrequency);
	QueryPerformanceCounter(&timerStart);
	//start = clock();
	printf("\n\tMatrix (C) = ");
	for (int i = 0; i < row; i++)
	{
		printf("\n");
		for (int j = 0; j < column; j++)
		{
			matrix6[i][j] = matrix4[i][j] - matrix5[i][j];
			printf("\t%d ", matrix6[i][j]);
		}
	}
	//end = clock();
	printf("\n");
	//printf("\ttime C = %lf\n", (double)(end - start) / CLK_TCK);
	QueryPerformanceCounter(&timerStop);
	time = (double)(timerStop.QuadPart - timerStart.QuadPart) / (double)timerFrequency.QuadPart;
	printf("\ttime C  =  %f\n", time);



	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < column; j++)
		{
			matrix6[i][j] = 0;
		}
	}
	printf("\n");



	int iterations = row*column;
	QueryPerformanceFrequency(&timerFrequency);
	QueryPerformanceCounter(&timerStart);
	//start = clock();
	//for (int k = 0; k < 50; k++)
	//{

	_asm
	{
		pusha             //; сохранить в стек все регистры
		xor     eax, eax
		xor     esi, esi
		l1 :
		mov eax, [matrix4 + esi]           //заносим элемы 1-ой матрицы

			sub eax, [matrix5 + esi]        //вычитаем элем-ы
			mov[matrix6 + esi], eax         //сохр рез-т

			add esi, 4                      //шаг по 4 байта?????
			sub iterations, 1              //по 1 числу за итерацию
			jnz l1                                //прыжок на метку пока iter!=0

			popa                          //восст. из стека сохр-ые регистры
	}
	//}
	//end = clock();
	QueryPerformanceCounter(&timerStop);
	printf("\n\tMatrix (Asm) = ");
	for (int i = 0; i < row; i++)
	{
		printf("\n");
		for (int j = 0; j < column; j++)
		{
			printf("\t%d ", matrix6[i][j]);
		}
	}
	printf("\n");
	//printf("\ttime asm  =   %lf\n", (double)(end - start) / CLK_TCK);
	time = (double)(timerStop.QuadPart - timerStart.QuadPart) / (double)timerFrequency.QuadPart;
	printf("\ttime Asm(w/o MMX)  =   %f\n", time);



	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < column; j++)
		{
			matrix6[i][j] = 0;
		}
	}
	printf("\n");



	QueryPerformanceFrequency(&timerFrequency);
	QueryPerformanceCounter(&timerStart);
	//start = clock();
	iterations = row*column;
	for (int k1 = 0; k1 < 60; k1++)
	{
		_asm
		{
			pusha                           //сохранить в стек все регистры
			mov ecx, 8
			xor esi, esi
			l2 :
			movq MM0, [matrix4 + esi]         //чтение из пам€ти

				psubd MM0, [matrix5 + esi]     //вычитание элем-ов матриц по 2 сразу
				movq[matrix6 + esi], MM0     //сохр рез-т 

				add esi, 8                     //шаг по 2 байта * 4
				loop l2
				emms                         //вернуть режим сопроцессора
				popa                         //восстановить регистры
		}
	}
	QueryPerformanceCounter(&timerStop);
	//end = clock();

	printf("\n\tMatrix (Asm(MMX)) = ");
	for (int i = 0; i < row; i++)
	{
		printf("\n");
		for (int j = 0; j < column; j++)
		{
			printf("\t%d ", matrix6[i][j]);
		}
	}

	printf("\n");
	//printf("\ttime asm(MMX)  =   %lf\n", (double)(end - start) / CLK_TCK);
	time = (double)(timerStop.QuadPart - timerStart.QuadPart) / (double)timerFrequency.QuadPart;
	printf("\ttime Asm(with MMX)  =  %f\n", time);
	_getch();
	return 0;
}