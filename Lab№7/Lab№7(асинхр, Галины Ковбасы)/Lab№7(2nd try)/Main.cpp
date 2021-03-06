// Main.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "CPort.h"
#include <windows.h>

int main()
{
	setlocale(LC_ALL, "Russian");
	cout << "Выберите порт для открытия ( COM1 или COM2 )" << endl;
	char str[20];
	cin >> str; 
	COMOpen(str);                   //если кнопка нажата - открыть порт
	counter = 0;
	char c;
	cout << "Введите w для ввода или с для выхода" << endl;
	while (1)
	{
		cin >> c;
		if (c == 'w')
		{
			cout << "Введите строку" << endl;
			cin >> str;
			memset(bufwr, 0, BUFSIZE);			//очистить программный буфер
			PurgeComm(COMport, PURGE_TXCLEAR);  //очистить буфер порта
			strcpy((char*)bufwr, str);
			ResumeThread(writer);
		}
		else if (c == 'c') {
			COMClose();							//закрыть порт
			return 0;
		}
		else
			system("pause");
	}
	return 0;
}