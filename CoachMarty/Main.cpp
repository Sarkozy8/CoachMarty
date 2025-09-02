#include "App.h"

int main()
{
    try
    {
        std::unique_ptr<App> app = std::make_unique<App>();
        app->Run();
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
    }

    return 0;
}
