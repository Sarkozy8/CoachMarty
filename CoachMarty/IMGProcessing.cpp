#include "IMGProcessing.h"

IMGProcessing::IMGProcessing()
{
}

IMGProcessing::~IMGProcessing()
{
}

// Preprocess image for better OCR results
cv::Mat IMGProcessing::PreprocessText(const std::string filePath)
{
    // Load image using OpenCV
    cv::Mat image = cv::imread(filePath);
    if (image.empty())
    {
        throw std::runtime_error("Could not open or find the image: " + filePath);
    }

    // Grayscale and invert
    cv::Mat gray, inverted;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    cv::bitwise_not(gray, inverted);

    // Binary threshold
    cv::Mat binary;
    cv::threshold(inverted, binary, 150, 255, cv::THRESH_BINARY);

    // Dilation
    cv::Mat dilated;
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 2));
    cv::dilate(binary, dilated, kernel);

    // Scale up
    cv::Mat scaled;
    cv::resize(dilated, scaled, cv::Size(), 2.0, 2.0, cv::INTER_LINEAR);

    // Sharpen
    cv::Mat sharp;
    cv::GaussianBlur(scaled, sharp, cv::Size(0, 0), 3);
    cv::addWeighted(scaled, 1.5, sharp, -0.5, 0, sharp);

    // Save and load with Leptonica
    if (!cv::imwrite(filePath, sharp))
    {
        throw std::runtime_error("Could not write the image to file: " + filePath);
    }

    return sharp;
}

// Check if a pixel on the screen matches a target color within a tolerance (Used for checking if Stats Display is
// available)
bool IMGProcessing::CheckPixelColor(std::vector<HMONITOR> monitors, const int monitorIndex, int x, int y,
                                    const COLORREF targetColor, const int tolerance)
{
    // Get the monitor's dimensions
    MONITORINFO monitorInfo = {sizeof(MONITORINFO)};
    GetMonitorInfo(monitors[monitorIndex], &monitorInfo);
    int monLeft = monitorInfo.rcMonitor.left;
    int monTop = monitorInfo.rcMonitor.top;
    int monRight = monitorInfo.rcMonitor.right;
    int monBottom = monitorInfo.rcMonitor.bottom;

    // Clamp coordinates to monitor bounds
    x = (std::max)(0, (std::min)(x, monRight - monLeft - 1));
    y = (std::max)(0, (std::min)(y, monBottom - monTop - 1));

    // Create a device context for the monitor
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMemDC = CreateCompatibleDC(hdcScreen);
    HBITMAP hbmScreen = CreateCompatibleBitmap(hdcScreen, 1, 1);
    SelectObject(hdcMemDC, hbmScreen);

    // Copy the pixel from the monitor
    BitBlt(hdcMemDC, 0, 0, 1, 1, hdcScreen, monLeft + x, monTop + y, SRCCOPY);

    // Extra BITMAPINFO
    BITMAPINFOHEADER bi;
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = 1;
    bi.biHeight = -1; // top-down
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    // Create a buffer to hold the pixel data
    cv::Mat mat(1, 1, CV_8UC4);
    GetDIBits(hdcScreen, hbmScreen, 0, 1, mat.data, (BITMAPINFO *)&bi, DIB_RGB_COLORS);

    // Release resources
    DeleteObject(hbmScreen);
    DeleteDC(hdcMemDC);
    ReleaseDC(NULL, hdcScreen);

    // Extract RGB values (its on ABGR)
    cv::Vec4b pixel = mat.at<cv::Vec4b>(0, 0);
    int b = pixel[0];
    int g = pixel[1];
    int r = pixel[2];

    int tr = GetRValue(targetColor);
    int tg = GetGValue(targetColor);
    int tb = GetBValue(targetColor);

    // Compare with tolerance
    return (abs(r - tr) <= tolerance) && (abs(g - tg) <= tolerance) && (abs(b - tb) <= tolerance);
}

// Compare logos to determine which side the mini logo is on (1 = Left, 2 = Right) ()
int IMGProcessing::CompareLogos()
{
    // Load images
    cv::Mat miniLogo = cv::imread("MiniLogo.png", cv::IMREAD_COLOR);
    cv::Mat leftLogo = cv::imread("LeftLogo.png", cv::IMREAD_COLOR);
    cv::Mat rightLogo = cv::imread("RightLogo.png", cv::IMREAD_COLOR);

    if (miniLogo.empty() || leftLogo.empty() || rightLogo.empty())
    {
        throw std::runtime_error("Could not load one or more logo images.");
    }

    // Resize miniLogo for template matching
    cv::Mat miniResizedLeft, miniResizedRight;
    cv::resize(miniLogo, miniResizedLeft, leftLogo.size());
    cv::resize(miniLogo, miniResizedRight, rightLogo.size());

    // Template matching: miniLogo as template, left/right as search images (in color). TM_SQDIFF gave me the best
    // results
    cv::Mat resultLeft, resultRight;
    cv::matchTemplate(leftLogo, miniLogo, resultLeft, cv::TM_SQDIFF);
    cv::matchTemplate(rightLogo, miniLogo, resultRight, cv::TM_SQDIFF);

    double minValL, maxValL, minValR, maxValR;
    cv::minMaxLoc(resultLeft, &minValL, &maxValL);
    cv::minMaxLoc(resultRight, &minValR, &maxValR);

    std::cout << "Left Match Confidence: " << maxValL << std::endl;
    std::cout << "Right Match Confidence: " << maxValR << std::endl;
    return (maxValL > maxValR) ? 1 : 2;
}
