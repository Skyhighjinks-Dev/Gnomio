#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <windows.h>

struct OCRResult {
    std::string text;
    cv::Rect boundingBox;
};

class OCREngine {
public:
    std::vector<OCRResult> ProcessOCR(const HWND& nPath);
private:
    bool AreBoxesClose(const cv::Rect& a, const cv::Rect& b);
    std::vector<OCRResult> PerformOCR(const cv::Mat& nImg);
    void MergeOCRResults(std::vector<OCRResult>& nResults);
    std::vector<OCRResult> ProcessROIAndEntireImage(const cv::Mat& nImg, const cv::Rect& nROI, const std::string& nImgName);
    cv::Mat CaptureWindow(HWND nHWND);
};
