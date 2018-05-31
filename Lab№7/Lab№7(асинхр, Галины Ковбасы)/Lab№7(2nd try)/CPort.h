#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE
#pragma warning(disable : 4996)

#include <iostream>
#include <windows.h>
#include <time.h>
#include <string.h>
using namespace std;
#define BUFSIZE 255

unsigned char bufrd[BUFSIZE], bufwr[BUFSIZE]; //приёмный и передающий буферы

HANDLE COMport;					//дескриптор порта
HANDLE reader;					//дескриптор потока чтения из порта
HANDLE writer;					//дескриптор потока записи в порт

OVERLAPPED overlapped;			//структуры OVERLAPPED для асинхронных операций
OVERLAPPED overlappedwr;

bool fl = 0;					//флаг, указывающий на успешность операций записи (1 - успешно, 0 - не успешно)

unsigned long counter;			//счётчик принятых байтов, обнуляется при каждом открытии порта	

//void COMOpen(void);           //открыть порт
void COMClose(void);            //закрыть порт

void ReadPrinting(void);

//главная функция потока, реализует приём байтов из COM-порта
DWORD WINAPI ReadThread(LPVOID)
{
	COMSTAT comstat;		//структура текущего состояния порта, в данной программе используется для определения количества принятых в порт байтов
	DWORD bytes, temp, mask, result;	//переменная temp используется в качестве заглушки

	overlapped.hEvent = CreateEvent(NULL, true, true, NULL);	//создать сигнальный объект-событие для асинхронных операций
	SetCommMask(COMport, EV_RXCHAR);                   	        //установить маску на срабатывание по событию приёма байта в порт
	while (1)
	{
		WaitCommEvent(COMport, &mask, &overlapped);               	//ожидать события приёма байта (это и есть перекрываемая операция)
		result = WaitForSingleObject(overlapped.hEvent, INFINITE);	//приостановить поток до прихода байта
		if (result == WAIT_OBJECT_0)								//если событие прихода байта произошло
		{
			if (GetOverlappedResult(COMport, &overlapped, &temp, true)) //проверяем, успешно ли завершилась перекрываемая операция WaitCommEvent
				if ((mask & EV_RXCHAR) != 0)							//если произошло именно событие прихода байта
				{
					ClearCommError(COMport, &temp, &comstat);		//нужно заполнить структуру COMSTAT
					bytes = comstat.cbInQue;                          //количество принятых байтов
					if (bytes)
					{
						ReadFile(COMport, bufrd, bytes, &temp, &overlapped);     //прочитать байты из порта в буфер программы
						counter += bytes;                                          //увеличиваем счётчик байтов
						ReadPrinting();
					}
				}
		}
	}
}

//выводим принятые байты на экран и в файл
void ReadPrinting()
{
	cout << "Принято: " << (char*)bufrd << endl;
	cout << "Всего принято " << counter << " байт";
	memset(bufrd, 0, BUFSIZE);					//очистить буфер (чтобы данные не накладывались друг на друга)
}

//главная функция потока, выполняет передачу байтов из буфера в COM-порт
DWORD WINAPI WriteThread(LPVOID)
{
	DWORD temp, result;	//temp - переменная-заглушка

	overlappedwr.hEvent = CreateEvent(NULL, true, true, NULL);   	  //создать событие
	while (1)
	{
		WriteFile(COMport, bufwr, strlen((char*)bufwr), &temp, &overlappedwr);  //записать байты в порт (перекрываемая операция!)
		result = WaitForSingleObject(overlappedwr.hEvent, INFINITE);	  //приостановить поток, пока не завершится перекрываемая операция WriteFile

		if ((result == WAIT_OBJECT_0) && (GetOverlappedResult(COMport, &overlappedwr, &temp, true)))	//если операция завершилась успешно
		{
			cout << "Передача прошла успешно" << endl;    //вывести сообщение об этом в строке состояния
		}
		else { cout << "Ошибка передачи" << endl; } 	//иначе вывести в строке состояния сообщение об ошибке

		SuspendThread(writer);

	}
}

//функция открытия и инициализации порта
void COMOpen(char * portname)
{
	//portname -имя порта (например, "COM1", "COM2" и т.д.)
	DCB dcb;                //структура для общей инициализации порта DCB
	COMMTIMEOUTS timeouts;  //структура для установки таймаутов

	COMport = CreateFile(portname, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	// - 0 - порт не может быть общедоступным (shared)
	// - NULL - дескриптор порта не наследуется, используется дескриптор безопасности по умолчанию
	// - OPEN_EXISTING - порт должен открываться как уже существующий файл
	// - FILE_FLAG_OVERLAPPED - этот флаг указывает на использование асинхронных операций
	// - NULL - указатель на файл шаблона не используется при работе с портами

	if (COMport == INVALID_HANDLE_VALUE)            //если ошибка открытия порта
	{
		cout << "Не удалось открыть порт" << endl;  //вывести сообщение в строке состояния
		return;
	}

	dcb.DCBlength = sizeof(DCB); 		//в первое поле структуры DCB необходимо занести её длину, она будет использоваться функциями настройки порта для контроля корректности структуры

	if (!GetCommState(COMport, &dcb))	//считать структуру DCB из порта ,если не удалось - закрыть порт и вывести сообщение об ошибке в строке состояния
	{
		COMClose();
		cout << "Не удалось считать DCB" << endl;
		return;
	}

	//инициализация структуры DCB
	dcb.BaudRate = 115200;								   //задаём максимальную скорость передачи = 115200 бод
	dcb.fBinary = TRUE;                                    //включаем двоичный режим обмена
	dcb.fOutxCtsFlow = FALSE;                              //выключаем режим слежения за сигналом CTS
	dcb.fOutxDsrFlow = FALSE;                              //выключаем режим слежения за сигналом DSR
	dcb.fDtrControl = DTR_CONTROL_DISABLE;                 //отключаем использование линии DTR
	dcb.fDsrSensitivity = FALSE;                           //отключаем восприимчивость драйвера к состоянию линии DSR
	dcb.fNull = FALSE;                                     //разрешить приём нулевых байтов
	dcb.fRtsControl = RTS_CONTROL_DISABLE;                 //отключаем использование линии RTS
	dcb.fAbortOnError = FALSE;                             //отключаем остановку всех операций чтения/записи при ошибке
	dcb.ByteSize = 8;                                      //задаём 8 бит в байте
	dcb.Parity = 0;                                        //отключаем проверку чётности
	dcb.StopBits = 0;                                      //задаём один стоп-бит

														   //загрузить структуру DCB в порт
	if (!SetCommState(COMport, &dcb))	//если не удалось - закрыть порт и вывести сообщение об ошибке в строке состояния
	{
		COMClose();
		cout << "Не удалось установить DCB" << endl;
		return;
	}

	//установить таймауты
	timeouts.ReadIntervalTimeout = 0;	 	//таймаут между двумя символами
	timeouts.ReadTotalTimeoutMultiplier = 0;//общий таймаут операции чтения
	timeouts.ReadTotalTimeoutConstant = 0;  //константа для общего таймаута операции чтения
	timeouts.WriteTotalTimeoutMultiplier = 0;//общий таймаут операции записи
	timeouts.WriteTotalTimeoutConstant = 0;  //константа для общего таймаута операции записи

											 //записать структуру таймаутов в порт
	if (!SetCommTimeouts(COMport, &timeouts))	//если не удалось - закрыть порт и вывести сообщение об ошибке в строке состояния
	{
		COMClose();
		cout << "Не удалось установить тайм-ауты" << endl;
		return;
	}

	//установить размеры очередей приёма и передачи
	SetupComm(COMport, 2000, 2000);

	PurgeComm(COMport, PURGE_RXCLEAR);									//очистить принимающий буфер порта

	reader = CreateThread(NULL, 0, ReadThread, NULL, 0, NULL);			//поток чтения запущен
	writer = CreateThread(NULL, 0, WriteThread, NULL, CREATE_SUSPENDED, NULL);	//создаём поток записи в остановленном состоянии (предпоследний параметр = CREATE_SUSPENDED)

}

//функция закрытия порта
void COMClose()
{

	if (writer)			//если работает поток записи 
	{
		TerminateThread(writer, 0);
		CloseHandle(overlappedwr.hEvent);//закрыть объект-событие
		CloseHandle(writer);
	}
	if (reader)		   //если работает поток чтения 
	{
		TerminateThread(reader, 0);
		CloseHandle(overlapped.hEvent);	//закрыть объект-событие
		CloseHandle(reader);
	}

	CloseHandle(COMport);                //закрыть порт
	COMport = 0;

}