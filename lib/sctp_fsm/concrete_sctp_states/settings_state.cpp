#include "settings_state.h"

SctpState& Settings::getInstance()
{
	static Settings singleton;
	return singleton;
}