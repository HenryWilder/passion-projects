#pragma once
#include <raylib.h>
#include <vector>
#include "IVec.h"
#include "UIColors.h"
#include "Buttons.h"

class Group;
class Graph;
struct Tab;
struct Tool;
enum class Mode;
enum class ModeType;

// Reusable address so clipboard doesn't have to delete
extern Blueprint g_clipboardBP;

void DrawTextShadowedIV(const std::string& text, IVec2 pos, int fontSize, Color color, Color shadow);

enum class LogType
{
    // Just FYI
    info = 0,
    // Upcoming might not be successful
    attempt = 1,
    // Successful
    success = 2,
    // Unsuccessful, can still continue
    warning = 3,
    // Cannot continue
    error = 4,
};

struct UIStyle
{
    Color fontColor = UIColor(UIColorID::UI_COLOR_FOREGROUND);
    Color backgroundColor = UIColor(UIColorID::UI_COLOR_BACKGROUND1);
    
    static UIStyle Title();
    static UIStyle Subtitle();
    static UIStyle Property();
};

//struct Property
//{
//    Property()
//    {
//        memset(this, 0, sizeof(Property));
//        name = "";
//    }
//    ~Property()
//    {
//        if (value.tag == Type::ty_str)
//        {
//            if (value.s != nullptr)
//                delete[] value.s;
//        }
//        else if (value.tag == Type::ty_arr)
//        {
//            if (value.a.arr != nullptr)
//                delete[] value.a.arr;
//        }
//    }
//
//    enum class Type{ ty_bool, ty_char, ty_int, ty_flt, ty_ptr, ty_str, ty_arr };
//    std::string name;
//    struct Value
//    {
//        Value()
//        {
//            memset(this, 0, sizeof(Value));
//        }
//        Type tag;
//        union
//        {
//            bool b;
//            char c;
//            int i;
//            float f;
//            void* p; // Use as a number, not a reference
//            char* s;
//            struct
//            {
//                size_t size = 0;
//                Property* arr = nullptr;
//            } a;
//        };
//        Type TypeOf() const
//        {
//            return tag;
//        }
//        std::string ToString() const
//        {
//            switch (tag)
//            {
//            case Property::Type::ty_bool: return b ? "true" : "false";
//            case Property::Type::ty_char: return { c };
//            case Property::Type::ty_int:  return std::to_string(i);
//            case Property::Type::ty_flt:  return std::to_string(f);
//            case Property::Type::ty_ptr:  return TextFormat("0x%p", p);
//            case Property::Type::ty_str:  return s;
//            default: return "";
//            }
//        }
//    } value;
//
//    const std::string& Name() const { return name; }
//
//    template<typename T> const T& GetValue() const { static_assert(false, "No instantiation of this function exists for a property of this type"); }
//    template<> const bool GetValue<bool>() const { return value.b; }
//    template<> const char GetValue<char>() const { return value.c; }
//    template<> const int GetValue<int>() const { return value.i; }
//    template<> const float GetValue<float>() const { return value.f; }
//    template<> const void* GetValue<void*>() const { return value.p; }
//    template<> const std::string GetValue<std::string>() const { return value.s; }
//
//    size_t GetArraySize() const { return value.a.size; }
//    Property& GetArrayElementAt(size_t index) const { return value.a.arr[index]; }
//};
//template<typename T> Property CreateProperty(const std::string& name, const T& value) { static_assert(false, "No instantiation exists for a property of this type"); }
//template<> Property CreateProperty<bool>(const std::string& name, const bool& value) { return { .name{ name }, .value{ .tag{ Property::Type::ty_bool }, .b{value} } }; }
//template<> Property CreateProperty<char>(const std::string& name, const char& value) { return { .name{ name }, .value{ .tag{ Property::Type::ty_char }, .c{value} } }; }
//template<> Property CreateProperty<int>(const std::string& name, const int& value) { return { .name{ name }, .value{ .tag{ Property::Type::ty_int }, .i{value} } }; }
//template<> Property CreateProperty<float>(const std::string& name, const float& value) { return { .name{ name }, .value{ .tag{ Property::Type::ty_flt }, .f{value} } }; }
//template<class Ty> Property CreateProperty<std::add_pointer_t<Ty>>(const std::string& name, const std::add_pointer_t<Ty>& value)
//{
//    return { .name{ name }, .value{ .tag{ Property::Type::ty_ptr }, .p{value} } };
//}
//template<> Property CreateProperty<std::string>(const std::string& name, const std::string& value)
//{
//    Property prop = { .name{ name }, .value{.tag{ Property::Type::ty_str }, .s{new char[value.size() + 1]} } };
//    for (size_t i = 0; i < value.size(); ++i)
//    {
//        prop.value.s[i] = value[i];
//    }
//    prop.value.s[value.size()] = '\0';
//    return prop;
//}
//template<> Property CreateProperty<std::vector<Property>>(const std::string& name, const std::vector<Property>& value)
//{
//    Property prop = { .name{ name }, .value{.tag{ Property::Type::ty_arr }, .a{ .size{ value.size() }, .arr{ new Property[value.size()] } } } };
//    for (size_t i = 0; i < value.size(); ++i)
//    {
//        prop.value.s[i] = value[i];
//    }
//    prop.value.s[value.size()] = '\0';
//    return prop;
//}

struct Window
{
    Window();
    ~Window();

private:
    Texture2D iconSheet16x;
    Texture2D iconSheet32x;
public:

    int windowWidth;
    int windowHeight;

    size_t activeTab;
    std::vector<Tab*> tabs;
    Tab& CurrentTab();
    const Tab& CurrentTab() const;

    uint8_t blueprintLOD = 0;
    uint8_t clipboardPreviewLOD = 0;
    uint8_t pastePreviewLOD = 0;
    int uiScale = 1;

    IVec2 cursorUIPos = IVec2::Zero();
    IVec2 cursorPos = IVec2::Zero();
    IVec2 cursorPosPrev = IVec2::Zero(); // For checking if there was movement
    bool b_cursorMoved = false;

    uint8_t framesPerTick = 6; // Number of frames in a tick
    uint8_t tickFrame = framesPerTick - 1; // Evaluate on 0
    bool tickThisFrame;

    Gate gatePick = Gate::OR;
    Gate lastGate = Gate::OR;
    uint8_t storedExtraParam = 0;
    ElbowConfig currentWireElbowConfig = ElbowConfig::horizontal;

    std::string deviceParameterTextFmt;

    Node* hoveredNode = nullptr;
    Wire* hoveredWire = nullptr;
    Group* hoveredGroup = nullptr;

    Blueprint* clipboard = nullptr;

    int propertyNumber; // For the "PushProperty" functions
    IRect propertiesPaneRec;

    double timeOfLastLog = 0.0;
    LogType minLogLevel = LogType::info; // Allows user to surpress logs below this level
    std::string consoleOutput[6];
    IRect consolePaneRec;

    IconButton modeButtons[4];
    const IconButton* ButtonFromMode(Mode mode) const;
    IconButton gateButtons[9];
    const IconButton* ButtonFromGate(Gate gate) const;
    ColorButton paramButtons[10];
    const ColorButton* ButtonFromParameter(uint8_t param) const;
    IconButton blueprintsButton;
    IconButton clipboardButton;
    IconButton settingsButton;
    TextButton toolPaneSizeButton;
    TextButton propertiesToggleButton;
    TextButton consoleToggleButton;
    Button* const allButtons[29];
    IRect toolPaneRec;
    bool toolPaneSizeState = true;
    bool consoleOn = true;
    bool propertiesOn = true;
    bool selectionPreview = false;

private:

    Tool* base;
    Tool* overlay; // Menu modes are stored in here

public:

    int FontSize() const;
    IVec2 FontPadding() const;

    void DrawUIIcon(Texture2D iconSheet, IVec2 iconColRow, IVec2 pos, Color tint) const;

    inline Tool* GetBaseTool()
    {
        return base;
    }
    inline Tool* GetTool()
    {
        if (!!overlay)
            return overlay;
        return base;
    }
    Mode GetBaseMode();
    Mode GetMode();
    ModeType GetModeType();

    void SetMode(Mode newMode);
    void SetGate(Gate newGate);

    void ClearOverlayMode();

    void IncrementTick();

    void UpdateCursorPos();

    // Zoom/pan
    void UpdateCamera();

    void UpdateSize();

    void CopySelectionToClipboard();

    void MakeGroupFromSelection();

    bool IsSelectionRectValid() const;

    void SaveBlueprint();

    void DrawClipboardPreview() const;
    bool IsClipboardValid() const;
    void ClearClipboard();
    bool SelectionExists() const;
    void ClearSelection();
    void DestroySelection();

    void CheckHotkeys();

    IVec2 GetCursorDelta() const;

    bool CursorInBounds(IRect bounds) const;
    bool CursorInUIBounds(IRect uiBounds) const;

    void DrawGrid(int gridSize = g_gridSize) const;

    IRect GetSelectionBounds(const std::vector<Node*>& vec) const;
    IRect GetSelectionBounds() const;

    static Color ResistanceBandColor(uint8_t index);
    Color ExtraParamColor() const;

    void DrawTooltipAtCursor(const std::string& text, Color color);

    void DrawTooltipAtCursor_Shadowed(const std::string& text, Color color);

    void ReloadPanes();
    void SaveConfig() const;
    void ReloadConfig();

    void UpdateTool();
    void DrawTool();

    void ReloadToolPane();
    void ToggleToolPaneSize();
    void DrawToolPane();

    void ToggleProperties();
    void ToggleConsole();

    void CleanConsolePane();
    void DrawConsoleOutput();
    void Log(LogType type, const std::string& output);
    void ClearLog();

    void CleanPropertiesPane();
    void PushProperty(const std::string& name, const std::string& value);
    inline void PushProperty_int(const std::string& name, int value)
    {
        PushProperty(name, TextFormat("%i", value));
    }
    inline void PushProperty_uint(const std::string& name, size_t value)
    {
        PushProperty(name, TextFormat("%u", value));
    }
    inline void PushProperty_ptr(const std::string& name, void* value)
    {
        PushProperty(name, TextFormat("0x%p", value));
    }
    inline void PushProperty_str(const std::string& name, const std::string& value)
    {
        PushProperty(name, value);
    }
    void PushProperty_longStr(const std::string& name, const std::string& value);
    inline void PushProperty_bool(const std::string& name, bool value)
    {
        PushProperty(name, value ? "true" : "false");
    }
    void PushPropertyTitle(const std::string& title);
    void PushPropertySubtitle(const std::string& title, Color color = UIColor(UIColorID::UI_COLOR_FOREGROUND));
    void PushPropertySpacer();
    void PushPropertySection_Node(const std::string& name, Node* value);
    void PushPropertySection_Wire(const std::string& name, Wire* value);
    void PushPropertySection_Selection(const std::string& name, const std::vector<Node*>& value);
    void PushPropertySection_Group(const std::string& name, Group* value);
    void DrawToolProperties();

    IVec2 WindowExtents() const;
};
