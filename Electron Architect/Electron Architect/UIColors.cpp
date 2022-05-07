#include "UIColors.h"

Color uiColors[18];

Color& UIColor(UIColorID id)
{
	return uiColors[(unsigned)id];
}
