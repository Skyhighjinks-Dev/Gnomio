//
// Created by amdro on 26/08/2024.
//

#pragma once

#include <chrono>
#include <thread>
#include <vector>
#include <windows.h>
#include <ShellScalingAPI.h>
#include <map>

#include <iostream>

class MCProcess {
private:
    DWORD ProcessID;
    HWND Process;

public:
    MCProcess(DWORD nProcessID);
    void SendKeys(const std::string& nMessage, int nTimeout);
    void ClickCoordinates(const int& x, const int& y) const;
    HWND& GetProcessHandle();

private:
    void RetrieveAndAssignData();
    void SendKey(WORD nKey);
    void SendKeys(std::vector<WORD> nKeys, const int nTimeout);
    int GetDpiForWindow() const;
};