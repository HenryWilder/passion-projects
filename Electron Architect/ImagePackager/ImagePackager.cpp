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
        "program_icon",
        "icons_blueprint16x",
        "icon_blueprints16x",
        "icon_blueprints32x",
        "icon_clipboard16x",
        "icon_clipboard32x",
        "icons_gate16x",
        "icons_gate32x",
        "icons_mode16x",
        "icons_mode32x",
    };

    for (const char* f : files)
    {
        Export(f);
    }

    CloseWindow();
}
