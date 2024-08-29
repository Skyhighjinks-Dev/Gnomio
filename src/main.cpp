#include "../include/Processes/MCProcess.h"
#include "../include/Processes/MCProcessManager.h"
#include "../include/Processes/OCR/OCREngine.h"

int main() {
    DWORD procID[] = { 20864 };
    MCProcessManager mcManager;

    for(DWORD dWord : procID) {
        mcManager.AddProcess(std::make_unique<MCProcess>(dWord));
    }

    HWND& hwnd = mcManager.GetProcessPtr(0)->GetProcessHandle();

    SetWindowPos(hwnd, HWND_TOP, 0, 0, 1300, 800, SWP_NOZORDER | SWP_NOMOVE);

    OCREngine eng;
    std::vector<OCRResult> result = eng.ProcessOCR(hwnd);


    for(OCRResult ocr : result) {
        if(ocr.text.compare("Pumpkin/Melon") == 0)
            mcManager.GetProcessPtr(0)->ClickCoordinates(ocr.boundingBox.x, ocr.boundingBox.y);
    }

    int z;
    std::cin >> z;

    return 0;
}
