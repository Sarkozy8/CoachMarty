#pragma once
#include <sapi.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <curl/curl.h>
#include <windows.h>
#include <mmsystem.h>
#include <cstdlib>
#include <vector>
#include <locale>
#include <codecvt>
#include <map>

class AISpeak
{
  public:
    AISpeak();
    ~AISpeak();
    void DebriefCurrentGame(const std::map<std::string, std::string> &stats, bool isHalfTime, bool isEndOfTheGame,
                            int team);
    void SpeakDebrief(const std::string &speech);
};
