#include <dos.h>
#include <stdio.h>
#include <io.h>
#include <stdlib.h>
unsigned int blinking = 0;
int quitFlag = 0;
unsigned int repeat = 1;
unsigned int attempt = 3;

void interrupt(*oldInt09)(...);
void interrupt newInt09(...)
{
	unsigned char keyCode;

	keyCode = inp(0x60);/* read scancode from keyboard controller ;60h � ������� ������,*/
	if (keyCode == 0x01)
	{
		quitFlag = 1;
		return;
	}
	
	if (keyCode == 0xFE && blinking)//FE � ��������� ������,���� 3 �������
	{
		if (attempt-- == 0)
		{
			printf("Error. Input register of keyboard is busy.\n");
			quitFlag = 1;
		}
		repeat = 1;
	}
	else
	{
		repeat = 0;
		attempt = 3;
	}

	printf("\t%x", keyCode);
	outp(0x20, 0x20); // �������� ����������� ���������� (master) ������ EOI (end of interrupt)
	outp(0xA0, 0x20);// �������� ������� ����������� ���������� (slave) ������ EOI (end of interrupt)
}

void keyBlinking(unsigned char i)
{
	int timeout = 50000;
	repeat=1;
	while (repeat)
	{
		//��������� �������� ������ �� ������� ������ ����������
		while (timeout-- > 0)
		{
			//��� ���������� �������� ��������, ������ ������ ������� ������ ����������� ������ ����� �������� ����������� �������� �������� ����������� ����������.
			if ((inp(0x64) & 0x02) == 0x00) //������ ��������� �����, ��������(0 � 1 ����) ��� �����(1 � 1 ����) ������� 64h � ������� ��������� (�������)
			{
				break;
			}
		}
		if (!timeout)
		{
			printf("Timeout error.\n");
			disable();
			setvect(0x09, oldInt09);
			enable();
			exit(1);
		}
		//��� ���������� ������������ ����� 60h ������������ ��� EDh. ����� ������, � ������������ � ������� ������ ���������� ����������.

		outp(0x60, 0xED); //����� ��������, ����� ����������� ����
		delay(200);
	}
	timeout = 50000;
	repeat=1;
	while (repeat)
	{
		while (timeout-- > 0){
			if ((inp(0x64) & 0x02) == 0x00) //������ ��������� �����
			{
				break;
			}
		}
		if (!timeout)
		{
			printf("Timeout error.\n");
			disable();
			setvect(0x09, oldInt09);
			enable();
			exit(1);
		}
		outp(0x60, i); //����� ��������, ����� ����������� ����
		delay(200);
	}
}

int main()
{
	delay(200);
	disable();
	oldInt09 = getvect(0x09); //�������� ������ ���������� ����������
	setvect(0x09, newInt09);	// ������������� ����� ���������� 
	enable();
	blinking = 1;
	for(int j=0; j<10;j++)
	{
		keyBlinking(0x04);
		delay(300);
		keyBlinking(0x00);
	}
	blinking = 0;
	
	while (!quitFlag);
	disable();
	setvect(0x09, oldInt09);
	enable();
	return 0;
}
