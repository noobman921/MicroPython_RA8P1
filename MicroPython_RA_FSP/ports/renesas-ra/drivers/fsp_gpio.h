/*
 * fsp_gpio.h
 *
 *  Created on: 2026年6月16日
 *      Author: qq292
 */

#ifndef MICROPYTHON_PORTS_RENESAS_RA_DRIVERS_FSP_GPIO_H_
#define MICROPYTHON_PORTS_RENESAS_RA_DRIVERS_FSP_GPIO_H_

#include "modmachine.h"
#include "mpy_board_cfg.h"

void fsp_gpio_config(const machine_pin_obj_t *obj_t, uint32_t cfg);
void fsp_gpio_write(const machine_pin_obj_t *obj, uint32_t state);
void fsp_gpio_toggle(const machine_pin_obj_t *obj);
uint32_t fsp_gpio_read(const machine_pin_obj_t *obj);


#endif /* MICROPYTHON_PORTS_RENESAS_RA_DRIVERS_FSP_GPIO_H_ */
