#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define UNICODE

#include <windows.h>
#include <iostream>

int main()
{
    // make all our handles inheritable
    SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };

    // ┌───────────┐     ┌────────────┐    ┌───────────┐
    // │ Parent    │ --> │ child stdin│ -->│ cmd.exe   │
    // └───────────┘     └────────────┘    └───────────┘
    HANDLE childStdin_Read,  parentStdin_Write;
    // ┌───────────┐     ┌────────────┐    ┌───────────┐
    // │ cmd.exe   │ --> │ child stdout│-->│ Parent    │
    // └───────────┘     └────────────┘    └───────────┘
    HANDLE parentStdout_Read, childStdout_Write;

    // create the two pipes
    if (!CreatePipe(&childStdin_Read,  &parentStdin_Write,  &sa, 0) ||
        !CreatePipe(&parentStdout_Read, &childStdout_Write, &sa, 0))
    {
        std::cerr << "CreatePipe failed: " << GetLastError() << "\n";
        return 1;
    }

    // build the ConPTY
    HPCON hPC;
    COORD size{ 80, 25 };
    if (FAILED(CreatePseudoConsole(size, childStdin_Read, childStdout_Write, 0, &hPC)))
    {
        std::cerr << "CreatePseudoConsole failed\n";
        return 1;
    }

    // set up STARTUPINFOEX with the pseudo console handle
    SIZE_T attrSize = 0;
    STARTUPINFOEXW si;
    ZeroMemory(&si, sizeof(si));
    si.StartupInfo.cb = sizeof(si);
    InitializeProcThreadAttributeList(NULL, 1, 0, &attrSize);
    si.lpAttributeList = (LPPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(GetProcessHeap(), 0, attrSize);
    InitializeProcThreadAttributeList(si.lpAttributeList, 1, 0, &attrSize);
    UpdateProcThreadAttribute(si.lpAttributeList, 0, PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE, hPC, sizeof(hPC), NULL, NULL);

    // spawn cmd.exe with handle inheritance on
    PROCESS_INFORMATION pi{};
    if (!CreateProcessW(
            L"C:\\Windows\\System32\\cmd.exe",
            NULL,       // no cmdline args
            NULL, NULL, // no extra security
            TRUE,       // inherit handles!
            EXTENDED_STARTUPINFO_PRESENT,
            NULL, NULL, // inherit CWD + environment
            &si.StartupInfo,
            &pi))
    {
        std::cerr << "CreateProcessW failed: " << GetLastError() << "\n";
        return 1;
    }

    // we no longer need these ends in the parent
    CloseHandle(childStdin_Read);
    CloseHandle(childStdout_Write);

    // send "dir\r\n" *into* the child’s STDIN
    const char cmd[] = "cd\r\n";
    DWORD written;
    if (!WriteFile(parentStdin_Write, cmd, (DWORD)strlen(cmd), &written, NULL))
    {
        std::cerr << "WriteFile to child stdin failed: " << GetLastError() << "\n";
    }
    // close the write-end if you’re done sending commands
    // this will cause the child to see EOF if it reads more
    CloseHandle(parentStdin_Write);

    // read the child’s STDOUT
    CHAR buf[256];
    DWORD read;
    while (ReadFile(parentStdout_Read, buf, sizeof(buf) - 1, &read, NULL) && read)
    {
        buf[read] = 0;
        std::cout << buf;
    }

    // wait, clean up
    WaitForSingleObject(pi.hProcess, INFINITE);
    ClosePseudoConsole(hPC);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    DeleteProcThreadAttributeList(si.lpAttributeList);
    HeapFree(GetProcessHeap(), 0, si.lpAttributeList);
    CloseHandle(parentStdin_Write);
    CloseHandle(parentStdout_Read);

    return 0;
}
