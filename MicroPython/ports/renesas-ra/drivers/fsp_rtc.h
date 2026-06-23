/*
 * fsp_rtc.h
 *
 *  Created on: 2026年6月17日
 *      Author: qq292
 */

#ifndef MICROPYTHON_PORTS_RENESAS_RA_DRIVERS_FSP_RTC_H_
#define MICROPYTHON_PORTS_RENESAS_RA_DRIVERS_FSP_RTC_H_

#include "mpy_board_cfg.h"

void fsp_rtc_gettime(rtc_time_t* time);
void fsp_rtc_settime(rtc_time_t* time);

#endif /* MICROPYTHON_PORTS_RENESAS_RA_DRIVERS_FSP_RTC_H_ */
