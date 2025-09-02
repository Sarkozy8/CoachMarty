#pragma once
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>

class AIRead
{
  public:
    AIRead();
    ~AIRead();
    std::string ReadText(const char *filePath);
    tesseract::TessBaseAPI *tess;
};
