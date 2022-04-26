#include "ProgramData.h"
#include "ToolModes.h"

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
