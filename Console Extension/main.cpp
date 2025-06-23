#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define UNICODE

#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <tchar.h>

#pragma comment(lib, "kernel32.lib")

int main() {
    SECURITY_ATTRIBUTES saAttr = {};
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE; // Make the handles inheritable
    saAttr.lpSecurityDescriptor = NULL;
    // Step 1: Create pipes for communication
    HANDLE hPipePTYIn = nullptr, hPipePTYOut = nullptr;
    HANDLE hPipeTerminalIn = nullptr, hPipeTerminalOut = nullptr;

    if (!CreatePipe(&hPipePTYIn, &hPipeTerminalOut, &saAttr, 0)) {
        std::cerr << "CreatePipe 1 failed.\n";
        return 1;
    }
    if (!CreatePipe(&hPipeTerminalIn, &hPipePTYOut, &saAttr, 0)) {
        std::cerr << "CreatePipe 2 failed.\n";
        return 1;
    }

    // Step 2: Create the pseudoconsole
    HPCON hPC;
    COORD consoleSize = { 80, 25 };
    HRESULT hr = CreatePseudoConsole(consoleSize, hPipePTYIn, hPipePTYOut, 0, &hPC);
    if (FAILED(hr)) {
        std::cerr << "CreatePseudoConsole failed. HRESULT = 0x" << std::hex << hr << "\n";
        return 1;
    }


    // Step 3: Set up STARTUPINFOEX with ConPTY handle
    STARTUPINFOEXW siEx = { 0 };
    siEx.StartupInfo.cb = sizeof(STARTUPINFOEXW);

    SIZE_T attrListSize = 0;
    InitializeProcThreadAttributeList(NULL, 1, 0, &attrListSize);
    siEx.lpAttributeList = (LPPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(GetProcessHeap(), 0, attrListSize);
    InitializeProcThreadAttributeList(siEx.lpAttributeList, 1, 0, &attrListSize);

    UpdateProcThreadAttribute(
        siEx.lpAttributeList,
        0,
        PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE,
        hPC,
        sizeof(HPCON),
        NULL,
        NULL
    );

    // Step 4: Create the child process (cmd.exe)
    PROCESS_INFORMATION pi = { 0 };
    if (!CreateProcessW(
        L"C:\\Windows\\System32\\cmd.exe", // or L"powershell.exe"
        NULL, NULL, NULL, TRUE,
        EXTENDED_STARTUPINFO_PRESENT,
        NULL, NULL,
        &siEx.StartupInfo, &pi)) {
        std::cerr << "CreateProcessW failed. Error: " << GetLastError() << "\n";
        return 1;
    }
    Sleep(100);
    const char* init = "cd\r\n"; // You can use "cd\r\n" or any other command
    DWORD written = 0;
    if (!WriteFile(hPipeTerminalIn, init, strlen(init), &written, NULL)) {
        std::cerr << "WriteFile to cmd.exe failed: " << GetLastError() << "\n";
    } else {
        std::cout << "[+] Sent command to cmd.exe: " << init << "\n";
    }

    HANDLE hOutputThread = CreateThread(NULL, 0, [](LPVOID param) -> DWORD {
    HANDLE hOutput = (HANDLE)param;
    char buffer[256];
    DWORD bytesRead;

    while (true) {
        BOOL success = ReadFile(hOutput, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
        if (!success || bytesRead == 0) {
            std::cerr << "[!] ReadFile failed or no bytes. Exiting output thread.\n";
            std::cerr << "bytesRead: " << bytesRead << "\n";
            break;
        }

        buffer[bytesRead] = '\0'; // Null-terminate
        std::cout << buffer << std::flush; // Ensure output appears immediately
    }

    return 0;
}, hPipeTerminalOut, 0, NULL);

    // Step 5.5: Read keyboard input and write to the pseudoconsole
    HANDLE hInputThread = CreateThread(NULL, 0, [](LPVOID param) -> DWORD {
        HANDLE hInput = (HANDLE)param;
        DWORD written;
        char ch;
        while ((ch = getchar()) != EOF) {
            WriteFile(hInput, &ch, 1, &written, NULL);
        }
        return 0;
    }, hPipeTerminalIn, 0, NULL);
    WaitForSingleObject(hOutputThread, INFINITE);
    WaitForSingleObject(hInputThread, INFINITE);

    // Step 6: Cleanup
    ClosePseudoConsole(hPC);
    CloseHandle(hOutputThread);
    CloseHandle(hInputThread);
    DeleteProcThreadAttributeList(siEx.lpAttributeList);
    HeapFree(GetProcessHeap(), 0, siEx.lpAttributeList);
    CloseHandle(hPipePTYIn);
    CloseHandle(hPipePTYOut);
    CloseHandle(hPipeTerminalIn);
    CloseHandle(hPipeTerminalOut);

    return 0;
}
