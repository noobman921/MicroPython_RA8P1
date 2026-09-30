/*
 * board_cfg.c
 *
 *  Created on: 2026年6月16日
 *      Author: qq292
 */
#include "mpy_board_cfg.h"

// GPIO
bsp_io_port_pin_t ra_pin_table[RA_PIN_NUM] = {
		BSP_IO_PORT_01_PIN_08
};

// SPI
#if RA_SPI_NUM
spi_ctrl_t* ra_spi_table[RA_SPI_NUM] = {
		&g_spi0_ctrl,
};
spi_cfg_t ra_spi_cfg_table[RA_SPI_NUM] = {};
#endif

// UART
#if RA_UART_NUM
uart_ctrl_t* ra_uart_table[RA_UART_NUM] = {
		&g_uart0_ctrl,
};
uart_cfg_t ra_uart_cfg_table[RA_UART_NUM] = {};
#endif

// I2C
#if RA_I2C_MASTER_NUM
i2c_master_ctrl_t* ra_i2c_master_table[RA_I2C_MASTER_NUM] = {
		&g_i2c_master0_ctrl,
};
i2c_master_cfg_t ra_i2c_master_cfg_table[RA_I2C_MASTER_NUM] = {};
#endif

// PWM
#if RA_PWM_NUM
timer_ctrl_t* ra_pwm_table[RA_PWM_NUM] = {
		&g_timer0_ctrl,
};
extern timer_cfg_t ra_pwm_cfg_table[RA_PWM_NUM] = {};
#endif

void board_init(void){
	// GPIO
	R_IOPORT_Open(&RA_GPIO_CTRL, &RA_GPIO_CFG);
	// REPL
	R_SCI_B_UART_Open(&RA_REPL_CTRL, &RA_REPL_CFG);
	// RTC
#if RA_RTC
	R_RTC_Open(&RA_RTC_CTRL, &RA_RTC_CFG);
#endif
	// SPI
#if RA_SPI_NUM
	ra_spi_cfg_table[0] = g_spi0_cfg;
#endif
	// UART
#if RA_UART_NUM
	ra_uart_cfg_table[0] = g_uart0_cfg;
#endif
	// I2C
#if RA_I2C_MASTER_NUM
	ra_i2c_master_cfg_table[0] = g_i2c_master0_cfg;
#endif
	// PWM
#if RA_PWM_NUM
	ra_pwm_cfg_table[0] = g_timer0_cfg;
#endif
}
