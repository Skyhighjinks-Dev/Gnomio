#include "../include/Processes/MCProcess.h"
#include "../include/Processes/MCProcessManager.h"
#include "../include/Processes/OCR/OCREngine.h"

#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <iostream>
#include <vector>


// Function to get all process IDs of javaw.exe
std::vector<DWORD> GetProcessIDsByName(const std::wstring& processName) {
    std::vector<DWORD> processIDs;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to take a snapshot of the processes." << std::endl;
        return processIDs;
    }

    PROCESSENTRY32W processEntry;
    processEntry.dwSize = sizeof(PROCESSENTRY32W);

    if (Process32FirstW(snapshot, &processEntry)) {
        do {
            // Compare the process name with javaw.exe
            if (_wcsicmp(processEntry.szExeFile, processName.c_str()) == 0) {
                processIDs.push_back(processEntry.th32ProcessID);
            }
        } while (Process32NextW(snapshot, &processEntry));
    } else {
        std::cerr << "Failed to retrieve the first process." << std::endl;
    }

    CloseHandle(snapshot);
    return processIDs;
}


int main() {
    std::wstring processName = L"javaw.exe";
    std::vector<DWORD> processIDs = GetProcessIDsByName(processName);

    MCProcessManager mcManager;
    for(DWORD dWord : processIDs) {
        mcManager.AddProcess(std::make_unique<MCProcess>(dWord));
    }
    HWND& hwnd = mcManager.GetProcessPtr(0)->GetProcessHandle();

    RECT rect = {0, 0, 1300, 800}; // Desired client area size (width = 1300, height = 800)
    // Adjust the rectangle to take the non-client area (borders, title bar, etc.) into account
    AdjustWindowRect(&rect, GetWindowLong(hwnd, GWL_STYLE), FALSE);
    // Set the window size to include the non-client area
    SetWindowPos(hwnd, HWND_TOP, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER | SWP_NOMOVE);

    OCREngine engine;

    mcManager.GetProcessPtr(0)->ConnectToServer();
    mcManager.GetProcessPtr(0)->ConnectToSkyblock();
    mcManager.GetProcessPtr(0)->StartMelonScript(engine);

    std::cout << "\nDone" << std::endl;

    int z;
    std::cin >> z;

    return 0;
}
