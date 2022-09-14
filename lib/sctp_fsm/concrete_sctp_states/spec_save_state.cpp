#include "spec_save_state.h"

SctpState& SpecSave::getInstance()
{
	static SpecSave singleton;
	return singleton;
}