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

#include "py/runtime.h"
#include "py/mphal.h"
#include "systick.h"
#include "hal_data.h"

volatile uint32_t uwTick;


void SysTick_Handler(void) {
    // Instead of calling HAL_IncTick we do the increment here of the counter.
    // This is purely for efficiency, since SysTick is called 1000 times per
    // second at the highest interrupt priority.
    uint32_t uw_tick = uwTick + 1;
    uwTick = uw_tick;

    // Read the systick control register. This has the side effect of clearing
    // the COUNTFLAG bit, which makes the logic in mp_hal_ticks_us
    // work properly.
    SysTick->CTRL;

}

// Core delay function that does an efficient sleep and may switch thread context.
void mp_hal_delay_ms(mp_uint_t ms) {
	// use systick counter to do the delay
	uint32_t start = uwTick;
	mp_uint_t elapsed = 0;
	// 让出线程，未实现
	do {
		mp_event_wait_ms(ms - elapsed);
		elapsed = uwTick - start;
	} while (elapsed < ms);
}


void mp_hal_delay_us(mp_uint_t us) {
	// 未实现
}

mp_uint_t mp_hal_ticks_ms(void) {
    return uwTick;
}

mp_uint_t mp_hal_ticks_us(void) {
	// 未实现
}

mp_uint_t mp_hal_ticks_cpu(void) {
	// 未实现
}
