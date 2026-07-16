/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Damien P. George
 * Copyright (c) 2023 Vekatech Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

// This file is never compiled standalone, it's included directly from
// extmod/machine_pwm.c via MICROPY_PY_MACHINE_PWM_INCLUDEFILE.

#include "py/mphal.h"
#include "py/mperrno.h"

typedef struct _machine_pwm_obj_t {
    mp_obj_base_t base;
    timer_ctrl_t* ctrl;
    timer_cfg_t* cfg;
    uint8_t pwm_id;
    uint8_t duty;
    uint32_t duty_u16;
    uint32_t duty_ns;
    uint32_t freq;
} machine_pwm_obj_t;


/******************************************************************************/
// MicroPython bindings for PWM

static void mp_machine_pwm_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_pwm_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "PWM(id=%u, freq=%u, duty_u16=%u, duty_ns=%u, duty=%u) note: duty is correct all time, duty_u16 and duty_ns may be not correct if another changed last time",
              self->pwm_id, self->freq, self->duty_u16, self->duty_ns, self->duty);
}

static void mp_machine_pwm_init_helper(machine_pwm_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {

    enum { ARG_id, ARG_freq, ARG_duty_u16, ARG_duty_ns };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_id,  MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_freq, MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_duty_u16, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_duty_ns, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} }
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // 修改频率
    if (args[ARG_freq].u_int != -1) {
        if ((args[ARG_freq].u_int < 0) || (args[ARG_freq].u_int > 24000000)) {
            mp_raise_ValueError(MP_ERROR_TEXT("freq should be 0-24000000"));
        } else {
            int32_t ret = fsp_pwm_setFreq(self->ctrl, args[ARG_freq].u_int);
            if (ret != 0) {
                mp_raise_ValueError(MP_ERROR_TEXT("failed to set freq"));
            }
            self->freq = args[ARG_freq].u_int;
        }
    }

    // 设置默认值
    self->duty_u16 = 0;
    self->duty_ns = 0;

     // 设置占空比
    if(args[ARG_duty_u16].u_int != -1) {
        if ((args[ARG_duty_u16].u_int < 0) || (args[ARG_duty_u16].u_int > 65535)) {
            mp_raise_ValueError(MP_ERROR_TEXT("duty should be 0-65535"));
        } else {
            int32_t ret = fsp_pwm_setDuty(self->ctrl, args[ARG_duty_u16].u_int * 100 / 65535);
            if (ret != 0) {
                mp_raise_ValueError(MP_ERROR_TEXT("failed to set duty"));
            }
            self->duty = (uint8_t)((args[ARG_duty_u16].u_int * 100) / 65535);
            self->duty_u16 = args[ARG_duty_u16].u_int;
        }
    }
    else if(args[ARG_duty_ns].u_int != -1) {
        uint32_t period_counts = args[ARG_duty_ns].u_int *self->freq / 1000000000;
        uint32_t period = 0;
        fsp_pwm_getPeriod(self->ctrl, &period);
        if (period_counts > period) {
            mp_raise_ValueError(MP_ERROR_TEXT("duty should be in period range"));
        } else {
            int32_t ret = fsp_pwm_setDuty(self->ctrl, period_counts * 100 / period);
            if (ret != 0) {
                mp_raise_ValueError(MP_ERROR_TEXT("failed to set duty"));
            }
            self->duty = (uint8_t)(period_counts * 100 / period);
            self->duty_ns = args[ARG_duty_ns].u_int;
        }
    }
}

static mp_obj_t mp_machine_pwm_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    machine_pwm_obj_t *self = mp_obj_malloc(machine_pwm_obj_t, &machine_pwm_type);
    enum { ARG_id, ARG_freq, ARG_duty_u16, ARG_duty_ns };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_id,  MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_freq, MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_duty_u16, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_duty_ns, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} }
    };
    // 此处不设计默认值，是考虑Congiguration中已经设置了默认值，用户不传入时使用默认值即可
    mp_arg_check_num(n_args, n_kw, 1, 3, true);
    mp_arg_val_t init_args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, args, MP_ARRAY_SIZE(allowed_args), allowed_args, init_args);

    // 查表获取对象
    self->pwm_id = init_args[ARG_id].u_int;
    self->ctrl = ra_pwm_table[self->pwm_id];
    self->cfg = &ra_pwm_cfg_table[self->pwm_id];
    fsp_pwm_config(self->ctrl, self->cfg);

    // 设置频率
    if (init_args[ARG_freq].u_int != -1) {
        if ((init_args[ARG_freq].u_int < 0) || (init_args[ARG_freq].u_int > 24000000)) {
            mp_raise_ValueError(MP_ERROR_TEXT("freq should be 0-24000000"));
        } else {
            int32_t ret = fsp_pwm_setFreq(self->ctrl, init_args[ARG_freq].u_int);
            if (ret != 0) {
                mp_raise_ValueError(MP_ERROR_TEXT("failed to set freq"));
            }
            self->freq = init_args[ARG_freq].u_int;
        }
    }

    // 设置默认值
    self->duty_u16 = 0;
    self->duty_ns = 0;

    // 设置占空比
    if(init_args[ARG_duty_u16].u_int != -1) {
        if ((init_args[ARG_duty_u16].u_int < 0) || (init_args[ARG_duty_u16].u_int > 65535)) {
            mp_raise_ValueError(MP_ERROR_TEXT("duty should be 0-65535"));
        } else {
            int32_t ret = fsp_pwm_setDuty(self->ctrl, init_args[ARG_duty_u16].u_int * 100 / 65535);
            if (ret != 0) {
                mp_raise_ValueError(MP_ERROR_TEXT("failed to set duty"));
            }
            self->duty = (uint8_t)((init_args[ARG_duty_u16].u_int * 100) / 65535);
            self->duty_u16 = init_args[ARG_duty_u16].u_int;
        }
    }
    else if(init_args[ARG_duty_ns].u_int != -1) {
        uint32_t period_counts = init_args[ARG_duty_ns].u_int *self->freq / 1000000000;
        uint32_t period = 0;
        fsp_pwm_getPeriod(self->ctrl, &period);
        if (period_counts > period) {
            mp_raise_ValueError(MP_ERROR_TEXT("duty should be in period range"));
        } else {
            int32_t ret = fsp_pwm_setDuty(self->ctrl, period_counts * 100 / period);
            if (ret != 0) {
                mp_raise_ValueError(MP_ERROR_TEXT("failed to set duty"));
            }
            self->duty = (uint8_t)(period_counts * 100 / period);
            self->duty_ns = init_args[ARG_duty_ns].u_int;
        }
    }
    return MP_OBJ_FROM_PTR(self);
}

static void mp_machine_pwm_deinit(machine_pwm_obj_t *self) {
    int ret = fsp_pwm_close(self->ctrl);
    if (ret != 0) {
        mp_raise_ValueError(MP_ERROR_TEXT("failed to close pwm"));
    }
    self->pwm_id = 0;
    self->duty = 0;
    self->duty_u16 = 0;
    self->duty_ns = 0;
    self->freq = 0;
}

static mp_obj_t mp_machine_pwm_freq_get(machine_pwm_obj_t *self) {
    return MP_OBJ_NEW_SMALL_INT(self->freq);
}

static void mp_machine_pwm_freq_set(machine_pwm_obj_t *self, mp_int_t freq) {
    if ((freq < 0) || (freq > 24000000)) {
        mp_raise_ValueError(MP_ERROR_TEXT("freq should be 0-24000000"));
    } else {
        int32_t ret = fsp_pwm_setFreq(self->ctrl, freq);
        if (ret != 0) {
            mp_raise_ValueError(MP_ERROR_TEXT("failed to set freq"));
        }
        self->freq = freq;
    }
}

static void mp_machine_pwm_duty_set_ns(machine_pwm_obj_t *self, mp_int_t duty_ns) {
    uint32_t period_counts = duty_ns *self->freq / 1000000000;
    uint32_t period = 0;
    fsp_pwm_getPeriod(self->ctrl, &period);
    if (period_counts > period) {
        mp_raise_ValueError(MP_ERROR_TEXT("duty should be in period range"));
    } else {
        int32_t ret = fsp_pwm_setDuty(self->ctrl, period_counts * 100 / period);
        if (ret != 0) {
            mp_raise_ValueError(MP_ERROR_TEXT("failed to set duty"));
        }
        self->duty = (uint8_t)(period_counts * 100 / period);
        self->duty_ns = duty_ns;
    }
}

static mp_obj_t mp_machine_pwm_duty_get_ns(machine_pwm_obj_t *self) {
    return MP_OBJ_NEW_SMALL_INT(self->duty_ns);
}

static void mp_machine_pwm_duty_set_u16(machine_pwm_obj_t *self, mp_int_t duty_u16) {
    if ((duty_u16 < 0) || (duty_u16 > 65535)) {
        mp_raise_ValueError(MP_ERROR_TEXT("duty should be 0-65535"));
    } else {
        int32_t ret = fsp_pwm_setDuty(self->ctrl, duty_u16 * 100 / 65535);
        if (ret != 0) {
            mp_raise_ValueError(MP_ERROR_TEXT("failed to set duty"));
        }
        self->duty = (uint8_t)((duty_u16 * 100) / 65535);
        self->duty_u16 = duty_u16;
    }
}

static mp_obj_t mp_machine_pwm_duty_get_u16(machine_pwm_obj_t *self) {
    return MP_OBJ_NEW_SMALL_INT(self->duty_u16);
}
