#include <iostream>
#include <vector>
#include <windows.h>

void ErrorExit(const char* message) {
    std::cerr << message << " Error: " << GetLastError() << std::endl;
    exit(1);
}

int main() {
    HANDLE hReadPipe, hWritePipe;
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };
    sa.bInheritHandle = TRUE;
   

    // Создаем анонимный канал
    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
        ErrorExit("Failed to create pipe.");
    }

    // Создаем семафор
    HANDLE hServerSemaphore = CreateSemaphore(nullptr, 0, 1, L"Semaphore");

    if (!hServerSemaphore) {
        ErrorExit("Failed to create semaphores.");
    }

    // Подготовка запуска дочернего процесса
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi = {};
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags |= STARTF_USESTDHANDLES;
    si.hStdInput = hReadPipe;
    si.hStdOutput = hWritePipe;

    wchar_t cmdLine[] = L"Alfavit.exe";

    if (!CreateProcess(nullptr, cmdLine, nullptr, nullptr, TRUE, CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &pi)) {
        ErrorExit("Failed to create Alfavit process.");
    }

    // Ввод данных
    std::cout << "Enter the size of the array: ";
    int size;
    std::cin >> size;

    std::vector<char> array(size);
    std::cout << "Enter array elements (char): ";
    for (int i = 0; i < size; ++i) {
        std::cin >> array[i];
    }

    std::cout << "Enter a number N: ";
    int n;
    std::cin >> n;

    // Передача данных в Alfavit
    DWORD written;
    WriteFile(hWritePipe, &size, sizeof(size), &written, nullptr); // Отправляем размер массива
    WriteFile(hWritePipe, &n, sizeof(n), &written, nullptr);       // Отправляем число N

    for (char ch : array) {
        WriteFile(hWritePipe, &ch, sizeof(ch), &written, nullptr); // Отправляем элементы массива
    }
    ReleaseSemaphore(hServerSemaphore, 1, nullptr);           // Уведомляем alfavit

    WaitForSingleObject(hServerSemaphore, INFINITE);

    // Чтение обработанного массива
    std::cout << "Processed characters received from Alfavit: ";
    for (int i = 0; i < size; ++i) {
        char processedChar;
        DWORD read;
        ReadFile(hReadPipe, &processedChar, sizeof(processedChar), &read, nullptr);
        std::cout << processedChar << " ";
    }

    std::cout << std::endl;
    // Закрытие ресурсов
    CloseHandle(hReadPipe);
    CloseHandle(hWritePipe);
    CloseHandle(hServerSemaphore);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return 0;
}
