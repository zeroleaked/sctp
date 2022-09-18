#include "conc_blank_state.h"

SctpState& ConcBlank::getInstance()
{
	static ConcBlank singleton;
	return singleton;
}