/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2018 Damien P. George
 * Copyright (c) 2021,2022 Renesas Electronics Corporation
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

#include <stdio.h>
#include <string.h>

#include "py/runtime.h"
#include "py/mphal.h"
#include "py/mperrno.h"
#include "extmod/modmachine.h"
#include "mphalport.h"

typedef struct _machine_hard_spi_obj_t {
    mp_obj_base_t base;
    spi_ctrl_t* ctrl;
    spi_cfg_t* cfg;
    uint8_t spi_id;
    uint8_t polarity;
    uint8_t phase;
    uint8_t bits;
    uint8_t firstbit;
//    mp_hal_pin_obj_t sck;
//    mp_hal_pin_obj_t mosi;
//    mp_hal_pin_obj_t miso;
} machine_hard_spi_obj_t;
#if MICROPY_PY_MACHINE_SPI

void spi_set_params(spi_cfg_t* cfg, int polarity, int phase, int firstbit){
	if(polarity != -1){
		cfg->clk_polarity = polarity;
	}
	if(phase != -1){
		cfg->clk_phase = phase;
	}
	if(firstbit != -1){
		cfg->bit_order = firstbit;
	}
}

/******************************************************************************/
// Implementation of hard SPI for machine module

static void machine_hard_spi_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
	machine_hard_spi_obj_t *self = MP_OBJ_TO_PTR(self_in);
	const char *firstbit_str = (self->firstbit == MICROPY_PY_MACHINE_SPI_MSB) ? "MSB" : "LSB";
	mp_printf(print, "SPI(id=%u, polarity=%u, phase=%u, bits=%u, firstbit=%s)",
			  self->spi_id, self->polarity, self->phase, self->bits, firstbit_str);
}

mp_obj_t machine_hard_spi_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
//    MP_MACHINE_SPI_CHECK_FOR_LEGACY_SOFTSPI_CONSTRUCTION(n_args, n_kw, all_args);

    enum { ARG_id, ARG_baudrate, ARG_polarity, ARG_phase, ARG_bits, ARG_firstbit, ARG_sck, ARG_mosi, ARG_miso };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_id,       MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_baudrate, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_int = MP_OBJ_NULL} },
        { MP_QSTR_polarity, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_phase,    MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_bits,     MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 8} },
        { MP_QSTR_firstbit, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = MICROPY_PY_MACHINE_SPI_MSB} },
        { MP_QSTR_sck,      MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_mosi,     MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_miso,     MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);



    // here we would check the sck/mosi/miso pins and configure them, but it's not implemented
	if (args[ARG_baudrate].u_obj != MP_OBJ_NULL
		|| args[ARG_sck].u_obj != MP_OBJ_NULL
		|| args[ARG_mosi].u_obj != MP_OBJ_NULL
		|| args[ARG_miso].u_obj != MP_OBJ_NULL) {
		mp_raise_ValueError(MP_ERROR_TEXT("explicit choice of baudrate/sck/mosi/misois not implemented"));
	}

    machine_hard_spi_obj_t *self = mp_obj_malloc(machine_hard_spi_obj_t, &machine_spi_type);
    // get static peripheral object
    self->spi_id = args[ARG_id].u_int;
    if (self->spi_id < 0 || self->spi_id >= RA_SPI_NUM) {
		mp_raise_ValueError(MP_ERROR_TEXT("invalid SPI id"));
	}
    // 查表
    self->ctrl = ra_spi_table[self->spi_id];
    self->cfg = &ra_spi_cfg_table[self->spi_id];
    // 设置cfg
    spi_set_params(self->cfg, args[ARG_polarity].u_int, args[ARG_phase].u_int, args[ARG_firstbit].u_int);
    // 配置spi
    int ret = fsp_spi_config(self->ctrl, self->cfg);
    if (ret != 0) {
        mp_raise_OSError(-ret);
    }
    self->polarity = args[ARG_polarity].u_int;
    self->phase = args[ARG_phase].u_int;
    self->bits = args[ARG_bits].u_int;
    self->firstbit = args[ARG_firstbit].u_int;
    return MP_OBJ_FROM_PTR(self);
}

static void machine_hard_spi_init(mp_obj_base_t *self_in, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    machine_hard_spi_obj_t *self = (machine_hard_spi_obj_t *)self_in;

    enum { ARG_baudrate, ARG_polarity, ARG_phase, ARG_bits, ARG_firstbit };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_baudrate, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_int = MP_OBJ_NULL} },
        { MP_QSTR_polarity, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_phase,    MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_bits,     MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_firstbit, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    if (args[ARG_baudrate].u_obj != MP_OBJ_NULL
		|| args[ARG_bits].u_int != 8) {
		mp_raise_ValueError(MP_ERROR_TEXT("explicit choice of baudrate/bits is not implemented"));
	}
    // 查表
    spi_ctrl_t* ctrl = self->ctrl;
    spi_cfg_t* cfg = self->cfg;
    // 设置cfg
    spi_set_params(cfg, args[ARG_polarity].u_int, args[ARG_phase].u_int, args[ARG_firstbit].u_int);
    // 配置spi
    int ret = fsp_spi_config(ctrl, cfg);
    if (ret != 0) {
        mp_raise_OSError(-ret);
    }
    if(args[ARG_polarity].u_int != -1){
    	self->polarity = args[ARG_polarity].u_int;
    }
    if(args[ARG_phase].u_int != -1){
    	self->phase = args[ARG_phase].u_int;
    }
    if(args[ARG_bits].u_int != -1){
    	self->bits = args[ARG_bits].u_int;
    }
    if(args[ARG_firstbit].u_int != -1){
		self->firstbit = args[ARG_firstbit].u_int;
	}
}

static void machine_hard_spi_deinit(mp_obj_base_t *self_in) {
    machine_hard_spi_obj_t *self = (machine_hard_spi_obj_t *)self_in;
    int ret = fsp_spi_close(self->ctrl);
    if (ret != 0) {
		mp_raise_OSError(-ret);
	}
}

static void machine_hard_spi_transfer(mp_obj_base_t *self_in, size_t len, const uint8_t *src, uint8_t *dest) {
    machine_hard_spi_obj_t *self = (machine_hard_spi_obj_t *)self_in;
    int ret = fsp_spi_transfer(self->ctrl, len, src, dest, self->bits);
    if (ret != 0) {
		mp_raise_OSError(-ret);
	}
}

static const mp_machine_spi_p_t machine_hard_spi_p = {
    .init = machine_hard_spi_init,
    .deinit = machine_hard_spi_deinit,
    .transfer = machine_hard_spi_transfer,
};

MP_DEFINE_CONST_OBJ_TYPE(
    machine_spi_type,
    MP_QSTR_SPI,
    MP_TYPE_FLAG_NONE,
    make_new, machine_hard_spi_make_new,
    print, machine_hard_spi_print,
    protocol, &machine_hard_spi_p,
    locals_dict, &mp_machine_spi_locals_dict
    );

#endif // MICROPY_PY_MACHINE_SPI

