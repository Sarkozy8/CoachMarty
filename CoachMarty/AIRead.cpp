#include "AIRead.h"

AIRead::AIRead()
{
    tess = new tesseract::TessBaseAPI();

    if (tess->Init("X:/Game_Developer/C++ Projects/.Libraries/Tesseract/tessdata", "eng"))
    {
        throw std::runtime_error("Could not initialize tesseract.");
    }
}

AIRead::~AIRead()
{
    delete tess;
}

// Read text from image file using Tesseract OCR
std::string AIRead::ReadText(const char *filePath)
{
    // Read image
    Pix *pix = pixRead(filePath);

    // Set the image to recognize
    tess->SetImage(pix);

    // Get OCR result
    char *out = tess->GetUTF8Text();

    // Convert to String for easier handling
    std::string outStr = std::string(out);

    // Clean up
    pixDestroy(&pix);
    delete[] out;

    return outStr;
}
