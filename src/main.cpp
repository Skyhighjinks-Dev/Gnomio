#include <vector>
#include <iostream>
#include <windows.h>

#include "Processes/MCProcess.h"

int main() {
    DWORD procID = 19856;
    MCProcess process(procID);

    process.SendKeys("BTHELLO WORLD!", 500);

    int x;
    std::cin >> x;

}