#include "os/command.h"

#include <iostream>
#include <locale>

#include "tchar.h"
#include "windows.h"


bool execute_command_list(std::ostream &outstream, const std::string &command, std::vector<std::string> args)
{
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );
    HANDLE cout_r = NULL;
    HANDLE cout_w = NULL;

    SECURITY_ATTRIBUTES sec_a;
    memset(&sec_a, 1, sizeof sec_a);

    sec_a.nLength = sizeof(SECURITY_ATTRIBUTES);
    sec_a.lpSecurityDescriptor = NULL;

    CreatePipe(&cout_r, &cout_w, &sec_a, 0);
    SetHandleInformation(cout_r, HANDLE_FLAG_INHERIT, 0);
    //startInf.cb = sizeof(STARTUPINFO);
    si.hStdOutput = cout_w;
    si.hStdError = cout_w;
    si.dwFlags |= STARTF_USESTDHANDLES;
    std::string commandLine =command;
    for (auto& arg : args)
    {
        commandLine+=" "+arg;
    }
    //
    // Start the child process.
    if( !CreateProcessA(NULL,   // No module name (use command line)
        const_cast<LPSTR>(commandLine.c_str()),        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        TRUE,          // Set handle inheritance to FALSE
        NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW | CREATE_NEW_PROCESS_GROUP,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
    )
    {

        printf( "CreateProcess failed (%ld).\n", GetLastError() );
        return false;
    }

    // Wait until child process exits.
    WaitForSingleObject( pi.hProcess, INFINITE );
    // Close process and thread handles.
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
    CloseHandle( cout_w );


    CHAR chBuf[4096];
    DWORD dwRead;
    for (;;)
    {

        BOOL bSuccess = ReadFile(cout_r, chBuf, 4096, &dwRead, NULL);
        if (!bSuccess || dwRead == 0) break;


        std::string s(chBuf, dwRead);
        outstream<<s;
        outstream.flush();
    }
    return true;
}
