#include "conc_sample_state.h"

SctpState& ConcSample::getInstance()
{
	static ConcSample singleton;
	return singleton;
}