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
spi_ctrl_t* ra_spi_table[RA_SPI_NUM] = {
		&g_spi0_ctrl,
};
spi_cfg_t ra_spi_cfg_table[RA_SPI_NUM] = {};

// UART
uart_ctrl_t* ra_uart_table[RA_UART_NUM] = {
		&g_uart0_ctrl,
};
uart_cfg_t ra_uart_cfg_table[RA_UART_NUM] = {};

// I2C
i2c_master_ctrl_t* ra_i2c_master_table[RA_I2C_MASTER_NUM] = {
		&g_i2c_master0_ctrl,
};
i2c_master_cfg_t ra_i2c_master_cfg_table[RA_I2C_MASTER_NUM] = {};

// PWM
timer_ctrl_t* ra_pwm_table[RA_PWM_NUM] = {
		&g_timer0_ctrl,
};
extern timer_cfg_t ra_pwm_cfg_table[RA_PWM_NUM] = {};

void board_init(void){
	// GPIO
	R_IOPORT_Open(&RA_GPIO_CTRL, &RA_GPIO_CFG);
	// REPL
	R_SCI_B_UART_Open(&RA_REPL_CTRL, &RA_REPL_CFG);
	// RTC
	R_RTC_Open(&RA_RTC_CTRL, &RA_RTC_CFG);
	// SPI
	ra_spi_cfg_table[0] = g_spi0_cfg;
	// UART
	ra_uart_cfg_table[0] = g_uart0_cfg;
	// I2C
	ra_i2c_master_cfg_table[0] = g_i2c_master0_cfg;
	// PWM
	ra_pwm_cfg_table[0] = g_timer0_cfg;
	// OSPI
	R_OSPI_B_Open(&RA_OSPI_FLASH_CTRL, &RA_OSPI_FLASH_CFG);

}
