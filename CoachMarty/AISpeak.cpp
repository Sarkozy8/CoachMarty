#include "AISpeak.h"

AISpeak::AISpeak()
{
}

AISpeak::~AISpeak()
{
}

// Calls llama.cpp to generate a debrief based on the stats obtained throught the game. This is called at halftime and
// end of the game.
void AISpeak::DebriefCurrentGame(const std::map<std::string, std::string> &stats, bool isHalfTime, bool isEndOfTheGame,
                                 int team)
{

    // Create the prompt based on the stats
    // You can modify this prompt to change the behavior of the AI however you want.

    std::string prompt = "You are Coach Marty from Blue Mountain State the series. I am going to give you some "
                         "American Football stats and I want you to give me some advise of what to improve and what "
                         "the whole team is doing right. I want you to do A LOT OF TRASH TALK. \n ";
    if (isEndOfTheGame)
    {
        prompt += "The game is over. Please if the player lost talk a little bit of smack but end with a good note and "
                  "some advise. If the player won, tell him some good jokes Marty style and tell him what he did good. "
                  "If the player lost by a lot, say he did pretty bad. If we won by a lot, say we kick their asses.";
    }
    else if (isHalfTime)
    {
        prompt += "Right now we are in Halftime and the player needs some pep talk to push forward.";
    }

    if (team == 1)
    {
        prompt += "Player is on the left side team. Left side Stats assigned to the player. Right Side Stats assigned "
                  "to the opponent. BUT never say the left side team. Just talk about them and us.";
    }
    if (team == 2)
    {
        prompt += "Player is on the right side team. Right side Stats assigned to the player. Left Side Stats assigned "
                  "to the opponent. BUT never say the right side team. Just talk about them and us.";
    }
    prompt += "Here are the stats:\n";
    for (const auto &pair : stats)
    {
        prompt += pair.first + ": " + pair.second + "\n";
    }

    // Here I sent the constraints for the answer. You can modify this to your liking and very needed for some models.
    prompt += "VERY IMPORTANT. ONLY USE NUMBERS FROM THE PROMPT. YOU ARE TALKING TO A PLAYER AND "
              "TELLING HIM EVERYTHING IN 6 SENTENCES, NO MORE.\n";

    // Mak txt file that llma.cpp will read
    std::ofstream out("prompt.txt");
    out << prompt;
    out.close();

    // Call llama.cpp to generate the debrief through terminal command (Here you can change the -hf for different
    // models. This will download this model to the computer if you dont have it. More models in HuggingFace)
    // You can also use lighter models if you want faster performance but less quality.
    // Example of lighter model: -hf ggml-org/gemma-3-1b-it-GGUF
    // Local model also work: -m C:/path/to/your/model.gguf
    // Change --n-gpu-layers according to your GPU VRAM. More layers = faster but more VRAM usage.

    std::system("llama-cli -hf ggml-org/gemma-3-12b-it-qat-GGUF -f prompt.txt -no-cnv --n-gpu-layers 10 "
                "--no-context-shift > output.txt");

    // Read the output from llama.cpp
    std::ifstream in("output.txt");
    std::string result((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    // Sanitize the output to only get the debrief part (Depending of your prompt, this step can vary.)
    // Here I look for the "NO MORE." marker to know where the debrief starts and remove everything before it.
    const std::string marker = "NO MORE.";
    size_t markerPos = result.find(marker);
    if (markerPos != std::string::npos)
    {
        result = result.erase(0, markerPos + marker.length());
    }
    else
    {
        std::cout << "\"NO MORE.\" marker not found in output." << std::endl;
    }

    const std::string endMarker = "[end of text]";
    size_t endPos = result.find(endMarker);
    if (endPos != std::string::npos)
    {
        result.erase(endPos, endMarker.length());
    }

    // Translate text defrief into speech
    SpeakDebrief(result);
}

// Turns text into speech using SAPI.
// In case you would like to use another TTS engine, this is the place to do it.
// I did not include any other TTS engine as SAPI is included in Windows by default.
void AISpeak::SpeakDebrief(const std::string &speech)
{
    std::cout << "AI Speech: " << speech << std::endl;

    // Convert string to wide string for SAPI
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wspeech = converter.from_bytes(speech);

    // Initialize COM and SAPI
    ISpVoice *pVoice = nullptr;
    if (FAILED(::CoInitialize(nullptr)))
        return;

    // Create SAPI voice instance
    HRESULT hr = CoCreateInstance(CLSID_SpVoice, nullptr, CLSCTX_ALL, IID_ISpVoice, (void **)&pVoice);
    if (SUCCEEDED(hr))
    {
        pVoice->Speak(wspeech.c_str(), 0, nullptr); // Speak the text
        pVoice->Release();                          // Release the voice instance
    }
    ::CoUninitialize(); // Uninitialize COM
}
