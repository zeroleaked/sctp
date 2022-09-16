#include "conc_table_state.h"

SctpState& ConcTable::getInstance()
{
	static ConcTable singleton;
	return singleton;
}