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