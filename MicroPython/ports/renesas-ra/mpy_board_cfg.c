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

#if RA_SPI_NUM
// SPI
spi_ctrl_t* ra_spi_table[RA_SPI_NUM] = {
		&g_spi0_ctrl,
};
spi_cfg_t ra_spi_cfg_table[RA_SPI_NUM] = {};
#endif

#if RA_UART_NUM
// UART
uart_ctrl_t* ra_uart_table[RA_UART_NUM] = {
		&g_uart0_ctrl,
};
uart_cfg_t ra_uart_cfg_table[RA_UART_NUM] = {};
#endif

void board_init(void){
	// GPIO
	R_IOPORT_Open(&RA_GPIO_CTRL, &RA_GPIO_CFG);
	// REPL
	R_SCI_B_UART_Open(&RA_REPL_CTRL, &RA_REPL_CFG);
	// RTC
	R_RTC_Open(&RA_RTC_CTRL, &RA_RTC_CFG);
#if RA_SPI_NUM
	// SPI
	ra_spi_cfg_table[0] = g_spi0_cfg;
#endif
#if RA_UART_NUM
	// UART
	ra_uart_cfg_table[0] = g_uart0_cfg;
#endif
	// OSPI
#if RA_OSPI_FLASH_CTRL
	R_OSPI_B_Open(&RA_OSPI_FLASH_CTRL, &RA_OSPI_FLASH_CFG);
#endif
}
