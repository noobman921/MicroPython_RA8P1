/*
 * fsp_rtc.h
 *
 *  Created on: 2026年6月17日
 *      Author: qq292
 */

#ifndef MICROPYTHON_PORTS_RENESAS_RA_DRIVERS_FSP_RTC_H_
#define MICROPYTHON_PORTS_RENESAS_RA_DRIVERS_FSP_RTC_H_

#include "mpy_board_cfg.h"

#if RA_RTC

void fsp_rtc_gettime(rtc_time_t* time);
void fsp_rtc_settime(rtc_time_t* time);

#else

// 缺少 FSP RTC 依赖时的空实现
void fsp_rtc_gettime(void *time);
void fsp_rtc_settime(void *time);

#endif

#endif /* MICROPYTHON_PORTS_RENESAS_RA_DRIVERS_FSP_RTC_H_ */
