#include <windows.h>
#include <conio.h>
#include <fstream>
#include <iostream>
#include <string>

DWORD WINAPI GreetingThread(LPVOID lpParam) {
	std::cout << "Поток 1 создан.\n";
	char message[] = "hello world";
	for (int i = 0; i < 11; ++i) {
		std::cout << message[i] << std::endl;
	}
	std::cout << "Поток с выводом приветствия завершен.\n";
	return 0;
}

DWORD WINAPI WriteMessageThread(LPVOID lpParam) {
	std::cout << "Поток с записью введенного сообщения стартовал.\n";
	std::string s;
	std::cout << "Введите текст \n";
	std::cin >> s;
	

	std::ofstream file("result.txt");
	if (file.is_open()) {
		file << s;
		file.close();
		std::cout << "Сообщение было записано.\n";
	}
	else {
		std::cerr << "Не удалось открыть файл result.txt.\n";
	}

	return 0;
}

int main() {
	setlocale(LC_ALL, "ru");

	wchar_t lpszComLine[130];
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	HANDLE hThread,hInheritProcess;
	DWORD IDThread;

	hThread = GetCurrentProcess();

	if (!DuplicateHandle(
		GetCurrentProcess(), 
		hThread,        
		GetCurrentProcess(),
		&hInheritProcess,   
		0,                   
		TRUE,                
		DUPLICATE_SAME_ACCESS))
	{
		std::cerr << "Ошибка копирования псевдодескриптора.\n";
		return GetLastError();
	}

	std::cout << "\nВведите любой знак для начала дочернего процесса: ";
	_getch();

	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	wsprintf(lpszComLine, L"C:\\Users\\chiki\\source\\repos\\secproject\\x64\\Debug\\secproject.exe %d", (int)hInheritProcess);

	if (!CreateProcess(
		NULL,
		lpszComLine,
		NULL,
		NULL,
		TRUE,
		CREATE_NEW_CONSOLE,
		NULL,
		NULL,
		&si,
		&pi
	)) {
		std::cerr << "Новый процесс не создан.\n";
		_cputs("Нажмите любую клавишу, чтобы завершить.\n");
		_getch();
		return GetLastError();
	}

	std::cout << "PID: " << pi.dwProcessId << std::endl;
	std::cout << "Дескриптор: " << pi.hProcess << std::endl;

	WaitForSingleObject(pi.hProcess, INFINITE);

	TerminateThread(pi.hThread, 0);
	TerminateProcess(pi.hProcess, 0);

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	CloseHandle(hInheritProcess);


	HANDLE hFile = CreateFile(L"test.txt", GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		std::cerr << "Не удалось открыть файл.\n";
		return 1;
	}

	HANDLE hFile2;
	if (!DuplicateHandle(GetCurrentProcess(), hFile, GetCurrentProcess(), &hFile2, 0, FALSE, DUPLICATE_SAME_ACCESS)) {
		std::cerr << "Не удалось дублировать дескриптор файла.\n";
		return 1;
	}

	char buffer[52] = "This line was written by handle!";
	DWORD dWritten;
	if (!WriteFile(hFile2, buffer, strlen(buffer), &dWritten, NULL)) {
		std::cerr << "Не удалось записать данные в файл.\n";
		return 1;
	}

	CloseHandle(hFile);
	CloseHandle(hFile2);

	std::ifstream file("test.txt");
	if (!file) {
		std::cerr << "Не удалось открыть файл.\n";
		return 1;
	}

	std::string line;
	while (std::getline(file, line)) {
		std::cout << "Значения файла записанное дескриптором: " << line << '\n';
	}

	file.close();

	HANDLE hThread1 = NULL;
	HANDLE hThread2 = NULL;
	char input;
	HANDLE hInheritProcess1;

	while (true) {
		if (hThread1 != NULL) {
			WaitForSingleObject(hThread1, INFINITE);
		}

		if (hThread2 != NULL) {
			WaitForSingleObject(hThread2, INFINITE);
		}

		std::cout << "\nВыберите действие:\n";
		std::cout << "1. Создать поток с приветствием\n";
		std::cout << "2. Создать поток с записью сообщения\n";
		std::cout << "3. Завершить поток c приветствим\n";
		std::cout << "4. Завершить поток с записью сообщения\n";
		std::cout << "n. Закрыть программу\n";
		std::cout << "Ваш выбор: ";
		std::cin >> input;
		system("cls");

		switch (input) {
		case '1':
			if (hThread1 == NULL) {
				hThread1 = CreateThread(NULL, 0, GreetingThread, NULL, 0, NULL);
				if (hThread1 == NULL) {
					std::cerr << "Не удалось создать поток.\n";
				}				
			}
			else {
				std::cerr << "Поток уже создан.\n";
			}
			break;
		case '2':
			if (hThread2 == NULL) {
				hThread2 = CreateThread(NULL, 0, WriteMessageThread, NULL, 0, NULL);
				if (hThread2 == NULL) {
					std::cerr << "Не удалось создать поток.\n";
				}
			}
			else {
				std::cerr << "Поток уже создан.\n";
			}
			break;
		case '3':
			if (hThread1 != NULL) {
				TerminateThread(hThread1, 0);
				CloseHandle(hThread1);
				hThread1 = NULL;

				std::cout << "Поток с приветствием завершен.\n";
			}
			else {
				std::cerr << "Поток не был создан.\n";
			}
			break;
		case '4':
			if (hThread2 != NULL) {
				TerminateThread(hThread2, 0);
				CloseHandle(hThread2);
				hThread2 = NULL;
				std::cout << "Поток c записью текста завершен.\n";
			}
			else {
				std::cerr << "Поток не был создан.\n";
			}			
			break;
		case 'n':
			if (hThread1 != NULL) {
				TerminateThread(hThread1, 0);
				CloseHandle(hThread1);
			}
			if (hThread2 != NULL) {
				TerminateThread(hThread2, 0);
				CloseHandle(hThread2);
			}
			
			return 0;
		default:
			std::cerr << "Неверный выбор. Попробуйте снова.\n";
			break;
		}
	}
	return 0;
}