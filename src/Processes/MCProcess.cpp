//
// Created by amdro on 26/08/2024.
//

#include "../../include/Processes/MCProcess.h"

MCProcess::MCProcess(DWORD nProcessID) {
    this->ProcessID = nProcessID;

    RetrieveAndAssignData();
}

void MCProcess::RetrieveAndAssignData() {
    struct HandleData {
        DWORD pid;
        HWND hWnd;
    };

    HandleData data = { this->ProcessID, nullptr };
    auto EnumWindowsCallback = [](HWND hWnd, LPARAM lParam) -> BOOL {
        HandleData& data = *reinterpret_cast<HandleData*>(lParam);
        DWORD windowPid = 0;
        GetWindowThreadProcessId(hWnd, &windowPid);
        if (windowPid == data.pid) {
            data.hWnd = hWnd;
            return FALSE;  // Stop enumeration
        }
        return TRUE;  // Continue enumeration
    };
    EnumWindows(EnumWindowsCallback, reinterpret_cast<LPARAM>(&data));

    this->Process = data.hWnd;
}


// ReSharper disable once CppMemberFunctionMayBeConst
void MCProcess::SendKey(WORD nKey) {
    SetForegroundWindow(this->Process);

    INPUT input = {0};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = nKey;

    SendInput(1, &input, sizeof(INPUT));
    input.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
}

void MCProcess::SendKeys(std::vector<WORD> nKeys, const int nTimeout) {
    for(WORD word : nKeys) {
        std::cout << word << std::endl;
        SendKey(word);
        std::this_thread::sleep_for(std::chrono::milliseconds(nTimeout));
    }
}

void MCProcess::SendKeys(const std::string& nMessage, const int nTimeout) {
    std::vector<WORD> keys;
    for (char buff : nMessage) {
        keys.push_back(static_cast<WORD>(buff));
    }

    SendKeys(keys, nTimeout);
}

HWND& MCProcess::GetProcessHandle() {
    return this->Process;
}

int MCProcess::GetDpiForWindow() const {
    HMONITOR hMonitor = MonitorFromWindow(this->Process, MONITOR_DEFAULTTONEAREST);
    UINT dpiX = 96, dpiY = 96; // Default DPI is 96 (100%)
    if (SUCCEEDED(GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY))) {
        return dpiX; // Assuming square pixels (dpiX == dpiY)
    }
    return 96; // Default DPI if the call fails
}


void MCProcess::ClickCoordinates(const int &x, const int &y) const {
    if (this->Process == NULL) {
        std::cout << "Invalid window handle." << std::endl;
        return;
    }

    int ogX = x;
    int ogY = y;


    // Bring the window to the foreground
    SetForegroundWindow(this->Process);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Confirm that the window is in the foreground
    if (GetForegroundWindow() != this->Process) {
        std::cout << "Failed to bring the window to the foreground." << std::endl;
        return;
    }

    POINT point;
    point.x = x;
    point.y = y;

    std::cout << "Adjusted click coordinates:\nX - " << point.x << "\nY - " << point.y << std::endl;

    // Convert client coordinates to screen coordinates
    ClientToScreen(this->Process, &point);

    // Set the cursor position
    SetCursorPos(point.x , point.y);

    LPARAM lParam = MAKELPARAM(point.x, point.y);

    // Attempt mouse click using SendInput or another method compatible with Java
    INPUT input[2] = {};
    input[0].type = INPUT_MOUSE;
    input[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    input[1].type = INPUT_MOUSE;
    input[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;

    SendInput(2, input, sizeof(INPUT));

    std::cout << "Simulated mouse click at " << point.x << ", " << point.y << std::endl;
}
