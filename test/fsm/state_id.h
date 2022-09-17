#pragma once

typedef enum {
    NONE,
    STATE_IDLE,
    STATE_MENU,
    STATE_SPEC_BLANK,
    STATE_CONC_CURVES,
    STATE_HISTORY,
    STATE_SETTINGS,
    STATE_SPEC_SAMPLE,
    STATE_SPEC_RESULT,
    STATE_SPEC_SAVE,
    STATE_CONC_TABLE

} state_id_t;