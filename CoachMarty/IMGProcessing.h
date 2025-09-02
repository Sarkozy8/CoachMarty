#pragma once
#include <opencv2/opencv.hpp>
#include <windows.h>
#include <string>

class IMGProcessing
{
  public:
    IMGProcessing();
    ~IMGProcessing();
    cv::Mat PreprocessText(const std::string filePath);
    bool CheckPixelColor(std::vector<HMONITOR> monitors, const int monitorIndex, int x, int y,
                         const COLORREF targetColor, const int tolerance);
    int CompareLogos();
};
