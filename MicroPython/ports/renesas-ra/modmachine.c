#include "modmachine.h"


#define MICROPY_PY_MACHINE_EXTRA_GLOBALS_BASE \
        { MP_ROM_QSTR(MP_QSTR_Pin), MP_ROM_PTR(&machine_pin_type) },

#if RA_RTC
#define MICROPY_PY_MACHINE_EXTRA_GLOBALS_RTC \
        { MP_ROM_QSTR(MP_QSTR_RTC), MP_ROM_PTR(&machine_rtc_type) },
#else
#define MICROPY_PY_MACHINE_EXTRA_GLOBALS_RTC
#endif

#define MICROPY_PY_MACHINE_EXTRA_GLOBALS \
        MICROPY_PY_MACHINE_EXTRA_GLOBALS_BASE \
        MICROPY_PY_MACHINE_EXTRA_GLOBALS_RTC

// idle()
// This executies a wfi machine instruction which reduces power consumption
// of the MCU until an interrupt occurs, at which point execution continues.
static void mp_machine_idle(void) {
    __WFI();
}
