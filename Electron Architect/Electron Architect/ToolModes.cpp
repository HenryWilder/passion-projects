#include "ProgramData.h"
#include "ToolModes.h"
#include "HUtility.h"

#define CREATE_AS_METHOD_FOR_TOOL(extension, name) \
	extension##_##name* extension##_##name::As(Mode mode) { _ASSERT_EXPR(mode == GetMode(), L"Tried to access non-" WSTRINGIFY(name) L" member from " WSTRINGIFY(name)); return this; }

CREATE_AS_METHOD_FOR_TOOL(Tool,    Pen)
CREATE_AS_METHOD_FOR_TOOL(Tool,    Edit)
CREATE_AS_METHOD_FOR_TOOL(Tool,    Erase)
CREATE_AS_METHOD_FOR_TOOL(Tool,    Interact)
CREATE_AS_METHOD_FOR_TOOL(Overlay, Button)
CREATE_AS_METHOD_FOR_TOOL(Overlay, Paste)
CREATE_AS_METHOD_FOR_TOOL(Menu,    Icon)
CREATE_AS_METHOD_FOR_TOOL(Menu,    Select)