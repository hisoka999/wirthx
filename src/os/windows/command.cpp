#include "os/command.h"

#include <iostream>
#include <locale>

#include "tchar.h"
#include "windows.h"


bool execute_command_list(std::ostream &outstream, std::ostream &errorStream, const std::string &command,
                          std::vector<std::string> args)
{
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    HANDLE cout_r = nullptr;
    HANDLE cout_w = nullptr;

    HANDLE cerr_r = nullptr;
    HANDLE cerr_w = nullptr;
    SECURITY_ATTRIBUTES sec_a;
    memset(&sec_a, 1, sizeof sec_a);

    sec_a.nLength = sizeof(SECURITY_ATTRIBUTES);
    sec_a.lpSecurityDescriptor = nullptr;

    CreatePipe(&cout_r, &cout_w, &sec_a, 0);
    CreatePipe(&cerr_r, &cerr_w, &sec_a, 0);

    SetHandleInformation(cout_r, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(cerr_r, HANDLE_FLAG_INHERIT, 0);

    // startInf.cb = sizeof(STARTUPINFO);
    si.hStdOutput = cout_w;
    si.hStdError = cerr_w;
    si.dwFlags |= STARTF_USESTDHANDLES;
    std::string commandLine = command;
    for (auto &arg: args)
    {
        commandLine += " " + arg;
    }
    //
    // Start the child process.
    if (!CreateProcessA(nullptr, // No module name (use command line)
                        const_cast<LPSTR>(commandLine.c_str()), // Command line
                        nullptr, // Process handle not inheritable
                        nullptr, // Thread handle not inheritable
                        TRUE, // Set handle inheritance to FALSE
                        NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW | CREATE_NEW_PROCESS_GROUP, // No creation flags
                        nullptr, // Use parent's environment block
                        nullptr, // Use parent's starting directory
                        &si, // Pointer to STARTUPINFO structure
                        &pi) // Pointer to PROCESS_INFORMATION structure
    )
    {

        printf("CreateProcess failed (%ld).\n", GetLastError());
        return false;
    }

    // Wait until child process exits.
    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exit_code;
    if (FALSE == GetExitCodeProcess(pi.hProcess, &exit_code))
    {
        std::cerr << "GetExitCodeProcess() failure: " << GetLastError() << "\n";
    }
    else if (STILL_ACTIVE == exit_code)
    {
        std::cout << "Still running\n";
    }
    else if (exit_code != 0)
    {
        std::cerr << "Process terminated with exit code " << exit_code << "\n";
    }

    // Close process and thread handles.
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(cout_w);
    CloseHandle(cerr_w);


    CHAR chBuf[4096];
    DWORD dwRead;
    for (;;)
    {

        BOOL bSuccess = ReadFile(cout_r, chBuf, 4096, &dwRead, NULL);
        if (!bSuccess || dwRead == 0)
            break;


        std::string s(chBuf, dwRead);
        outstream << s;
        outstream.flush();
    }

    for (;;)
    {

        BOOL bSuccess = ReadFile(cerr_r, chBuf, 4096, &dwRead, NULL);
        if (!bSuccess || dwRead == 0)
            break;


        std::string s(chBuf, dwRead);
        errorStream << s;
        errorStream.flush();
    }

    return exit_code == 0;
}
