/*
 * board_cfg.h
 *
 *  Created on: 2026年6月16日
 *      Author: qq292
 */

#ifndef MICROPYTHON_PORTS_RENESAS_RA_MPY_BOARD_CFG_H_
#define MICROPYTHON_PORTS_RENESAS_RA_BOARD_CFG_H_

#include "hal_data.h"
#include "mpconfigport.h"

// 文件系统设置
#define FLASH_SIZE			(8 * 1024 * 1024)
#define OSPI_B_SECTOR_SIZE_4K				(0x1000) //flash扇区

// 不可调整
#define RA_REPL_CTRL mpy_repl_uart_ctrl
#define RA_REPL_CFG  mpy_repl_uart_cfg

#define RA_GPIO_CTRL g_ioport_ctrl
#define RA_GPIO_CFG  g_bsp_pin_cfg

//RTC
#define RA_RTC 1
#if RA_RTC
#define RA_RTC_CTRL  g_rtc0_ctrl
#define RA_RTC_CFG   g_rtc0_cfg
#endif

#define RA_FS_FLASH 1
#if RA_FS_FLASH
#define RA_OSPI_FLASH_CTRL	g_ospi0_ctrl
#define RA_OSPI_FLASH_CFG	g_ospi0_cfg
#endif


// 可调整
#define RA_PIN_NUM 5
extern bsp_io_port_pin_t ra_pin_table[RA_PIN_NUM];

#define RA_SPI_NUM 1
#if RA_SPI_NUM
extern spi_ctrl_t* ra_spi_table[RA_SPI_NUM];
extern spi_cfg_t ra_spi_cfg_table[RA_SPI_NUM];
#endif

#define RA_UART_NUM 1
#if RA_UART_NUM
extern uart_ctrl_t* ra_uart_table[RA_UART_NUM];
extern uart_cfg_t ra_uart_cfg_table[RA_UART_NUM];
#endif

#define RA_I2C_MASTER_NUM 1
#if RA_I2C_MASTER_NUM
extern i2c_master_ctrl_t* ra_i2c_master_table[RA_I2C_MASTER_NUM];
extern i2c_master_cfg_t ra_i2c_master_cfg_table[RA_I2C_MASTER_NUM];
#endif

#define RA_PWM_NUM 1
#if RA_PWM_NUM
extern timer_ctrl_t* ra_pwm_table[RA_PWM_NUM];
extern timer_cfg_t ra_pwm_cfg_table[RA_PWM_NUM];
#endif



void board_init(void);

#endif /* MICROPYTHON_PORTS_RENESAS_RA_MPY_BOARD_CFG_H_ */
