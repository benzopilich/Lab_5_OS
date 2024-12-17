#include <iostream>
#include <vector>
#include <windows.h>
#include <cctype>
using namespace std;

void ErrorExit(const char* message) {
    std::cerr << message << " Error: " << GetLastError() << std::endl;
    exit(1);
}

int main() {
    HANDLE hReadPipe = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE hWritePipe = GetStdHandle(STD_OUTPUT_HANDLE);

    // Подключение к семафорам
    HANDLE hServerSemaphore = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, L"Semaphore");

    if (!hServerSemaphore) {
        ErrorExit("Failed to open semaphores.");
    }

    // Чтение данных из канала
    WaitForSingleObject(hServerSemaphore, INFINITE);

    int size, n;
    DWORD read;
    ReadFile(hReadPipe, &size, sizeof(size), &read, nullptr); // Читаем размер массива
    ReadFile(hReadPipe, &n, sizeof(n), &read, nullptr);       // Читаем число N

    std::vector<char> originalArray(size);     // Массив исходных символов
    std::vector<char> processedArray(size);    // Массив обработанных символов

    for (int i = 0; i < size; ++i) {
        char ch;
        ReadFile(hReadPipe, &ch, sizeof(ch), &read, nullptr);
        originalArray[i] = ch;

        // Проверяем, является ли символ цифрой или знаком препинания
        if (isdigit(ch) || ispunct(ch)) {
            processedArray[i] = ch;
        }
        else {
            processedArray[i] = '-'; // Если не подходит, заменяем
        }

        WriteFile(hWritePipe, &processedArray[i], sizeof(char), &read, nullptr); // Отправляем обработанный символ
    }


    // Вывод массивов на консоль
    std::cerr << "Original array: ";
    for (char ch : originalArray) {
        std::cerr << ch << " ";
    }
    std::cerr << std::endl;

    std::cerr << "Processed array: ";
    for (char ch : processedArray) {
        std::cerr << ch << " ";
    }
    std::cerr << std::endl;
    ReleaseSemaphore(hServerSemaphore, 1, nullptr);// уведомляем сервер
    while(1){}
    // Закрытие ресурсов
    CloseHandle(hReadPipe);
    CloseHandle(hWritePipe);
    CloseHandle(hServerSemaphore);
    return 0;
}
