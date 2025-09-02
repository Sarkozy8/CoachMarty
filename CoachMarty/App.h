#pragma once
#include <windows.h>
#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <conio.h>
#include <iostream>
#include <string>
#include <map>
#include <thread>
#include <chrono>
#include "IMGProcessing.h"
#include "AIRead.h"
#include "ScreenCapture.h"
#include "AISpeak.h"

// Stats display dimensions
#define StatsTitleX1 1210
#define StatsTitleY1 90
#define StatsTitleX2 1522
#define StatsTitleY2 145

// Color and position to check if stats display is visible
#define StatsDisplayColor RGB(35, 35, 37)
#define StatsColorX 1035
#define StatsColorY 450

// Logo positions for team identification
#define MiniLogoX1 210
#define MiniLogoY1 500
#define MiniLogoX2 264
#define MiniLogoY2 554

#define LeftLogoX1 1074
#define LeftLogoY1 134
#define LeftLogoX2 1192
#define LeftLogoY2 251

#define RightLogoX1 1523
#define RightLogoY1 134
#define RightLogoX2 1670
#define RightLogoY2 251

class App
{
  public:
    App();
    ~App();
    IMGProcessing imgProcessor;
    AIRead aiReader;
    AISpeak aiSpeaker;
    ScreenCapture screenCapturer;

    bool isRunning = true;
    bool isNewGame = false;
    bool isHalftime = false;
    bool isEndOfGame = false;
    int whichTeamAreYou = 0; // 0 = none, 1 = left, 2 = right
    std::map<std::string, std::string> stats;

    void Run();

  private:
    bool CheckForNewGame();
    bool CheckForHalfTime();
    bool CheckForEndOfGame();
    void CheckForStatsAndUpdateDB();
    std::string CheckTheStat(const int x1, const int y1, const int x2, const int y2);
    void UpdateDB(const std::string statsTitle);
    void CheckWhichTeamYouOn();
    void RemoveSpaces(std::string &str);
};
