/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
 * Copyright (c) 2021 Renesas Electronics Corporation
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

#include "py/runtime.h"
#include "extmod/modmachine.h"
#include "shared/timeutils/timeutils.h"

#include "drivers/fsp_rtc.h"

#define RTC_INIT_YEAR   2015
#define RTC_INIT_MONTH  1
#define RTC_INIT_DATE   1
#define RTC_INIT_HOUR   0
#define RTC_INIT_MINUTE 0
#define RTC_INIT_SECOND 0

/******************************************************************************/
// MicroPython bindings

typedef struct _machine_rtc_obj_t {
    mp_obj_base_t base;
} machine_rtc_obj_t;

static const machine_rtc_obj_t machine_rtc_obj = {{&machine_rtc_type}};

static bool rtc_initialized = false;


/// \classmethod \constructor()
/// Create an RTC object.
static mp_obj_t machine_rtc_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 0, 0, false);
    if (!rtc_initialized) {
		// 只在第一次创建时设置默认时间
		rtc_time_t tm = {
			.tm_year = RTC_INIT_YEAR - 1900,
			.tm_mon = RTC_INIT_MONTH,
			.tm_mday = RTC_INIT_DATE,
			.tm_hour = RTC_INIT_HOUR,
			.tm_min = RTC_INIT_MINUTE,
			.tm_sec = RTC_INIT_SECOND,
		};
		fsp_rtc_settime(&tm);
		rtc_initialized = true;
	}
    // return constant object
    return MP_OBJ_FROM_PTR(&machine_rtc_obj);
}

// force rtc to re-initialise
static mp_obj_t machine_rtc_init(mp_obj_t self_in, mp_obj_t datetime_tuple) {
	mp_obj_t *items;
	mp_obj_get_array_fixed_n(datetime_tuple, 8, &items);
	rtc_time_t tm = {
		.tm_year = mp_obj_get_int(items[0]) - 1900,
		.tm_mon = mp_obj_get_int(items[1]),
		.tm_mday = mp_obj_get_int(items[2]),
		.tm_hour = mp_obj_get_int(items[3]),
		.tm_min = mp_obj_get_int(items[4]),
		.tm_sec = mp_obj_get_int(items[5]),
	};
	fsp_rtc_settime(&tm);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(machine_rtc_init_obj, machine_rtc_init);

// force rtc to re-initialise
static mp_obj_t machine_rtc_deinit(mp_obj_t self_in) {
	rtc_time_t tm = {
		.tm_year = RTC_INIT_YEAR - 1900,
		.tm_mon = RTC_INIT_MONTH,
		.tm_mday = RTC_INIT_DATE,
		.tm_hour = RTC_INIT_HOUR,
		.tm_min = RTC_INIT_MINUTE,
		.tm_sec = RTC_INIT_SECOND,
	};
	fsp_rtc_settime(&tm);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(machine_rtc_deinit_obj, machine_rtc_deinit);

/// \method datetime([datetimetuple])
/// Get or set the date and time of the RTC.
///
/// With no arguments, this method returns an 8-tuple with the current
/// date and time.  With 1 argument (being an 8-tuple) it sets the date
/// and time.
///
/// The 8-tuple has the following format:
///
///     (year, month, day, hours, minutes, seconds, subseconds, tzinfo)
///
/// `weekday` is 1-7 for Monday through Sunday.
///
/// `subseconds` counts down from 255 to 0
mp_obj_t machine_rtc_datetime(size_t n_args, const mp_obj_t *args) {
    if (n_args == 1) {
    	rtc_time_t tm;
        fsp_rtc_gettime(&tm);
        mp_obj_t tuple[8] = {
            mp_obj_new_int(tm.tm_year + 1900),
            mp_obj_new_int(tm.tm_mon),
            mp_obj_new_int(tm.tm_mday),
            mp_obj_new_int(tm.tm_hour),
            mp_obj_new_int(tm.tm_min),
            mp_obj_new_int(tm.tm_sec),
            mp_obj_new_int(0),
			mp_obj_new_int(0),
        };
        return mp_obj_new_tuple(8, tuple);
    } else {
        // set date and time
        mp_obj_t *items;
        mp_obj_get_array_fixed_n(args[1], 8, &items);
        rtc_time_t tm;
        tm.tm_year = mp_obj_get_int(items[0]) - 1900;
        tm.tm_mon = mp_obj_get_int(items[1]);
        tm.tm_mday = mp_obj_get_int(items[2]);
        tm.tm_hour = mp_obj_get_int(items[3]);
        tm.tm_min = mp_obj_get_int(items[4]);
        tm.tm_sec = mp_obj_get_int(items[5]);
        fsp_rtc_settime(&tm);
        return mp_const_none;
    }
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_rtc_datetime_obj, 1, 2, machine_rtc_datetime);


static const mp_rom_map_elem_t machine_rtc_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_rtc_init_obj) },
	{ MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&machine_rtc_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_datetime), MP_ROM_PTR(&machine_rtc_datetime_obj) },
};
static MP_DEFINE_CONST_DICT(machine_rtc_locals_dict, machine_rtc_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    machine_rtc_type,
    MP_QSTR_RTC,
    MP_TYPE_FLAG_NONE,
    make_new, machine_rtc_make_new,
    locals_dict, &machine_rtc_locals_dict
    );
