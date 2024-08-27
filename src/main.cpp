#include "Processes/OCR/OCREngine.h"
#include "Processes/MCProcessManager.h"
#include "Processes/MCProcess.h"

int main() {
    DWORD procID[] = { 24872 };
    MCProcessManager mcManager;

    for(DWORD dWord : procID) {
        mcManager.AddProcess(std::make_unique<MCProcess>(dWord));
    }

    HWND& hwnd = mcManager.GetProcessPtr(0)->GetProcessHandle();

    SetWindowPos(hwnd, HWND_TOP, 0, 0, 1300, 800, SWP_NOZORDER | SWP_NOMOVE);

    OCREngine eng;
    if(eng.ProcessOCR(hwnd) == 0) {
        std::cout << "OCR Success" << std::endl;
    }
    else {
        std::cout << "OCR Failed" << std::endl;
    }

    int z;
    std::cin >> z;

    return 0;
}
