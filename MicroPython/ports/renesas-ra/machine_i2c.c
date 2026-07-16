/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016-2018 Damien P. George
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

#include "py/runtime.h"
#include "py/mphal.h"
#include "py/mperrno.h"
#include "extmod/modmachine.h"

#if MICROPY_PY_MACHINE_I2C

#define I2C_POLL_DEFAULT_TIMEOUT_US (50000) // 50ms

typedef struct _machine_hard_i2c_obj_t {
    mp_obj_base_t base;
    i2c_master_ctrl_t* ctrl;
    i2c_master_cfg_t* cfg;
    uint8_t i2c_id;
    uint32_t freq;
} machine_hard_i2c_obj_t;


static void machine_hard_i2c_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
     machine_hard_i2c_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "I2C(id=%u, freq=%u)", self->i2c_id, self->freq);
}

int machine_hard_i2c_transfer(mp_obj_base_t *self_in, uint16_t addr, size_t n, mp_machine_i2c_buf_t *bufs, unsigned int flags) {
    machine_hard_i2c_obj_t *self = MP_OBJ_TO_PTR(self_in);

    // 计算长度
    size_t remain_len = 0;
    for (size_t i = 0; i < n; ++i) {
        remain_len += bufs[i].len;
    }
    // 设置地址
    int ret = fsp_i2c_setAddress(self->ctrl, addr, self->cfg->addr_mode);
    if (ret < 0) {
        return ret;
    }
    // 分批发送 剩余长度大于0时需要发送restart信号
    int num_acks = 0; // only valid for write; for read it'll be 0
    for (; n--; ++bufs) {
        remain_len -= bufs->len;
        if (flags & MP_MACHINE_I2C_FLAG_READ) {
            ret = fsp_i2c_read(self->ctrl, bufs->buf, bufs->len, remain_len);
        } else {
            ret = fsp_i2c_write(self->ctrl, bufs->buf, bufs->len, remain_len);
        }
        if (ret < 0) {
            return ret;
        }
        num_acks ++;
    }

    return num_acks;
}


/******************************************************************************/
/* MicroPython bindings for machine API                                       */


mp_obj_t machine_hard_i2c_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {

    // parse args
    enum { ARG_id, ARG_freq };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_id,   MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_freq, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 400000} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    // 分配内存
    machine_hard_i2c_obj_t *self = mp_obj_malloc(machine_hard_i2c_obj_t, &machine_i2c_type);
    self->i2c_id = args[ARG_id].u_int;
    if (self->i2c_id < 0 || self->i2c_id >= RA_I2C_MASTER_NUM) {
		mp_raise_ValueError(MP_ERROR_TEXT("invalid I2C id"));
	}
    // 查表
    self->ctrl = ra_i2c_master_table[self->i2c_id];
    self->cfg = &ra_i2c_master_cfg_table[self->i2c_id];
    // 设置频率
    self->freq = args[ARG_freq].u_int;
    if (self->freq <= 100000) {
        self->cfg->rate = I2C_MASTER_RATE_STANDARD;
        self->freq = 100000;
    } else if (self->freq <= 400000) {
        self->cfg->rate = I2C_MASTER_RATE_FAST;
        self->freq = 400000;
    } else if (self->freq <= 1000000) {
        self->cfg->rate = I2C_MASTER_RATE_FASTPLUS;
        self->freq = 1000000;
    } else if (self->freq <= 3400000) {
        self->cfg->rate = I2C_MASTER_RATE_HIGHSPEED;
        self->freq = 3400000;
    } else {
        mp_raise_ValueError(MP_ERROR_TEXT("invalid I2C frequency"));
    }
    if(fsp_i2c_config(self->ctrl, self->cfg) != 0) {
        mp_raise_ValueError(MP_ERROR_TEXT("I2C config failed"));
    }
    return MP_OBJ_FROM_PTR(self);
}

static const mp_machine_i2c_p_t machine_hard_i2c_p = {
    .transfer = machine_hard_i2c_transfer,
};

MP_DEFINE_CONST_OBJ_TYPE(
    machine_i2c_type,
    MP_QSTR_I2C,
    MP_TYPE_FLAG_NONE,
    make_new, machine_hard_i2c_make_new,
    print, machine_hard_i2c_print,
    protocol, &machine_hard_i2c_p,
    locals_dict, &mp_machine_i2c_locals_dict
    );

#endif // MICROPY_HW_ENABLE_HW_I2C
