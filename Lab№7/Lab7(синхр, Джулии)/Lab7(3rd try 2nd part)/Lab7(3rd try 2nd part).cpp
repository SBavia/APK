// Lab7(3rd try 2nd part).cpp: определяет точку входа для консольного приложения.
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
	
	do
	{
		hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"Event");
	} while (!hEvent);  //ожидаем получение дескриптора события, чтобы считать данные

	cout << "Connected!" << endl; //если получили событие, то порты соединились

	hFile = CreateFile(L"COM2", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	// - 0 - порт не может быть общедоступным (shared)
	// - NULL - дескриптор порта не наследуется, используется дескриптор безопасности по умолчанию
	// - OPEN_EXISTING - порт должен открываться как уже существующий файл
	// - 0 - флаг указывает на использование синхронных операций
	// - NULL - указатель на файл шаблона не используется при работе с портами

	if (hFile == NULL || hFile == INVALID_HANDLE_VALUE)		//если ошибка открытия порта
	{
		cout << "Port error " << GetLastError() << endl;  //вывести сообщение в строке состояния
		system("pause");
		return 0;
	}

	while (1)
	{
		
		WaitForSingleObject(hEvent, INFINITE);			//ждем перехода события в сигнальное состояние, чтобы считать инф-ию
		ReadFile(hFile, &buffer, sizeof(buffer), &cbWritten, NULL);   //прочитать байты из порта в буфер программы
		cout << "BUFFER: " << buffer << endl;			//вывести инф-ию из буфера
		
		//fgets(buffer, 256, stdin);  //считать инф-ию из потока и записать в буфер
		
		if (strcmp(buffer, "\n") == 0)  //если нажали только enter => закрываем порт
		{
			CloseHandle(hFile);  //закрыть порт
			CloseHandle(hEvent); //закрыть объект-событие
			return 0;
		}
	}
}