// OCREngine.cpp
#include "../../../include/Processes/OCR/OCREngine.h"
#include <algorithm>



std::vector<OCRResult> OCREngine::ProcessOCR(const HWND* nHWND, const std::string nTextToFind, const OCR_TYPE nType) {
    // Capture window image
    cv::Mat capturedImage = CaptureWindow(nHWND);
    if (capturedImage.empty()) {
        throw std::runtime_error("Failed to capture window image.");
    }

    // Get areas of interest
    std::vector<RECT> areasOfInterest = GetAreasOfInterest(nType);

    for(RECT _rec : areasOfInterest) {
       std::cout << std::format("LEFT: {}, RIGHT: {}, TOP: {}, BOTTOM: {}", _rec.left, _rec.right, _rec.top, _rec.bottom);
    }

    // Process image via OCR
    std::vector<OCRResult> ocrResults = ProcessImageViaOCR(capturedImage, areasOfInterest, false);

    // Find specific text within OCR results
    return FindText(ocrResults, nTextToFind);
}

std::vector<OCRResult> OCREngine::ProcessOCR(const HWND* nHWND, const OCR_TYPE nType) {
    // Capture window image
    cv::Mat capturedImage = CaptureWindow(nHWND);
    if (capturedImage.empty()) {
        throw std::runtime_error("Failed to capture window image.");
    }

    // Get areas of interest
    std::vector<RECT> areasOfInterest = GetAreasOfInterest(nType);

    // Process image via OCR and capture all text
    return ProcessImageViaOCR(capturedImage, areasOfInterest, true);
}

std::vector<RECT> OCREngine::GetAreasOfInterest(OCR_TYPE nType) {
    // Define areas of interest based on the OCR type
    std::vector<RECT> areas;

    // Check for each flag using bitwise AND
    if (nType & TAUNAHI_LEFT_MENU) {
        areas.push_back({140, 130, 280, 420});
    }

    if (nType & TAUNAHI_CENTER_MENU) {
        areas.push_back({500, 120, 1000, 740});
    }

    if (nType & MC_MAIN_MENU_CENTER) {
        areas.push_back({ 430, 290, 860, 520 });
    }

    if (nType & MC_MULTI_PLAY_MENU_BOTTOM) {
        areas.push_back({ 330, 690, 970, 790 });
    }

    if (nType & MC_DIR_CON_BUTTON) {
        areas.push_back({ 450, 400, 860, 510 });
    }



    return areas;
}

std::vector<OCRResult> OCREngine::ProcessImageViaOCR(cv::Mat& nImage, std::vector<RECT> nAOI, bool nCaptureAll) {
    std::vector<OCRResult> results;
    tesseract::TessBaseAPI ocr;

    // Initialize Tesseract OCR engine
    if (ocr.Init(nullptr, "eng", tesseract::OEM_LSTM_ONLY)) {
        throw std::runtime_error("Could not initialize Tesseract.");
    }

    // Set OCR parameters
    ocr.SetPageSegMode(tesseract::PSM_AUTO); // Adjust segmentation mode as needed

    // Draw AOIs on the image to visualize before OCR processing
    for (const RECT& rect : nAOI) {
        // Calculate ROI coordinates and size
        int roiX = std::max(0, static_cast<int>(rect.left));
        int roiY = std::max(0, static_cast<int>(rect.top));
        int roiWidth = std::min(static_cast<int>(rect.right - rect.left), nImage.cols - roiX);
        int roiHeight = std::min(static_cast<int>(rect.bottom - rect.top), nImage.rows - roiY);

        // Check if the ROI is valid and within bounds
        if (roiWidth > 0 && roiHeight > 0) {
            // Draw AOI rectangle in blue
            cv::rectangle(nImage, cv::Rect(roiX, roiY, roiWidth, roiHeight), cv::Scalar(255, 0, 0), 2);
        } else {
            std::cout << "Invalid AOI dimensions, skipping this rectangle." << std::endl;
        }
    }

    std::vector<OCRLine> lines; // Store detected lines

    // Loop through each area of interest to perform OCR
    for (const RECT& rect : nAOI) {
        // Calculate ROI coordinates and size
        int roiX = std::max(0, static_cast<int>(rect.left));
        int roiY = std::max(0, static_cast<int>(rect.top));
        int roiWidth = std::min(static_cast<int>(rect.right - rect.left), nImage.cols - roiX);
        int roiHeight = std::min(static_cast<int>(rect.bottom - rect.top), nImage.rows - roiY);

        // Check if the ROI is valid and within bounds
        if (roiWidth <= 0 || roiHeight <= 0) {
            std::cout << "Invalid ROI, skipping OCR for this region." << std::endl;
            continue; // Skip invalid ROIs
        }

        // Create a rectangle for the ROI
        cv::Rect roi(roiX, roiY, roiWidth, roiHeight);
        cv::Mat croppedImage = nImage(roi);

        // Preprocess the cropped image for better OCR results
        cv::Mat gray, binary;
        cv::cvtColor(croppedImage, gray, cv::COLOR_BGR2GRAY); // Convert to grayscale
        cv::threshold(gray, binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU); // Binarize the image

        // Set preprocessed image to Tesseract
        ocr.SetImage(binary.data, binary.cols, binary.rows, 1, binary.step);
        ocr.Recognize(0);

        // Get OCR results
        tesseract::ResultIterator* ri = ocr.GetIterator();
        tesseract::PageIteratorLevel level = tesseract::RIL_WORD;

        if (ri != nullptr) {
            std::vector<OCRWord> currentLine;
            int prevBottom = 0;

            do {
                const char* word = ri->GetUTF8Text(level);
                if (word != nullptr) {
                    int left, top, right, bottom;
                    ri->BoundingBox(level, &left, &top, &right, &bottom);

                    // Adjust bounding box with offset
                    cv::Rect adjustedBoundingBox(
                        roiX + left,
                        roiY + top,
                        right - left,
                        bottom - top
                    );

                    // Group words into lines based on vertical proximity
                    if (!currentLine.empty() && std::abs(top - prevBottom) > 10) {
                        // Store current line as a detected line
                        lines.push_back({currentLine});
                        currentLine.clear();
                    }

                    currentLine.push_back({std::string(word), adjustedBoundingBox});
                    prevBottom = bottom;

                    delete[] word;
                }
            } while (ri->Next(level));

            // Push the last line if it exists
            if (!currentLine.empty()) {
                lines.push_back({currentLine});
            }
        } else {
            std::cout << "No text detected in this AOI." << std::endl;
        }
    }

    ocr.End();

    // Merge lines into paragraphs based on vertical and horizontal proximity
    std::vector<OCRParagraph> paragraphs = MergeLinesIntoParagraphs(lines);

    return ConvertParagraphsToOCRResults(paragraphs);
}


// Convert paragraphs back into OCRResult format for returning
std::vector<OCRResult> OCREngine::ConvertParagraphsToOCRResults(const std::vector<OCRParagraph>& paragraphs) {
    std::vector<OCRResult> results;
    for (const auto& paragraph : paragraphs) {
        std::string fullText;
        cv::Rect boundingBox;
        for (const auto& line : paragraph.Lines) {
            for (const auto& word : line.Words) {
                fullText += word.Text + " ";
                boundingBox |= word.BoundingBox; // Merge bounding boxes
            }
        }
        results.push_back({fullText, boundingBox});
    }
    return results;
}

// Helper function to merge lines into paragraphs
std::vector<OCRParagraph> OCREngine::MergeLinesIntoParagraphs(const std::vector<OCRLine>& lines) {
    std::vector<OCRParagraph> paragraphs;
    OCRParagraph currentParagraph;

    for (const auto& line : lines) {
        if (!currentParagraph.Lines.empty() && !IsLineProximal(currentParagraph.Lines.back(), line)) {
            paragraphs.push_back(currentParagraph);
            currentParagraph = OCRParagraph();
        }
        currentParagraph.Lines.push_back(line);
    }

    if (!currentParagraph.Lines.empty()) {
        paragraphs.push_back(currentParagraph);
    }

    return paragraphs;
}

// Helper function to check if two lines are vertically close enough to be grouped into a paragraph
bool OCREngine::IsLineProximal(const OCRLine& line1, const OCRLine& line2) {
    // Check if line2's top is within a certain threshold below line1's bottom
    return (std::abs(line2.Top() - line1.Bottom()) < 20); // Example threshold
}

cv::Mat OCREngine::CaptureWindow(const HWND* hwnd) {
    HDC hwindowDC = GetDC(*hwnd);
    HDC hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
    RECT windowRect;
    GetClientRect(*hwnd, &windowRect);

    int width = windowRect.right - windowRect.left;
    int height = windowRect.bottom - windowRect.top;

    HBITMAP hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
    SelectObject(hwindowCompatibleDC, hbwindow);

    BitBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, 0, 0, SRCCOPY);
    BITMAPINFOHEADER bi = {sizeof(BITMAPINFOHEADER), width, -height, 1, 32, BI_RGB, 0, 0, 0, 0, 0};
    
    cv::Mat image(height, width, CV_8UC4);
    GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, image.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    DeleteObject(hbwindow);
    DeleteDC(hwindowCompatibleDC);
    ReleaseDC(*hwnd, hwindowDC);
    return image;
}

std::vector<OCRResult> OCREngine::FindText(const std::vector<OCRResult>& results, const std::string& textToFind) {
    std::vector<OCRResult> filteredResults;
    for (const auto& result : results) {
        if (result.Text.find(textToFind) != std::string::npos) {
            filteredResults.push_back(result);
        }
    }
    return filteredResults;
}