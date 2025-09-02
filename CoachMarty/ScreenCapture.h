#pragma once
#include <windows.h>
#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>
#include <string>

class ScreenCapture
{
  public:
    ScreenCapture();
    ~ScreenCapture();
    void ScreenShot(const std::string filePath, const int x1, const int y1, const int x2, const int y2);
    int monitorIndex = 0;
    std::vector<HMONITOR> monitors;

  private:
    int GetMonitorIndex();
};
