/*
 * fsp_gpio.c
 *
 *  Created on: 2026年6月16日
 *      Author: qq292
 */
#include "fsp_gpio.h"

void fsp_gpio_config(const machine_pin_obj_t *obj, uint32_t cfg){
	R_IOPORT_PinCfg(&g_ioport_ctrl, obj->pin, cfg);
}

void fsp_gpio_write(const machine_pin_obj_t *obj, uint32_t state){
	R_IOPORT_PinWrite(&g_ioport_ctrl, obj->pin, state?BSP_IO_LEVEL_HIGH:BSP_IO_LEVEL_LOW);
}

void fsp_gpio_toggle(const machine_pin_obj_t *obj){
	bsp_io_level_t val;
	R_IOPORT_PinRead(&g_ioport_ctrl, obj->pin, &val);
	val = !val;
	R_IOPORT_PinWrite(&g_ioport_ctrl, obj->pin, val);
}

uint32_t fsp_gpio_read(const machine_pin_obj_t *obj){
	bsp_io_level_t val;
	R_IOPORT_PinRead(&g_ioport_ctrl, obj->pin, &val);
	return val;
}
