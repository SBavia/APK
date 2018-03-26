/*     16.  F(x) = sin(x)/( x^2+10)     */

#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<conio.h>
#include <windows.h>

void main(void)
{
	long double  a=0, b , step, x, f;
	char ans;
	int ten = 10, k=0;
	clock_t start, end;
	//LARGE_INTEGER frequency, start, finish;

	
	//QueryPerfomanceFrequency(&frequency);
	//QueryPerfomanceCounter(&start);
	do
	{
		do
		{
			printf("Input a: ");
			rewind(stdin);
			k = scanf("%lf", &a);
		} while (k == 0);
		do
		{
			printf("Input b: ");
			rewind(stdin);
			k = scanf("%lf", &b);
		} while (k == 0);
		do
		{
			printf("Input step: ");
			rewind(stdin);
			k = scanf("%lf", &step);
		} while (k == 0);

		x = a;
		f = 0;
		start = clock();
		/*QueryPerfomanceFrequency(&frequency);
		QueryPerfomanceCounter(&start);*/
		while (x <= b)
		{
			f += (sin(x) / (x*x + 10));
			x += step;
		};
		end = clock();
		/*QueryPerfomanceCounter(&finish);
		delay = ((double)finish.QuadPart - start.QuadPart) / (double)frequency.QuadPart;
		printf("\n%f\n", f);
		printf("time C = %f\n",delay);*/
		printf("\n%lf\n", f);
		printf("time C = %lf\n", (double)(end - start) / CLK_TCK);


		x = a;
		f = 0;
		start = clock();
		_asm {
			finit                 //����. ������-�
			fld b                 //����. ���. ����� � ����
			fld x                 //�����
loop_start :                      //����� �����
			fcom                  //������� ���������
			fstsw ax              //���. ���������� �������� ��������� � ��
			and ah, 01000101b     //�������� ���-� ����.
			jz loop_end           //������� �� ����� ����� ����� ���� x == b                                F(x) = sin(x)/( x^2+10) 
			fsin                  //����� �����
			fld x
			fmul x                //x*x
			fiadd ten             //������������� +
			fdiv                  //�������
			fadd f                //+
			fstp f                //�������� � ��������, � ����� �����������
			fld x
			fadd step             //+
			fst x                 //���� � ��������
			jmp loop_start        //����������� �������
loop_end :
			fwait                 //����� �����
		}
		end = clock();
		//QueryPerfomanceCounter(&finish);
		//float delay = (finish.QuadPart - start.QuadPart) / frequency.QuadPart;
		//printf("%f\n", f);
		//printf("time asm  =   %f\n", delay);
		printf("%lf\n", f);
		printf("time asm  =   %lf\n", (double)(end - start) / CLK_TCK);	


		printf("Cont?: ");
		rewind(stdin);
		scanf("%c", &ans);
	} while (ans == 'y');

	getch();
}


////LARGE_INTEGER frequency, start, finish;
////
////QueryPerfomanceFrequency(&frequency);
////QueryPerfomanceCounter(&start);
////QueryPerfomanceCounter(&finish);
////
////float delay = (finish.QuadPart - start.QuadPart) = 1000.0f / frequency.QuadPart;