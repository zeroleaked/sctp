#include "conc_regress_state.h"

SctpState& ConcRegress::getInstance()
{
	static ConcRegress singleton;
	return singleton;
}