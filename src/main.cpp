#include "../include/Processes/MCProcess.h"
#include "../include/Processes/MCProcessManager.h"
#include "../include/Processes/OCR/OCREngine.h"

int main() {
    DWORD procID[] = { 19604 };
    MCProcessManager mcManager;

    for(DWORD dWord : procID) {
        mcManager.AddProcess(std::make_unique<MCProcess>(dWord));
    }

    HWND& hwnd = mcManager.GetProcessPtr(0)->GetProcessHandle();

    RECT rect = {0, 0, 1300, 800}; // Desired client area size (width = 1300, height = 800)

    // Adjust the rectangle to take the non-client area (borders, title bar, etc.) into account
    AdjustWindowRect(&rect, GetWindowLong(hwnd, GWL_STYLE), FALSE);

    // Set the window size to include the non-client area
    SetWindowPos(hwnd, HWND_TOP, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER | SWP_NOMOVE);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    OCREngine eng;
    std::vector<OCRResult> result = eng.ProcessOCR(hwnd);


    for(OCRResult ocr : result) {
        if(ocr.text.compare("Anti AFK") == 0)
            mcManager.GetProcessPtr(0)->ClickCoordinates(ocr.boundingBox.x, ocr.boundingBox.y);
    }

    int z;
    std::cin >> z;

    return 0;
}
