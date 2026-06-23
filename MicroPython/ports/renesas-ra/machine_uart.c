/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2023 Damien P. George
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

// This file is never compiled standalone, it's included directly from
// extmod/machine_uart.c via MICROPY_PY_MACHINE_UART_INCLUDEFILE.

#include "py/runtime.h"
#include "py/mperrno.h"
#include "py/mphal.h"



typedef struct _machine_uart_obj_t {
    mp_obj_base_t base;

    uart_ctrl_t* ctrl;
	uart_cfg_t* cfg;

    uint8_t uart_id;
    uint32_t baudrate;
    uint8_t bits;
    uint8_t parity;
    uint8_t stop;

} machine_uart_obj_t;

static const char *parity_names[] = {"None", "EVEN", "ODD"};

static void mp_machine_uart_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
	machine_uart_obj_t *self = MP_OBJ_TO_PTR(self_in);
	mp_printf(print, "UART(%u, baudrate=%u, bits=%u, parity=%s, stop=%u)",
		self->uart_id, self->baudrate, self->bits,
		parity_names[self->parity], self->stop);
}

static void mp_machine_uart_init_helper(machine_uart_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_baudrate, ARG_bits, ARG_parity, ARG_stop };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_baudrate, MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_bits, MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_parity, MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_stop, MP_ARG_INT, {.u_int = -1} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    int ret;

    // 其他设置
    if (args[ARG_bits].u_int == 7) {
    	self->cfg->data_bits = UART_DATA_BITS_7;
    	self->bits = args[ARG_bits].u_int;
    } else if (args[ARG_bits].u_int == 8) {
    	self->cfg->data_bits = UART_DATA_BITS_8;
    	self->bits = args[ARG_bits].u_int;
    } else if (args[ARG_bits].u_int == -1){
    	// 保持默认值
    } else {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("invalid data bits"));
    }
//    else if (args[ARG_bits].u_int == 9) {
//        	self->cfg->data_bits = UART_DATA_BITS_9;
//        	self->bits = args[ARG_bits].u_int;
//        }
    if (args[ARG_parity].u_int == 0) {
        self->cfg->parity = UART_PARITY_OFF;
        self->parity = 0;
    } else if (args[ARG_parity].u_int == 1) {
    	self->cfg->parity = UART_PARITY_EVEN;
    	self->parity = 1;
    } else if (args[ARG_parity].u_int == 2) {
    	self->cfg->parity = UART_PARITY_ODD;
    	self->parity = 2;
    } else if (args[ARG_parity].u_int == -1){
    	// 保持默认值
    } else {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("invalid parity"));
    }

    if (args[ARG_stop].u_int == 1) {
        self->cfg->stop_bits = UART_STOP_BITS_1;
        self->stop = 1;
    } else if (args[ARG_stop].u_int == 2) {
    	self->cfg->stop_bits = UART_STOP_BITS_2;
    	self->stop = 2;
    } else if (args[ARG_stop].u_int == -1){
    	// 保持默认值
    } else {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("invalid stop bits"));
    }
    ret = fsp_uart_config(self->ctrl, self->cfg);
    if (ret != 0) {
   		mp_raise_OSError(-ret);
   	}

    // 设置波特率
	if (args[ARG_baudrate].u_int != -1) {
		ret = fsp_uart_set_baudrate(self->ctrl, args[ARG_baudrate].u_int);
		if (ret != 0) mp_raise_OSError(-ret);
		self->baudrate = args[ARG_baudrate].u_int;
	}
}

static mp_obj_t mp_machine_uart_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);
    enum { ARG_id, ARG_baudrate, ARG_bits, ARG_parity, ARG_stop };
	static const mp_arg_t allowed_args[] = {
		{ MP_QSTR_id, MP_ARG_REQUIRED | MP_ARG_INT },
		{ MP_QSTR_baudrate, MP_ARG_INT, {.u_int = 115200} },
		{ MP_QSTR_bits, MP_ARG_INT, {.u_int = 8} },
		{ MP_QSTR_parity, MP_ARG_INT, {.u_int = 0} },
		{ MP_QSTR_stop, MP_ARG_INT, {.u_int = 1} },
	};
	mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
	mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    machine_uart_obj_t *self = mp_obj_malloc(machine_uart_obj_t, &machine_uart_type);
    self->uart_id = args[ARG_id].u_int;
	if (self->uart_id < 0 || self->uart_id >= RA_UART_NUM) {
		mp_raise_ValueError(MP_ERROR_TEXT("invalid UART id"));
	}
	// 查表
	self->ctrl = ra_uart_table[self->uart_id];
	self->cfg = &ra_uart_cfg_table[self->uart_id];
	self->cfg->p_context = &self->uart_id;

	fsp_uart_buf_init(self->uart_id);

	int ret;
	// 其他设置
	if (args[ARG_bits].u_int == 7) {
		self->cfg->data_bits = UART_DATA_BITS_7;
		self->bits = args[ARG_bits].u_int;
	} else if (args[ARG_bits].u_int == 8) {
		self->cfg->data_bits = UART_DATA_BITS_8;
		self->bits = args[ARG_bits].u_int;
	} else {
		mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("invalid data bits"));
	}
//	else if (args[ARG_bits].u_int == 9) {
//		self->cfg->data_bits = UART_DATA_BITS_9;
//		self->bits = args[ARG_bits].u_int;
//	}
	if (args[ARG_parity].u_int == 0) {
		self->cfg->parity = UART_PARITY_OFF;
		self->parity = 0;
	} else if (args[ARG_parity].u_int == 1) {
		self->cfg->parity = UART_PARITY_EVEN;
		self->parity = 1;
	} else if (args[ARG_parity].u_int == 2) {
		self->cfg->parity = UART_PARITY_ODD;
		self->parity = 2;
	} else {
		mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("invalid parity"));
	}

	if (args[ARG_stop].u_int == 1) {
		self->cfg->stop_bits = UART_STOP_BITS_1;
		self->stop = 1;
	} else if (args[ARG_stop].u_int == 2) {
		self->cfg->stop_bits = UART_STOP_BITS_2;
		self->stop = 2;
	} else {
		mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("invalid stop bits"));
	}
	ret = fsp_uart_config(self->ctrl, self->cfg);
	if (ret != 0) {
		mp_raise_OSError(-ret);
	}

	// 设置波特率
	ret = fsp_uart_set_baudrate(self->ctrl, args[ARG_baudrate].u_int);
	if (ret != 0) {
		mp_raise_OSError(-ret);
	}
	self->baudrate = args[ARG_baudrate].u_int;

    return MP_OBJ_FROM_PTR(self);
}

static void mp_machine_uart_deinit(machine_uart_obj_t *self) {
    int ret = fsp_uart_close(self->ctrl);
	if (ret != 0) {
		mp_raise_OSError(-ret);
	}
}

static mp_int_t mp_machine_uart_any(machine_uart_obj_t *self) {
    return fsp_uart_rx_avail(self->uart_id);
}

static bool mp_machine_uart_txdone(machine_uart_obj_t *self) {
    return fsp_uart_tx_complete(self->ctrl);
}

static mp_uint_t mp_machine_uart_read(mp_obj_t self_in, void *buf_in, mp_uint_t size, int *errcode) {
    machine_uart_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint8_t *buffer = (uint8_t *)buf_in;
    uint32_t bytes_read = 0;
    return fsp_uart_read(self->uart_id, buffer, size);
}

static mp_uint_t mp_machine_uart_write(mp_obj_t self_in, const void *buf_in, mp_uint_t size, int *errcode) {
    machine_uart_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint8_t *buffer = (uint8_t *)buf_in;

    // wait for any pending transmission to complete
    while (!mp_machine_uart_txdone(self)) {

    }
    // 阻塞发送数据
    fsp_uart_write(self->ctrl, buf_in, size);
    return size;
}

static mp_uint_t mp_machine_uart_ioctl(mp_obj_t self_in, mp_uint_t request, uintptr_t arg, int *errcode) {
    machine_uart_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_uint_t ret = 0;

    if (request == MP_STREAM_POLL) {
        uintptr_t flags = arg;
        if ((flags & MP_STREAM_POLL_RD) && (mp_machine_uart_any(self) > 0)) {
            ret |= MP_STREAM_POLL_RD;
        }
        if ((flags & MP_STREAM_POLL_WR) && mp_machine_uart_txdone(self)) {
            ret |= MP_STREAM_POLL_WR;
        }
    } else if (request == MP_STREAM_FLUSH) {
        while (!mp_machine_uart_txdone(self)) {
        }
    } else {
        *errcode = MP_EINVAL;
        ret = MP_STREAM_ERROR;
    }

    return ret;
}

