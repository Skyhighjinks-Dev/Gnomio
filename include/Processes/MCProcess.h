//
// Created by amdro on 26/08/2024.
//

#pragma once

#include <chrono>
#include <thread>
#include <vector>
#include <windows.h>

#include <iostream>

class MCProcess {
private:
    DWORD ProcessID;
    HWND Process;

public:
    MCProcess(DWORD nProcessID);
    void SendKeys(const std::string& nMessage, const int nTimeout);
    HWND& GetProcessHandle();

private:
    void RetrieveAndAssignData();
    void SendKey(WORD nKey);
    void SendKeys(std::vector<WORD> nKeys, const int nTimeout);
};