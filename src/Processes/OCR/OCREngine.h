#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <windows.h>

class OCREngine {
private:
    struct OCRResult {
        std::string text;
        cv::Rect boundingBox;
    };

    bool AreBoxesClose(const cv::Rect& a, const cv::Rect& b);
    std::vector<OCRResult> PerformOCR(const cv::Mat& nImg);
    void MergeOCRResults(std::vector<OCRResult>& nResults);
    void ProcessROIAndEntireImage(const cv::Mat& nImg, const cv::Rect& nROI, const std::string& nImgName);
    cv::Mat CaptureWindow(HWND nHWND);
public:
    int ProcessOCR(const HWND& nPath);
};
