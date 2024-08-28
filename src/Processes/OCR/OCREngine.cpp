#include "../../../include/Processes/OCR/OCREngine.h"

bool OCREngine::AreBoxesClose(const cv::Rect& a, const cv::Rect& b) {
    int distanceThreshold = 20; // Adjust this threshold as necessary
    return (abs(a.y - b.y) < distanceThreshold) && (abs(a.x + a.width - b.x) < distanceThreshold);
}

std::vector<OCREngine::OCRResult> OCREngine::PerformOCR(const cv::Mat& nImg) {
    std::vector<OCRResult> results;

    // Preprocess the image
    cv::Mat processedImg;
    cv::cvtColor(nImg, processedImg, cv::COLOR_BGR2GRAY); // Convert to grayscale
    cv::GaussianBlur(processedImg, processedImg, cv::Size(3, 3), 0); // Apply slight blur to reduce noise
    cv::threshold(processedImg, processedImg, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU); // Binarization

    // Initialize Tesseract API
    tesseract::TessBaseAPI *ocr = new tesseract::TessBaseAPI();
    if (ocr->Init(NULL, "eng")) {
        std::cerr << "Could not initialize tesseract." << std::endl;
        return results;
    }

    // Set OCR recognition mode
    ocr->SetPageSegMode(tesseract::PSM_AUTO); // or PSM_SINGLE_BLOCK, PSM_SINGLE_LINE depending on the structure

    // Set the whitelist for characters to a-z, A-Z, and space
    ocr->SetVariable("tessedit_char_whitelist", "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ/ ");

    // Set the image for Tesseract
    ocr->SetImage(processedImg.data, processedImg.cols, processedImg.rows, 1, processedImg.step); // Note: Pass the grayscale image

    // Get OCR result
    ocr->Recognize(0);
    tesseract::ResultIterator* ri = ocr->GetIterator();
    tesseract::PageIteratorLevel level = tesseract::RIL_WORD;

    if (ri != 0) {
        do {
            const char* word = ri->GetUTF8Text(level);
            int x1, y1, x2, y2;
            ri->BoundingBox(level, &x1, &y1, &x2, &y2);

            if (word != nullptr) {
                OCRResult result;
                result.text = std::string(word);
                result.boundingBox = cv::Rect(cv::Point(x1, y1), cv::Point(x2, y2));
                results.push_back(result);

                delete[] word;
            }
        } while (ri->Next(level));
    }

    // Cleanup
    ocr->End();

    return results;
}


void OCREngine::MergeOCRResults(std::vector<OCRResult>& nResults) {
    std::vector<OCRResult> mergedResults;

    // Sort the results based on the y-coordinate (top to bottom) and then x-coordinate (left to right)
    std::sort(nResults.begin(), nResults.end(), [](const OCRResult& a, const OCRResult& b) {
        if (a.boundingBox.y == b.boundingBox.y) {
            return a.boundingBox.x < b.boundingBox.x;
        }
        return a.boundingBox.y < b.boundingBox.y;
    });

    for (auto& result : nResults) {
        bool merged = false;
        for (auto& mergedResult : mergedResults) {
            if (AreBoxesClose(mergedResult.boundingBox, result.boundingBox)) {
                // Merge text and bounding boxes
                mergedResult.text += " " + result.text;
                mergedResult.boundingBox = mergedResult.boundingBox | result.boundingBox;
                merged = true;
                break;
            }
        }
        if (!merged) {
            mergedResults.push_back(result);
        }
    }

    nResults = mergedResults;
}

void OCREngine::ProcessROIAndEntireImage(const cv::Mat& nImg, const cv::Rect& nROI, const std::string& nImgName) {
    cv::Mat imgCopy = nImg.clone();

    // Ensure the ROI is within the image bounds
    cv::Rect boundedROI = nROI & cv::Rect(0, 0, nImg.cols, nImg.rows);

    // Process the main region of interest (ROI)
    cv::Mat roiImg = nImg(boundedROI);
    std::vector<OCRResult> roiResults = PerformOCR(roiImg);
    MergeOCRResults(roiResults);

    for (const auto& result : roiResults) {
        cv::Rect adjustedBox = result.boundingBox;
        adjustedBox.x += boundedROI.x;
        adjustedBox.y += boundedROI.y;

        cv::rectangle(imgCopy, adjustedBox, cv::Scalar(0, 255, 0), 2);
        cv::putText(imgCopy, result.text, cv::Point(adjustedBox.x, adjustedBox.y - 5),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);
    }

    // Define additional ROIs with boundary checks
    cv::Rect additionalROI(320, 140, 800, 600);
    additionalROI &= cv::Rect(0, 0, nImg.cols, nImg.rows); // Ensure within bounds
    cv::Mat additionalRoiImg = nImg(additionalROI);

    std::vector<OCRResult> additionalResults = PerformOCR(additionalRoiImg);
    MergeOCRResults(additionalResults);

    for (const auto& result : additionalResults) {
        cv::Rect adjustedBox = result.boundingBox;
        adjustedBox.x += additionalROI.x;
        adjustedBox.y += additionalROI.y;

        cv::rectangle(imgCopy, adjustedBox, cv::Scalar(255, 0, 0), 2);
        cv::putText(imgCopy, result.text, cv::Point(adjustedBox.x, adjustedBox.y - 5),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 1);
    }

    // Additional bottom ROI with boundary checks
    cv::Rect bottomROI(320, 600, 800, 200);
    bottomROI &= cv::Rect(0, 0, nImg.cols, nImg.rows); // Ensure within bounds
    cv::Mat bottomRoiImg = nImg(bottomROI);

    std::vector<OCRResult> bottomResults = PerformOCR(bottomRoiImg);
    MergeOCRResults(bottomResults);

    for (const auto& result : bottomResults) {
        cv::Rect adjustedBox = result.boundingBox;
        adjustedBox.x += bottomROI.x;
        adjustedBox.y += bottomROI.y;

        cv::rectangle(imgCopy, adjustedBox, cv::Scalar(0, 0, 255), 2);
        cv::putText(imgCopy, result.text, cv::Point(adjustedBox.x, adjustedBox.y - 5),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 1);
    }

    // Show the image with bounding boxes
    cv::imshow(nImgName, imgCopy);
    cv::waitKey(0);
}

int OCREngine::ProcessOCR(const HWND& nHWND) {
    // Get the device context (DC) of the window
    HDC hWindowDC = GetDC(nHWND);

    // Get the device context to copy into
    HDC hWindowCompatibleDC = CreateCompatibleDC(hWindowDC);

    // Get the size of the window
    RECT windowRect;
    GetClientRect(nHWND, &windowRect);

    int _width = windowRect.right - windowRect.left;
    int _height = windowRect.bottom - windowRect.top;

    // Create a compatible bitmap from the Window DC
    HBITMAP hWindowBitmap = CreateCompatibleBitmap(hWindowDC, _width, _height);

    // Select the compatible bitmap into the compatible memory DC
    SelectObject(hWindowCompatibleDC, hWindowBitmap);

    // Copy the window image to the compatible memory DC
    BitBlt(hWindowCompatibleDC, 0, 0, _width, _height, hWindowDC, 0, 0, SRCCOPY);

    // Create a BITMAPINFOHEADER to hold the information about the bitmap
    BITMAPINFOHEADER bi = { sizeof(BITMAPINFOHEADER), _width, -_height, 1, 32, BI_RGB, 0, 0, 0, 0, 0 };

    // Create a cv::Mat object to hold the image
    cv::Mat mat(_height, _width, CV_8UC4); // 4 channels (RGBA)

    // Get the bitmap data and store it in the cv::Mat object
    GetDIBits(hWindowCompatibleDC, hWindowBitmap, 0, _height, mat.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    // Clean up
    DeleteObject(hWindowBitmap);
    DeleteDC(hWindowCompatibleDC);
    ReleaseDC(nHWND, hWindowDC);

    // Define the main region of interest (ROI) with boundary check
    int x = 90;
    int y = 140;
    int width = 250;
    int height = 350;
    cv::Rect roi(x, y, width, height);
    roi &= cv::Rect(0, 0, mat.cols, mat.rows); // Ensure within bounds

    // Process ROI and then the rest of the image
    this->ProcessROIAndEntireImage(mat, roi, "Large Image");

    return 0;

}