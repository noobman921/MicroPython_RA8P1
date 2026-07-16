/*
 * fsp_pwm.c
 *
 *  Created on: 2026年7月2日
 *      Author: qq292
 */
#include "fsp_pwm.h"

int32_t fsp_pwm_config(timer_ctrl_t* const ctrl, timer_cfg_t* const cfg){
    fsp_err_t err = R_GPT_Open(ctrl, cfg);
	if(err != FSP_SUCCESS){
		return -1;
	}
    err = R_GPT_Start(ctrl);
    if(err != FSP_SUCCESS){
        return -1;
    }
	return 0;
}

int32_t fsp_pwm_setFreq(timer_ctrl_t* const ctrl, uint32_t freq){
    //停止计数器
    fsp_err_t err = R_GPT_Stop(ctrl);
    if(err != FSP_SUCCESS){
        return -1;
    }
    // 计算周期
    uint32_t pclkd_freq_hz = R_FSP_SystemClockHzGet(FSP_PRIV_CLOCK_PCLKD) >> g_timer0_cfg.source_div; // 获取PCLKD频率并除以分频系数
    uint32_t period_counts = (pclkd_freq_hz / freq) - 1; // 计算周期计数值
    err = R_GPT_PeriodSet(ctrl, period_counts);
    if(err != FSP_SUCCESS){
        return -1;
    }
    err = R_GPT_Start(ctrl);
    if(err != FSP_SUCCESS){
        return -1;
    }
    return 0;
}

int32_t fsp_pwm_setDuty(timer_ctrl_t* const ctrl, uint32_t duty){
    timer_info_t info;
    fsp_err_t err = R_GPT_InfoGet(ctrl, &info);
    if(err != FSP_SUCCESS){
        return -1;
    }
    uint32_t period = info.period_counts;
    uint32_t duty_counts = (duty * period) / 100;
    // 对 A和B 引脚同时设置占空比，后期酌情修改
    err = R_GPT_DutyCycleSet(ctrl, duty_counts, GPT_IO_PIN_GTIOCA_AND_GTIOCB);
    if(err != FSP_SUCCESS){
        return -1;
    }
    return 0;
}

int32_t fsp_pwm_getPeriod(timer_ctrl_t* const ctrl, uint32_t* period){
    timer_info_t info;
    fsp_err_t err = R_GPT_InfoGet(ctrl, &info);
    if(err != FSP_SUCCESS){
        return -1;
    }
    *period = info.period_counts;
    return 0;
}


int32_t fsp_pwm_close(timer_ctrl_t* const ctrl){
    fsp_err_t err = R_GPT_Close(ctrl);
    if(err != FSP_SUCCESS){
        return -1;
    }
    return 0;
}