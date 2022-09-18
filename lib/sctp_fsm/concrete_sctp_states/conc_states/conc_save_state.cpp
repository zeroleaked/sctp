#include "conc_save_state.h"

SctpState& ConcSave::getInstance()
{
	static ConcSave singleton;
	return singleton;
}