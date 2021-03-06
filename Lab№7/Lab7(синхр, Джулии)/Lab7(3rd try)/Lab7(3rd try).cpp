// Lab7(3rd try).cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <iostream>
#include <Windows.h>
#include <conio.h>
using namespace std;

int main()
{
	HANDLE hFile;				//дескриптор порта
	DWORD cbWritten;			//переменная-заглушка
	HANDLE hEvent;				//дескриптор события
	char buffer[256];			//буфер для введенной инф-ии

	hFile = CreateFile(L"COM1", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	// - 0 - порт не может быть общедоступным (shared)
	// - NULL - дескриптор порта не наследуется, используется дескриптор безопасности по умолчанию
	// - OPEN_EXISTING - порт должен открываться как уже существующий файл
	// - 0 - флаг указывает на использование синхронных операций
	// - NULL - указатель на файл шаблона не используется при работе с портами

	if (hFile == NULL || hFile == INVALID_HANDLE_VALUE)			//если ошибка открытия порта
	{
		cout << "Open port error: " << GetLastError() << endl;	//вывести сообщение в строке состояния
		system("pause");
		return 0;
	}

	hEvent = CreateEvent(NULL, false, false, L"Event");   //создать событие

	if (hEvent == NULL || hEvent == INVALID_HANDLE_VALUE)  //если ошибка создания события
	{
		cout << "Create event error: " << GetLastError() << endl; //вывести сообщение в строке состояния
		system("pause");
		return 0;
	}

	while (1)
	{
		fflush(stdin);			//очистить поток ввода/вывода
		cout << "Input string: ";
		fgets(buffer, 256, stdin);  //считать инф-ию из потока и записать в буфер

		WriteFile(hFile, &buffer, sizeof(buffer), &cbWritten, NULL);  //записать байты в порт

		SetEvent(hEvent);			//установить событие в сигнальное состояние

		if (strcmp(buffer, "\n") == 0)   //если нажали только enter => закрываем порт
		{
			CloseHandle(hFile);  //закрыть порт
			CloseHandle(hEvent); //закрыть объект-событие
			return 0;
		}
	}
}