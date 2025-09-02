#include "ScreenCapture.h"

ScreenCapture::ScreenCapture()
{
    monitorIndex = GetMonitorIndex();
}

ScreenCapture::~ScreenCapture()
{
}

// Capture a screenshot of a specific rectangle on the selected monitor and save it to filePath
void ScreenCapture::ScreenShot(const std::string filePath, const int x1, const int y1, const int x2, const int y2)
{
    // Get the monitor's dimensions
    MONITORINFO monitorInfo = {sizeof(MONITORINFO)};
    GetMonitorInfo(monitors[monitorIndex], &monitorInfo);
    int monLeft = monitorInfo.rcMonitor.left;
    int monTop = monitorInfo.rcMonitor.top;
    int monRight = monitorInfo.rcMonitor.right;
    int monBottom = monitorInfo.rcMonitor.bottom;

    // Normalize coordinates
    int left = (std::min)(x1, x2);
    int top = (std::min)(y1, y2);
    int right = (std::max)(x1, x2);
    int bottom = (std::max)(y1, y2);

    // Clamp to monitor bounds
    left = std::max(0, left);
    top = std::max(0, top);
    right = std::min(monRight - monLeft, right);
    bottom = std::min(monBottom - monTop, bottom);

    int width = right - left;
    int height = bottom - top;

    if (width <= 0 || height <= 0)
        throw std::runtime_error("Invalid rectangle dimensions for screenshot.");

    // Create a device context for the monitor
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMemDC = CreateCompatibleDC(hdcScreen);
    HBITMAP hbmScreen = CreateCompatibleBitmap(hdcScreen, width, height);
    SelectObject(hdcMemDC, hbmScreen);

    // Copy the specified rectangle from the monitor
    BitBlt(hdcMemDC, 0, 0, width, height, hdcScreen, monLeft + left, monTop + top, SRCCOPY);

    // Extra BITMAPINFO
    BITMAPINFOHEADER bi;
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height; // Negative to ensure top-down bitmap
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    // Create a buffer to hold the pixel data
    cv::Mat mat(height, width, CV_8UC4); // 4 channels (BGRA)
    GetDIBits(hdcScreen, hbmScreen, 0, height, mat.data, (BITMAPINFO *)&bi, DIB_RGB_COLORS);

    // Release resources
    DeleteObject(hbmScreen);
    DeleteDC(hdcMemDC);
    ReleaseDC(NULL, hdcScreen);

    // Save the image using OpenCV
    if (!cv::imwrite(filePath, mat))
    {
        throw std::runtime_error("Could not write the image to file: " + filePath);
    }
}

// Prompt user to select a monitor and return its index (Prompt at the very beggining of the App)
int ScreenCapture::GetMonitorIndex()
{
    std::cout << "Enter the monitor index (0 for primary, 1 for secondary, etc.): ";
    std::cin >> monitorIndex;

    // Enumerate all monitors
    EnumDisplayMonitors(
        NULL, NULL,
        [](HMONITOR hMonitor, HDC, LPRECT, LPARAM lParam) -> BOOL {
            auto *monitorList = reinterpret_cast<std::vector<HMONITOR> *>(lParam);
            monitorList->push_back(hMonitor);
            return TRUE;
        },
        reinterpret_cast<LPARAM>(&monitors));

    // Check if the monitor index is valid
    if (monitorIndex < 0 || monitorIndex >= monitors.size())
    {
        throw std::runtime_error("Invalid monitor index!");
    }

    return monitorIndex;
}
