#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE
#pragma warning(disable : 4996)

#include <iostream>
#include <windows.h>
#include <time.h>
#include <string.h>
using namespace std;
#define BUFSIZE 255

unsigned char bufrd[BUFSIZE], bufwr[BUFSIZE]; //������� � ���������� ������

HANDLE COMport;					//���������� �����
HANDLE reader;					//���������� ������ ������ �� �����
HANDLE writer;					//���������� ������ ������ � ����

OVERLAPPED overlapped;			//��������� OVERLAPPED ��� ����������� ��������
OVERLAPPED overlappedwr;

bool fl = 0;					//����, ����������� �� ���������� �������� ������ (1 - �������, 0 - �� �������)

unsigned long counter;			//������� �������� ������, ���������� ��� ������ �������� �����	

//void COMOpen(void);           //������� ����
void COMClose(void);            //������� ����

void ReadPrinting(void);

//������� ������� ������, ��������� ���� ������ �� COM-�����
DWORD WINAPI ReadThread(LPVOID)
{
	COMSTAT comstat;		//��������� �������� ��������� �����, � ������ ��������� ������������ ��� ����������� ���������� �������� � ���� ������
	DWORD bytes, temp, mask, result;	//���������� temp ������������ � �������� ��������

	overlapped.hEvent = CreateEvent(NULL, true, true, NULL);	//������� ���������� ������-������� ��� ����������� ��������
	SetCommMask(COMport, EV_RXCHAR);                   	        //���������� ����� �� ������������ �� ������� ����� ����� � ����
	while (1)
	{
		WaitCommEvent(COMport, &mask, &overlapped);               	//������� ������� ����� ����� (��� � ���� ������������� ��������)
		result = WaitForSingleObject(overlapped.hEvent, INFINITE);	//������������� ����� �� ������� �����
		if (result == WAIT_OBJECT_0)								//���� ������� ������� ����� ���������
		{
			if (GetOverlappedResult(COMport, &overlapped, &temp, true)) //���������, ������� �� ����������� ������������� �������� WaitCommEvent
				if ((mask & EV_RXCHAR) != 0)							//���� ��������� ������ ������� ������� �����
				{
					ClearCommError(COMport, &temp, &comstat);		//����� ��������� ��������� COMSTAT
					bytes = comstat.cbInQue;                          //���������� �������� ������
					if (bytes)
					{
						ReadFile(COMport, bufrd, bytes, &temp, &overlapped);     //��������� ����� �� ����� � ����� ���������
						counter += bytes;                                          //����������� ������� ������
						ReadPrinting();
					}
				}
		}
	}
}

//������� �������� ����� �� ����� � � ����
void ReadPrinting()
{
	cout << "�������: " << (char*)bufrd << endl;
	cout << "����� ������� " << counter << " ����";
	memset(bufrd, 0, BUFSIZE);					//�������� ����� (����� ������ �� ������������� ���� �� �����)
}

//������� ������� ������, ��������� �������� ������ �� ������ � COM-����
DWORD WINAPI WriteThread(LPVOID)
{
	DWORD temp, result;	//temp - ����������-��������

	overlappedwr.hEvent = CreateEvent(NULL, true, true, NULL);   	  //������� �������
	while (1)
	{
		WriteFile(COMport, bufwr, strlen((char*)bufwr), &temp, &overlappedwr);  //�������� ����� � ���� (������������� ��������!)
		result = WaitForSingleObject(overlappedwr.hEvent, INFINITE);	  //������������� �����, ���� �� ���������� ������������� �������� WriteFile

		if ((result == WAIT_OBJECT_0) && (GetOverlappedResult(COMport, &overlappedwr, &temp, true)))	//���� �������� ����������� �������
		{
			cout << "�������� ������ �������" << endl;    //������� ��������� �� ���� � ������ ���������
		}
		else { cout << "������ ��������" << endl; } 	//����� ������� � ������ ��������� ��������� �� ������

		SuspendThread(writer);

	}
}

//������� �������� � ������������� �����
void COMOpen(char * portname)
{
	//portname -��� ����� (��������, "COM1", "COM2" � �.�.)
	DCB dcb;                //��������� ��� ����� ������������� ����� DCB
	COMMTIMEOUTS timeouts;  //��������� ��� ��������� ���������

	COMport = CreateFile(portname, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	// - 0 - ���� �� ����� ���� ������������� (shared)
	// - NULL - ���������� ����� �� �����������, ������������ ���������� ������������ �� ���������
	// - OPEN_EXISTING - ���� ������ ����������� ��� ��� ������������ ����
	// - FILE_FLAG_OVERLAPPED - ���� ���� ��������� �� ������������� ����������� ��������
	// - NULL - ��������� �� ���� ������� �� ������������ ��� ������ � �������

	if (COMport == INVALID_HANDLE_VALUE)            //���� ������ �������� �����
	{
		cout << "�� ������� ������� ����" << endl;  //������� ��������� � ������ ���������
		return;
	}

	dcb.DCBlength = sizeof(DCB); 		//� ������ ���� ��������� DCB ���������� ������� � �����, ��� ����� �������������� ��������� ��������� ����� ��� �������� ������������ ���������

	if (!GetCommState(COMport, &dcb))	//������� ��������� DCB �� ����� ,���� �� ������� - ������� ���� � ������� ��������� �� ������ � ������ ���������
	{
		COMClose();
		cout << "�� ������� ������� DCB" << endl;
		return;
	}

	//������������� ��������� DCB
	dcb.BaudRate = 115200;								   //����� ������������ �������� �������� = 115200 ���
	dcb.fBinary = TRUE;                                    //�������� �������� ����� ������
	dcb.fOutxCtsFlow = FALSE;                              //��������� ����� �������� �� �������� CTS
	dcb.fOutxDsrFlow = FALSE;                              //��������� ����� �������� �� �������� DSR
	dcb.fDtrControl = DTR_CONTROL_DISABLE;                 //��������� ������������� ����� DTR
	dcb.fDsrSensitivity = FALSE;                           //��������� ��������������� �������� � ��������� ����� DSR
	dcb.fNull = FALSE;                                     //��������� ���� ������� ������
	dcb.fRtsControl = RTS_CONTROL_DISABLE;                 //��������� ������������� ����� RTS
	dcb.fAbortOnError = FALSE;                             //��������� ��������� ���� �������� ������/������ ��� ������
	dcb.ByteSize = 8;                                      //����� 8 ��� � �����
	dcb.Parity = 0;                                        //��������� �������� ��������
	dcb.StopBits = 0;                                      //����� ���� ����-���

														   //��������� ��������� DCB � ����
	if (!SetCommState(COMport, &dcb))	//���� �� ������� - ������� ���� � ������� ��������� �� ������ � ������ ���������
	{
		COMClose();
		cout << "�� ������� ���������� DCB" << endl;
		return;
	}

	//���������� ��������
	timeouts.ReadIntervalTimeout = 0;	 	//������� ����� ����� ���������
	timeouts.ReadTotalTimeoutMultiplier = 0;//����� ������� �������� ������
	timeouts.ReadTotalTimeoutConstant = 0;  //��������� ��� ������ �������� �������� ������
	timeouts.WriteTotalTimeoutMultiplier = 0;//����� ������� �������� ������
	timeouts.WriteTotalTimeoutConstant = 0;  //��������� ��� ������ �������� �������� ������

											 //�������� ��������� ��������� � ����
	if (!SetCommTimeouts(COMport, &timeouts))	//���� �� ������� - ������� ���� � ������� ��������� �� ������ � ������ ���������
	{
		COMClose();
		cout << "�� ������� ���������� ����-����" << endl;
		return;
	}

	//���������� ������� �������� ����� � ��������
	SetupComm(COMport, 2000, 2000);

	PurgeComm(COMport, PURGE_RXCLEAR);									//�������� ����������� ����� �����

	reader = CreateThread(NULL, 0, ReadThread, NULL, 0, NULL);			//����� ������ �������
	writer = CreateThread(NULL, 0, WriteThread, NULL, CREATE_SUSPENDED, NULL);	//������ ����� ������ � ������������� ��������� (������������� �������� = CREATE_SUSPENDED)

}

//������� �������� �����
void COMClose()
{

	if (writer)			//���� �������� ����� ������ 
	{
		TerminateThread(writer, 0);
		CloseHandle(overlappedwr.hEvent);//������� ������-�������
		CloseHandle(writer);
	}
	if (reader)		   //���� �������� ����� ������ 
	{
		TerminateThread(reader, 0);
		CloseHandle(overlapped.hEvent);	//������� ������-�������
		CloseHandle(reader);
	}

	CloseHandle(COMport);                //������� ����
	COMport = 0;

}