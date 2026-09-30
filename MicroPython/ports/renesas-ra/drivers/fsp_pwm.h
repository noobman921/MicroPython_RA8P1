/*
 * fsp_pwm.h
 *
 *  Created on: 2026年7月2日
 *      Author: qq292
 */

#ifndef MICROPYTHON_PORTS_RENESAS_RA_DRIVERS_FSP_PWM_H_
#define MICROPYTHON_PORTS_RENESAS_RA_DRIVERS_FSP_PWM_H_

#include "mpy_board_cfg.h"

#if RA_PWM_NUM

int32_t fsp_pwm_config(timer_ctrl_t* const ctrl, timer_cfg_t* const cfg);
int32_t fsp_pwm_setFreq(timer_ctrl_t* const ctrl, uint32_t freq);
int32_t fsp_pwm_setDuty(timer_ctrl_t* const ctrl, uint32_t duty);
int32_t fsp_pwm_getPeriod(timer_ctrl_t* const ctrl, uint32_t* period);
int32_t fsp_pwm_close(timer_ctrl_t* const ctrl);

#else

// 缺少 FSP PWM(GPT) 依赖时的空实现
int32_t fsp_pwm_config(void *ctrl, void *cfg);
int32_t fsp_pwm_setFreq(void *ctrl, uint32_t freq);
int32_t fsp_pwm_setDuty(void *ctrl, uint32_t duty);
int32_t fsp_pwm_getPeriod(void *ctrl, uint32_t* period);
int32_t fsp_pwm_close(void *ctrl);

#endif

#endif /* MICROPYTHON_PORTS_RENESAS_RA_DRIVERS_FSP_PWM_H_ */
