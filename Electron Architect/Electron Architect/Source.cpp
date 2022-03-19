#include <vector>
#include <list>
#include <algorithm>
#include <raylib.h>
#include <extras\raygui.h>
#include <extras\easings.h>

using IVecInt_t = int;

struct IVec2
{
    IVec2() = default;
    constexpr IVec2(IVecInt_t x, IVecInt_t y) : x(x), y(y) {}

    IVecInt_t x;
    IVecInt_t y;

    bool operator==(IVec2 b) const
    {
        return x == b.x && y == b.y;
    }
    bool operator!=(IVec2 b) const
    {
        return x != b.x || y != b.y;
    }
};

IVec2 IVec2Zero()
{
    constexpr IVec2 null(0,0);
    return null;
}

IVec2 operator+(IVec2 a, IVec2 b)
{
    return IVec2(a.x + b.x, a.y + b.y);
}
IVec2 operator-(IVec2 a, IVec2 b)
{
    return IVec2(a.x - b.x, a.y - b.y);
}
IVec2 operator*(IVec2 a, IVec2 b)
{
    return IVec2(a.x * b.x, a.y * b.y);
}
IVec2 operator/(IVec2 a, IVec2 b)
{
    return IVec2(a.x / b.x, a.y / b.y);
}

IVec2 IVec2Scale_i(IVec2 a, int b)
{
    return IVec2(a.x * b, a.y * b);
}
inline IVec2 IVec2Scale_i(int a, IVec2 b)
{
    return IVec2Scale_i(b, a);
}
IVec2 IVec2Scale_f(IVec2 a, float b)
{
    return IVec2((IVecInt_t)((float)a.x * b), (IVecInt_t)((float)a.y * b));
}
inline IVec2 IVec2Scale_f(float a, IVec2 b)
{
    return IVec2Scale_f(b, a);
}


class Node;

struct Wire
{
    Wire() = default;
    Wire(Node * start, Node * end) : elbow(), start(start), end(end) {}

    static constexpr float g_elbowRadius = 1.0f;
    IVec2 elbow;
    Node* start;
    Node* end;
};

enum class Gate
{
    OR = '|',
    AND = '&',
    NOR = '!',
    XOR = '^',
};

struct Node
{
    Node() = default;
    Node(IVec2 position, Gate gate) : position(position), gate(gate) {}

    static constexpr float g_nodeRadius = 1.0f;
    IVec2 position;
    Gate gate;
    std::vector<Wire*> wires;
};

void SortBFS(std::vector<Node*>::iterator _First, std::vector<Node*>::iterator _Last)
{
    size_t nodeCount = std::distance(_First, _Last);
    bool* visited = new bool[nodeCount](false);
    std::vector<Node*>::iterator it = _First;
    while (it != _Last)
    {

    }
}

class NodeWorld
{
private:
    std::vector<Wire*> wires;
    std::vector<Node*> nodes;

public:
    void InsertNode(Node* node);
    void RemoveNode(Node* node);
};


int main()
{
    int windowWidth = 1280;
    int windowHeight = 720;
    InitWindow(windowWidth, windowHeight, "Electron Architect");
    SetTargetFPS(60);

    /******************************************
    *   Load textures, shaders, and meshes
    ******************************************/

    // TODO: Load persistent assets & variables

    while (!WindowShouldClose())
    {
        /******************************************
        *   Simulate frame and update variables
        ******************************************/

        // TODO: simulate frame

        /******************************************
        *   Draw the frame
        ******************************************/

        BeginDrawing(); {

            ClearBackground(BLACK);

            // TODO: Draw frame

        } EndDrawing();
    }

    /******************************************
    *   Unload and free memory
    ******************************************/

    // TODO: Unload variables

    CloseWindow();

	return 0;
}

