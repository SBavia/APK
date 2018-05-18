#include <dos.h>
#include <conio.h>
#include <stdio.h>

void interrupt newInt9(...); 		 // ������� ��������� ����������
void interrupt (*oldInt9)(...); 		 // ��������� �� ���������� ����������
void indicator(unsigned char mask);	 // ������� ���������� ������������
void blinking (void);				 // ������� ������� ������������


int quitFlag = 0; 		 // ���� ������ �� ���������
int blinkingON = 0;		 // ���� ������� ������������


void main(){
oldInt9 = getvect(0x09);	 // ��������� ��������� �� ������ ����������
setvect(0x09, newInt9); 	 // ������ ��� �� �����

while(!quitFlag){		 // ���� �� ���������� ���� ������
if (blinkingON) 		 // ���� ���������� ���� ������� �����������
blinking(); 		 // ������ ������������
}

setvect(0x09, oldInt9); // ��������������� ������ ���������� ����������
return;

}

// ������� ��������� ����������

void interrupt newInt9(...){

unsigned char value = 0;
oldInt9();
value = inp(0x60); 				// �������� �������� �� ����� 60h

if(value == 0x01) quitFlag = 1; 		// ������������� ���� ������(������ Esc)

if (value == 0x26 && blinkingON == 0) // ���������
	blinkingON = 1; 				 // ��� ����� ���� �������,

//else if (value == 0x26 && blinkingON == 1) // ���� ������ ������� L
//	blinkingON = 0;

printf("\t%x", value);
outp(0x20, 0x20); 				// ����� ����������� ����������
}

// ������� ���������� ������������

void indicator(unsigned char mask){


	int flg=0;			// ���� ��� ������������� ��������� ���������� �������
	while((inp(0x64) & 0x02) != 0x00);
	outp(0x60, 0xED);

	for(int i=9;i>0;i--){
		if((inp(0x60) == 0xFA)) {
			flg=1;
			break;
		}
		delay(200);
	}

if(flg){
	while((inp(0x64) & 0x02) != 0x00);
	outp(0x60, mask);
	}

	delay(50);

}


// ������� ������� ������������

void blinking (){


indicator(0x02); // ���. ��������� Num Lock
delay(150);

indicator(0x04); // ���. ��������� Caps Lock
delay(150);

indicator(0x6); // ���. ���������� Num Lock � Caps Lock
delay(200);

indicator(0x00); // ����. ��� ����������
delay(50);

indicator(0x06); // ���. ��� ����������
delay(100);

indicator(0x00); // ����. ��� ���������
blinkingON=0;
}