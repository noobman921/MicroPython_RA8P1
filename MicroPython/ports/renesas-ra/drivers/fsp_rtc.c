/*
 * fsp_rtc.c
 *
 *  Created on: 2026年6月17日
 *      Author: qq292
 */
#include "fsp_rtc.h"

#if RA_RTC

void fsp_rtc_settime(rtc_time_t* time){
	R_RTC_CalendarTimeSet(&g_rtc0_ctrl, time);
}

void fsp_rtc_gettime(rtc_time_t* time){
	R_RTC_CalendarTimeGet(&g_rtc0_ctrl, time);
}

#else

// 缺少 FSP RTC 依赖时的空实现
void fsp_rtc_settime(void *time){
}

void fsp_rtc_gettime(void *time){
}

#endif
