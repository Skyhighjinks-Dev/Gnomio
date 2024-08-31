// OCREngine.h
#pragma once

#define NOMINMAX // Prevent conflicts with std::min and std::max
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <opencv2/opencv.hpp>
#include <windows.h>
#include <vector>
#include <map>
#include <string>
#include <stdexcept>
#include <algorithm> // Required for std::max and std::min
#include <opencv2/highgui.hpp> // For displaying images
#include <opencv2/imgproc.hpp> // For image processing functions

struct OCRResult {
    std::string Text;
    cv::Rect BoundingBox;
};


// Structures for words, lines, and paragraphs
struct OCRWord {
    std::string Text;
    cv::Rect BoundingBox;
};

struct OCRLine {
    std::vector<OCRWord> Words;

    int Top() const { return Words.front().BoundingBox.y; }
    int Bottom() const { return Words.back().BoundingBox.y + Words.back().BoundingBox.height; }
};

struct OCRParagraph {
    std::vector<OCRLine> Lines;
};

enum OCR_TYPE {
    TAUNAHI_LEFT_MENU = 1,  // 1 (0001)
    TAUNAHI_CENTER_MENU = 2, // 2 (0010)
    MC_MAIN_MENU_CENTER = 4,
    MC_MULTI_PLAY_MENU_BOTTOM = 8,
    MC_DIR_CON_BUTTON = 16
};

class OCREngine {
public:
    // Process OCR with HWND, text to find, and OCR type
    std::vector<OCRResult> ProcessOCR(const HWND* nHWND, const std::string nTextToFind, const OCR_TYPE nType);

    // Process OCR with HWND and OCR type
    std::vector<OCRResult> ProcessOCR(const HWND* nHWND, const OCR_TYPE nType);

private:
    // Get areas of interest based on OCR type
    static std::vector<RECT> GetAreasOfInterest(OCR_TYPE nType);

    // Process the image via OCR
    std::vector<OCRResult> ProcessImageViaOCR(cv::Mat& nImage, std::vector<RECT> nAOI, bool nCaptureAll);

    // Capture screenshot of the specified window
    cv::Mat CaptureWindow(const HWND* hwnd);

    // Find text in the OCR results
    std::vector<OCRResult> FindText(const std::vector<OCRResult>& results, const std::string& textToFind);
    std::vector<OCRResult> ConvertParagraphsToOCRResults(const std::vector<OCRParagraph>& paragraphs);
    std::vector<OCRParagraph> MergeLinesIntoParagraphs(const std::vector<OCRLine>& lines);
    bool IsLineProximal(const OCRLine& line1, const OCRLine& line2);
};
