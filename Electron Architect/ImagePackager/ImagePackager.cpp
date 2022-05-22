#include <raylib.h>

void Export(const char* name)
{
    Image image = LoadImage(TextFormat("..\\Electron Architect\\%s.png", name));

    ExportImageAsCode(image, TextFormat("..\\Electron Architect\\%s.h", name));

    UnloadImage(image);
}

int main()
{
    InitWindow(1280, 720, "Test");
    
    const char* files[] = {
        "nodeIcons",
        "nodeIconsNTD",
        "nodeIconsHighlight",
    };

    for (const char* f : files)
    {
        Export(f);
    }

    CloseWindow();
}
