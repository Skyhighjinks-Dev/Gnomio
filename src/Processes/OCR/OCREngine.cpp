#include "OCREngine.h"

bool OCREngine::AreBoxesClose(const cv::Rect& a, const cv::Rect& b) {
    int distanceThreshold = 20; // Adjust this threshold as necessary
    return (abs(a.y - b.y) < distanceThreshold) && (abs(a.x + a.width - b.x) < distanceThreshold);
}

std::vector<OCREngine::OCRResult> OCREngine::PerformOCR(const cv::Mat& nImg) {
    std::vector<OCRResult> results;

    // Initialize Tesseract API
    tesseract::TessBaseAPI *ocr = new tesseract::TessBaseAPI();
    if (ocr->Init(NULL, "eng")) {
        std::cerr << "Could not initialize tesseract." << std::endl;
        return results;
    }

    // Set the whitelist for characters to a-z, A-Z, and space
    ocr->SetVariable("tessedit_char_whitelist", "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ ");

    // Set the image for Tesseract
    ocr->SetImage(nImg.data, nImg.cols, nImg.rows, 3, nImg.step);

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

    // Merge the text blocks that are close to each other
    for (size_t i = 0; i < nResults.size(); ++i) {
        if (!mergedResults.empty() && AreBoxesClose(mergedResults.back().boundingBox, nResults[i].boundingBox)) {
            mergedResults.back().text += " " + nResults[i].text;
            mergedResults.back().boundingBox = mergedResults.back().boundingBox | nResults[i].boundingBox;
        } else {
            mergedResults.push_back(nResults[i]);
        }
    }

    nResults = mergedResults;
}

void OCREngine::ProcessROIAndEntireImage(const cv::Mat& nImg, const cv::Rect& nROI, const std::string& nImgName) {
    cv::Mat imgCopy = nImg.clone();

    // Extract the region of interest (ROI)
    cv::Mat roiImg = nImg(nROI);

    // Perform OCR on the ROI
    std::vector<OCRResult> roiResults = PerformOCR(roiImg);
    MergeOCRResults(roiResults);

    // Display results and overlay them on the original image
    std::cout << "Results for " << nImgName << " (ROI):" << std::endl;
    for (const auto& result : roiResults) {
        // Adjust the bounding box to match the original image coordinates
        cv::Rect adjustedBox = result.boundingBox;
        adjustedBox.x += nROI.x;
        adjustedBox.y += nROI.y;

        // Print the text and bounding box
        std::cout << "Text: " << result.text << " - Location: "
                  << "x: " << adjustedBox.x << ", y: "
                  << adjustedBox.y << ", width: "
                  << adjustedBox.width << ", height: "
                  << adjustedBox.height << std::endl;

        // Draw bounding box and text on the original image
        cv::rectangle(imgCopy, adjustedBox, cv::Scalar(0, 255, 0), 2);
        cv::putText(imgCopy, result.text, cv::Point(adjustedBox.x, adjustedBox.y - 5),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);
    }

    // Create a mask for the ROI to ignore it during the full image OCR
    cv::Mat mask = cv::Mat::zeros(nImg.size(), CV_8UC1);
    mask(nROI).setTo(cv::Scalar(255)); // Set the ROI region to white in the mask

    // Invert the mask
    cv::bitwise_not(mask, mask);

    // Apply the mask to the original image (only the non-ROI area will be processed)
    cv::Mat maskedImg;
    nImg.copyTo(maskedImg, mask);

    // Perform OCR on the non-ROI part of the image
    std::vector<OCRResult> fullImageResults = PerformOCR(maskedImg);
    MergeOCRResults(fullImageResults);

    std::cout << "Results for " << nImgName << " (Full Image excluding ROI):" << std::endl;
    for (const auto& result : fullImageResults) {
        // No need to adjust bounding boxes as we are working on the entire image
        std::cout << "Text: " << result.text << " - Location: "
                  << "x: " << result.boundingBox.x << ", y: "
                  << result.boundingBox.y << ", width: "
                  << result.boundingBox.width << ", height: "
                  << result.boundingBox.height << std::endl;

        // Draw bounding box and text on the original image
        cv::rectangle(imgCopy, result.boundingBox, cv::Scalar(0, 0, 255), 2);
        cv::putText(imgCopy, result.text, cv::Point(result.boundingBox.x, result.boundingBox.y - 5),
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

    // Define the region of interest (ROI)
    int x = 90;
    int y = 150;
    int width = 250;
    int height = 350;
    cv::Rect roi(x, y, width, height);

    // Process ROI and then the rest of the image
    this->ProcessROIAndEntireImage(mat, roi, "Large Image");

    return 0;
}