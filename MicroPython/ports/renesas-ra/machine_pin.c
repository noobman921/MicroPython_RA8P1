/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016-2021 Damien P. George
 * Copyright (c) 2022 Renesas Electronics Corporation
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
#include <stdint.h>
#include <string.h>

#include "py/runtime.h"
#include "py/mphal.h"
#include "extmod/modmachine.h"
#include "extmod/virtpin.h"
#include "shared/runtime/mpirq.h"

#include "modmachine.h"
#include "drivers/fsp_gpio.h"
#include "mpy_board_cfg.h"

#define MP_HAL_PIN_MODE_INPUT       (0)
#define MP_HAL_PIN_MODE_OUTPUT      (1)


// pin.init(mode=-1, pull=-1, *, value=None, drive=0, alt=-1)
static mp_obj_t machine_pin_obj_init_helper(machine_pin_obj_t *self,
    size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    // 定义允许的参数
    enum { ARG_mode, ARG_pull, ARG_value, ARG_drive, ARG_alt };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_mode, MP_ARG_OBJ, {.u_rom_obj = MP_ROM_NONE} },
        { MP_QSTR_pull, MP_ARG_OBJ, {.u_rom_obj = MP_ROM_NONE} },
        { MP_QSTR_value, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_rom_obj = MP_ROM_NONE} },
        { MP_QSTR_drive, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_rom_obj = MP_ROM_NONE} },
        { MP_QSTR_alt, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // 检查不支持的可选参数
	if (args[ARG_pull].u_obj != mp_const_none) {
		mp_raise_ValueError(MP_ERROR_TEXT("pull mode not supported"));
	}
	if (args[ARG_drive].u_obj != mp_const_none) {
		mp_raise_ValueError(MP_ERROR_TEXT("drive not supported"));
	}
	if (args[ARG_alt].u_int != -1) {
		mp_raise_ValueError(MP_ERROR_TEXT("alt not supported"));
	}

    // get io mode
	uint32_t mode;
	if (args[ARG_mode].u_obj != mp_const_none) {
		mode = mp_obj_get_int(args[ARG_mode].u_obj);
		if(mode == MP_HAL_PIN_MODE_INPUT){
			fsp_gpio_config(self, IOPORT_CFG_PORT_DIRECTION_INPUT);
			self->mode = MP_HAL_PIN_MODE_INPUT;
		}
		else if(mode == MP_HAL_PIN_MODE_OUTPUT){
			fsp_gpio_config(self, IOPORT_CFG_PORT_DIRECTION_OUTPUT);
			self->mode = MP_HAL_PIN_MODE_OUTPUT;
		}
		else{
			mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("invalid pin mode: %d"), mode);
		}
	} else {
		// 默认INPUT
		fsp_gpio_config(self, IOPORT_CFG_PORT_DIRECTION_INPUT);
	}

	if (args[ARG_value].u_obj != mp_const_none) {
		bool val = mp_obj_is_true(args[ARG_value].u_obj);
		fsp_gpio_write(self, val);
	}

    return mp_const_none;
}

// Pin(id, ...)
mp_obj_t mp_pin_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);

    mp_int_t pin_id = mp_obj_get_int(args[0]);
    if (pin_id < 0 || pin_id >= RA_PIN_NUM) {
		mp_raise_ValueError(MP_ERROR_TEXT("invalid pin number"));
	}

    machine_pin_obj_t *self = mp_obj_malloc(machine_pin_obj_t, &machine_pin_type);
    self->pin = ra_pin_table[pin_id];
    self->index = pin_id;
    self->mode = MP_HAL_PIN_MODE_INPUT;

    if (n_args > 1 || n_kw > 0) {
        mp_map_t kw_args;
        mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
        machine_pin_obj_init_helper(self, n_args - 1, args + 1, &kw_args);
    }
    return MP_OBJ_FROM_PTR(self);
}

// pin(value) 或 pin()
static mp_obj_t machine_pin_call(mp_obj_t self_in, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 1, false);
    machine_pin_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (n_args == 0) {
        // 读取引脚值
        return MP_OBJ_NEW_SMALL_INT(fsp_gpio_read(self));
    } else {
        // 设置引脚值
    	mp_int_t state = mp_obj_get_int(args[0]);
    	fsp_gpio_write(self, state);
        return mp_const_none;
    }
}


// init(...)
static mp_obj_t machine_pin_obj_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    return machine_pin_obj_init_helper(args[0], n_args - 1, args + 1, kw_args);
}
MP_DEFINE_CONST_FUN_OBJ_KW(machine_pin_init_obj, 1, machine_pin_obj_init);

// value([x])
static mp_obj_t machine_pin_value(size_t n_args, const mp_obj_t *args) {
    return machine_pin_call(args[0], n_args - 1, 0, args + 1);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pin_value_obj, 1, 2, machine_pin_value);

// low() / high() / on() / off()
static mp_obj_t machine_pin_low(mp_obj_t self_in) {
	fsp_gpio_write(MP_OBJ_TO_PTR(self_in), 0);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(machine_pin_low_obj, machine_pin_low);

static mp_obj_t machine_pin_high(mp_obj_t self_in) {
	fsp_gpio_write(MP_OBJ_TO_PTR(self_in), 1);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(machine_pin_high_obj, machine_pin_high);


// 打印函数
static void machine_pin_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    // 打印引脚名称、模式、上下拉等信息
	machine_pin_obj_t *self = MP_OBJ_TO_PTR(self_in);

	// 读取当前电平值
	int value = fsp_gpio_read(self);  // 假设返回值 0 或 1

	// 模式字符串
	const char *mode_str = "UNKNOWN";
	if (self->mode == MP_HAL_PIN_MODE_INPUT) {
		mode_str = "IN";
	} else if (self->mode == MP_HAL_PIN_MODE_OUTPUT) {
		mode_str = "OUT";
	}

	mp_printf(print, "Pin(index=%u, mode=%s, value=%d)", self->index, mode_str, value);
}

// 8. 本地字典（方法表和常量）
static const mp_rom_map_elem_t machine_pin_locals_dict_table[] = {
    // 方法
    { MP_ROM_QSTR(MP_QSTR_init),  MP_ROM_PTR(&machine_pin_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_value), MP_ROM_PTR(&machine_pin_value_obj) },
    { MP_ROM_QSTR(MP_QSTR_on),    MP_ROM_PTR(&machine_pin_high_obj) },
    { MP_ROM_QSTR(MP_QSTR_off),   MP_ROM_PTR(&machine_pin_low_obj) },
    { MP_ROM_QSTR(MP_QSTR_low),   MP_ROM_PTR(&machine_pin_low_obj) },
    { MP_ROM_QSTR(MP_QSTR_high),  MP_ROM_PTR(&machine_pin_high_obj) },

    // 类常量
    { MP_ROM_QSTR(MP_QSTR_IN),          MP_ROM_INT(MP_HAL_PIN_MODE_INPUT) },
    { MP_ROM_QSTR(MP_QSTR_OUT),         MP_ROM_INT(MP_HAL_PIN_MODE_OUTPUT) },
};
static MP_DEFINE_CONST_DICT(machine_pin_locals_dict, machine_pin_locals_dict_table);

// Pin 类型定义
MP_DEFINE_CONST_OBJ_TYPE(
    machine_pin_type,
    MP_QSTR_Pin,
    MP_TYPE_FLAG_NONE,
    make_new, mp_pin_make_new,
    print, machine_pin_print,
    call, machine_pin_call,
    locals_dict, &machine_pin_locals_dict
    // 还可以添加 protocol 等
);
