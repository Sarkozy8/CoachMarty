#include "App.h"

App::App()
{
}

App::~App()
{
}

void App::Run()
{

    while (isRunning)
    {
        // Check for Escape key press (didnt use SDL since it too much setup for such a simple function)
        if (_kbhit())
        {
            int ch = _getch();
            if (ch == 27) // 27 is the ASCII code for Escape
            {
                isRunning = false;
                break;
            }
        }

        // Check if stats display is available
        if (imgProcessor.CheckPixelColor(screenCapturer.monitors, screenCapturer.monitorIndex, StatsColorX, StatsColorY,
                                         StatsDisplayColor, 10))
        {
            // Reset Data Base in case of new game
            if (CheckForNewGame())
            {
                isNewGame = true;
                isHalftime = false;
                isEndOfGame = false;
                int whichTeamAreYou = 0;
                stats.clear();

                std::cout << "New game detected. Data reset." << std::endl;
            }

            // If stats display is available, update DB
            if (isNewGame)
            {
                CheckForStatsAndUpdateDB();
            }
        }
        else if (isNewGame)
        {
            // Check for Halftime and End of Game are here to avoid checking when stats display  is available and only
            // after a new game is detected
            if (CheckForHalfTime())
            {
                isHalftime = true;
                aiSpeaker.DebriefCurrentGame(stats, isHalftime, isEndOfGame, whichTeamAreYou);

                std::cout << "Halftime detected." << std::endl;
            }

            if (CheckForEndOfGame())
            {
                isEndOfGame = true;
                isHalftime = true;
                isNewGame = false;
                aiSpeaker.DebriefCurrentGame(stats, isHalftime, isEndOfGame, whichTeamAreYou);

                std::cout << "End of game detected." << std::endl;
            }
            std::cout << "Stats display not detected." << std::endl;
        }

        // Run this loop every second
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

// Check for new game by looking for "WERTHER" text on Stats display
bool App::CheckForNewGame()
{
    // Check if New game by looking for "WERTHER" text at specific position, WERTHER is WEATHER with OCR error
    screenCapturer.ScreenShot("Weather.png", 1290, 108, 1430, 142);
    imgProcessor.PreprocessText("Weather.png");
    std::string result = aiReader.ReadText("Weather.png");
    RemoveSpaces(result);
    std::cout << "Recognized Text: " << result << std::endl;

    // If recognized text is "WEATHER" or "WERTHER", it's a new game
    if (result == "WERTHER" || result == "WEATHER")
    {
        return true;
    }

    return false;
}

// Check for halftime by looking for 3rd quarter at specific position
bool App::CheckForHalfTime()
{
    if (isHalftime)
    {
        return false; // Already in halftime, skip check
    }

    screenCapturer.ScreenShot("HalfTime.png", 872, 875, 886, 897);
    std::string result = aiReader.ReadText("HalfTime.png");
    RemoveSpaces(result);
    std::cout << "Recognized Text: " << result << std::endl;

    if (result == "3")
    {
        stats["Score_Left"] = CheckTheStat(776, 870, 845, 918);
        stats["Score_Right"] = CheckTheStat(1077, 870, 1142, 918);
        return true;
    }

    return false;
}

// Check for end of game by looking for "FINAL" text at the end of the game
bool App::CheckForEndOfGame()
{
    if (isEndOfGame)
    {
        return false; // Already at end of game
    }

    screenCapturer.ScreenShot("EndOfGame.png", 787, 791, 851, 811);
    imgProcessor.PreprocessText("EndOfGame.png");
    std::string result = aiReader.ReadText("EndOfGame.png");
    RemoveSpaces(result);
    std::cout << "Recognized Text: " << result << std::endl;

    if (result == "FINAL")
    {
        // Get final score
        stats["Score_Left"] = CheckTheStat(785, 825, 850, 854);
        stats["Score_Right"] = CheckTheStat(785, 867, 857, 897);

        return true;
    }

    return false;
}

// Check for stats title and update DB accordingly
void App::CheckForStatsAndUpdateDB()
{
    screenCapturer.ScreenShot("StatsTitle.png", StatsTitleX1, StatsTitleY1, StatsTitleX2, StatsTitleY2);
    imgProcessor.PreprocessText("StatsTitle.png");
    std::string statsTitle = aiReader.ReadText("StatsTitle.png");
    RemoveSpaces(statsTitle);
    std::cout << "Stats Text: " << statsTitle << std::endl;

    UpdateDB(statsTitle);
}

// Capture and read a specific stat from given coordinates
std::string App::CheckTheStat(const int x1, const int y1, const int x2, const int y2)
{
    screenCapturer.ScreenShot("Stat.png", x1, y1, x2, y2);
    imgProcessor.PreprocessText("Stat.png");
    std::string statValue = aiReader.ReadText("Stat.png");
    RemoveSpaces(statValue);

    std::cout << "Stat Value: " << statValue << std::endl;

    return statValue;
}

// Update DB based on stats title (Future Improvement: Either make a list of all the misspelled words for the Stats or
// train model so detect the game font better.)
void App::UpdateDB(const std::string statsTitle)
{

    if (statsTitle == "GAMESUMMARY")
    {
        std::cout << "Updating Game Summary stats..." << std::endl;
        stats["FirstDowns_Left"] = CheckTheStat(1097, 315, 1162, 356);
        stats["FirstDowns_Right"] = CheckTheStat(1549, 316, 1622, 355);
        // stats["PossessionTime_Left"] = CheckTheStat(1100, 370, 1163, 415); // Not Reliable because of the colons
        // stats["PossessionTime_Right"] = CheckTheStat(1547, 368, 1618, 411); // Not Reliable because of the colons
        stats["Yards_Left"] = CheckTheStat(1095, 421, 1167, 464);
        stats["Yards_Right"] = CheckTheStat(1548, 418, 1620, 463);
        CheckWhichTeamYouOn();
    }
    else if (statsTitle == "MATCHUDSUMMARY")
    {
        std::cout << "Updating Matchup Summary stats..." << std::endl;
        stats["OverallStats_Left"] = CheckTheStat(1097, 315, 1162, 356);
        stats["OverallStats_Right"] = CheckTheStat(1549, 316, 1622, 355);
        stats["OffenseStats_Left"] = CheckTheStat(1100, 370, 1163, 415);
        stats["OffenseStats_Right"] = CheckTheStat(1547, 368, 1618, 411);
        stats["DefenseStats_Left"] = CheckTheStat(1095, 421, 1167, 464);
        stats["DefenseStats_Right"] = CheckTheStat(1548, 418, 1620, 463);
        CheckWhichTeamYouOn();
    }
    else if (statsTitle == "TEAMOFFENSE" || statsTitle == "TERMOFFENSE")
    {
        std::cout << "Updating Team Offense stats..." << std::endl;
        stats["PassingYards_Left"] = CheckTheStat(1053, 316, 1171, 357);
        stats["PassingYards_Right"] = CheckTheStat(1543, 316, 1633, 357);
        stats["RushingYards_Left"] = CheckTheStat(1053, 371, 1171, 409);
        stats["RushingYards_Right"] = CheckTheStat(1543, 371, 1633, 409);
        stats["TouchDowns_Left"] = CheckTheStat(1053, 420, 1171, 460);
        stats["TouchDowns_Right"] = CheckTheStat(1543, 420, 1633, 460);
        CheckWhichTeamYouOn();
    }
    else if (statsTitle == "SEASONINDEFENSE")
    {
        std::cout << "Updating Season In Defense stats..." << std::endl;
        stats["YardsAllowedPerGame_Left"] = CheckTheStat(1053, 316, 1171, 357);
        stats["YardsAllowedPerGame_Right"] = CheckTheStat(1543, 316, 1633, 357);
        stats["PointsAllowedPerGame_Left"] = CheckTheStat(1053, 371, 1171, 409);
        stats["PointsAllowedPerGame_Right"] = CheckTheStat(1543, 371, 1633, 409);
        stats["Turnovers_Left"] = CheckTheStat(1053, 420, 1171, 460);
        stats["Turnovers_Right"] = CheckTheStat(1543, 420, 1633, 460);
        CheckWhichTeamYouOn();
    }
    else if (statsTitle == "SEASONINOFFENSE")
    {
        std::cout << "Updating Season in Offense stats..." << std::endl;
        stats["YardsPerGame_Left"] = CheckTheStat(1053, 316, 1171, 357);
        stats["YardsPerGame_Right"] = CheckTheStat(1543, 316, 1633, 357);
        stats["FirstDownPerGame_Left"] = CheckTheStat(1053, 371, 1171, 409);
        stats["FirstDownPerGame_Right"] = CheckTheStat(1543, 371, 1633, 409);
        stats["TouchdownsPerGame_Left"] = CheckTheStat(1053, 420, 1171, 460);
        stats["TouchdownsPerGame_Right"] = CheckTheStat(1543, 420, 1633, 460);
        CheckWhichTeamYouOn();
    }
    else if (statsTitle == "DEFENSE")
    {
        std::cout << "Updating Defense Summary stats..." << std::endl;
        stats["Deflections_Left"] = CheckTheStat(1053, 316, 1171, 357);
        stats["Deflections_Right"] = CheckTheStat(1543, 316, 1633, 357);
        stats["Sacks_Left"] = CheckTheStat(1053, 371, 1171, 409);
        stats["Sacks_Right"] = CheckTheStat(1543, 371, 1633, 409);
        stats["Interceptions_Left"] = CheckTheStat(1053, 420, 1171, 460);
        stats["Interceptions_Right"] = CheckTheStat(1543, 420, 1633, 460);
        CheckWhichTeamYouOn();
    }
}

// Determine which team the player is on by comparing logos
void App::CheckWhichTeamYouOn()
{
    if (whichTeamAreYou != 0)
    {
        return; // Already determined
    }
    // Capture logos
    screenCapturer.ScreenShot("MiniLogo.png", MiniLogoX1, MiniLogoY1, MiniLogoX2, MiniLogoY2);
    screenCapturer.ScreenShot("LeftLogo.png", LeftLogoX1, LeftLogoY1, LeftLogoX2, LeftLogoY2);
    screenCapturer.ScreenShot("RightLogo.png", RightLogoX1, RightLogoY1, RightLogoX2, RightLogoY2);
    whichTeamAreYou = imgProcessor.CompareLogos();
    if (whichTeamAreYou == 1)
    {
        std::cout << "You are on the LEFT team." << std::endl;
    }
    else if (whichTeamAreYou == 2)
    {
        std::cout << "You are on the RIGHT team." << std::endl;
    }
    else
    {
        std::cout << "Could not determine your team." << std::endl;
    }
}

// Snitize string by removing spaces and newlines
void App::RemoveSpaces(std::string &str)
{
    str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
    str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
    str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
}
