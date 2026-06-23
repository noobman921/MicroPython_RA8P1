#ifndef MICROPYTHON_PORTS_RENESAS_MPHALPORT_H_
#define MICROPYTHON_PORTS_RENESAS_MPHALPORT_H_
#include "irq.h"
#include "modmachine.h"
#include "drivers/fsp_gpio.h"

#define MICROPY_PY_PENDSV_ENTER   uint32_t atomic_state = raise_irq_pri(IRQ_PRI_PENDSV)
#define MICROPY_PY_PENDSV_EXIT    restore_irq_pri(atomic_state)


#define mp_hal_pin_obj_t        const machine_pin_obj_t *
//#define mp_hal_get_pin_obj(o)   //未实现
//#define mp_hal_pin_name(p)      //未实现
#define mp_hal_pin_input(p)     fsp_gpio_config(p, IOPORT_CFG_PORT_DIRECTION_INPUT)
#define mp_hal_pin_output(p)    fsp_gpio_config(p, IOPORT_CFG_PORT_DIRECTION_OUTPUT)
//#define mp_hal_pin_open_drain(p)    ra_gpio_config((p)->pin, MP_HAL_PIN_MODE_OPEN_DRAIN, MP_HAL_PIN_PULL_NONE, MP_HAL_PIN_DRIVE_0, 0)
#define mp_hal_pin_high(p)      fsp_gpio_write(p, 1)
#define mp_hal_pin_low(p)       fsp_gpio_write(p, 0)
#define mp_hal_pin_toggle(p)    fsp_gpio_toggle(p)
//#define mp_hal_pin_od_low(p)    mp_hal_pin_low(p)
//#define mp_hal_pin_od_high(p)   mp_hal_pin_high(p)
#define mp_hal_pin_read(p)      fsp_gpio_read(p)
#define mp_hal_pin_write(p, v)  do { if (v) { mp_hal_pin_high(p); } else { mp_hal_pin_low(p); } } while (0)


static inline void mp_hal_set_interrupt_char(char c) {
}

#endif
