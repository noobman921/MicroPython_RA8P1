/*
 * fsp_rtc.c
 *
 *  Created on: 2026年6月17日
 *      Author: qq292
 */
#include "fsp_rtc.h"

void fsp_rtc_settime(rtc_time_t* time){
	R_RTC_CalendarTimeSet(&g_rtc0_ctrl, time);
}

void fsp_rtc_gettime(rtc_time_t* time){
	R_RTC_CalendarTimeGet(&g_rtc0_ctrl, time);
}
