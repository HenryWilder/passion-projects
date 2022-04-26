#include "ProgramData.h"
#include "ToolModes.h"
#include "HUtility.h"

#if _DEBUG
ModeHandler::ModeHandler(ProgramData& _data)
{
	static int g_timesInitialized = 0;
	_ASSERT_EXPR(g_timesInitialized == 0, L"Cannot initialize mode handler multiple times");
	data = _data;
	++g_timesInitialized;
}
#else
ModeHandler::ModeHandler(ProgramData& _data) : data(_data) {}
#endif // _DEBUG

#define USE_MACRO // I like compressing code

#ifdef USE_MACRO
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
#else
      Tool_Pen*	      Tool_Pen::As(Mode mode) { _ASSERT_EXPR(mode == GetMode(), L"Tried to access non-" L"pen"      L" member from " L"pen"     ); return this; }
     Tool_Edit*	     Tool_Edit::As(Mode mode) { _ASSERT_EXPR(mode == GetMode(), L"Tried to access non-" L"edit"     L" member from " L"edit"    ); return this; }
    Tool_Erase*	    Tool_Erase::As(Mode mode) { _ASSERT_EXPR(mode == GetMode(), L"Tried to access non-" L"erase"    L" member from " L"erase"   ); return this; }
 Tool_Interact*	 Tool_Interact::As(Mode mode) { _ASSERT_EXPR(mode == GetMode(), L"Tried to access non-" L"interact" L" member from " L"interact"); return this; }
Overlay_Button* Overlay_Button::As(Mode mode) { _ASSERT_EXPR(mode == GetMode(), L"Tried to access non-" L"button"   L" member from " L"button"  ); return this; }
 Overlay_Paste*	 Overlay_Paste::As(Mode mode) { _ASSERT_EXPR(mode == GetMode(), L"Tried to access non-" L"paste"    L" member from " L"paste"   ); return this; }
     Menu_Icon*	     Menu_Icon::As(Mode mode) { _ASSERT_EXPR(mode == GetMode(), L"Tried to access non-" L"icon"     L" member from " L"icon"    ); return this; }
   Menu_Select*	   Menu_Select::As(Mode mode) { _ASSERT_EXPR(mode == GetMode(), L"Tried to access non-" L"select"   L" member from " L"select"  ); return this; }
#endif