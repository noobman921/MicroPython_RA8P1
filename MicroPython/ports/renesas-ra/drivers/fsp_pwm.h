/*
 * fsp_pwm.h
 *
 *  Created on: 2026年7月2日
 *      Author: qq292
 */

#ifndef MICROPYTHON_PORTS_RENESAS_RA_DRIVERS_FSP_PWM_H_
#define MICROPYTHON_PORTS_RENESAS_RA_DRIVERS_FSP_PWM_H_

#include "mpy_board_cfg.h"

int32_t fsp_pwm_config(timer_ctrl_t* const ctrl, timer_cfg_t* const cfg);
int32_t fsp_pwm_setFreq(timer_ctrl_t* const ctrl, uint32_t freq);
int32_t fsp_pwm_setDuty(timer_ctrl_t* const ctrl, uint32_t duty);
int32_t fsp_pwm_getPeriod(timer_ctrl_t* const ctrl, uint32_t* period);
int32_t fsp_pwm_close(timer_ctrl_t* const ctrl);

#endif /* MICROPYTHON_PORTS_RENESAS_RA_DRIVERS_FSP_PWM_H_ */
