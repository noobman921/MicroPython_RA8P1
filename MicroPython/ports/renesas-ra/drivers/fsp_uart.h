/*
 * fsp_uart.h
 *
 *  Created on: 2026年6月15日
 *      Author: qq292
 */

#ifndef MICROPYTHON_PORTS_RENESAS_RA_DRIVERS_FSP_UART_H_
#define MICROPYTHON_PORTS_RENESAS_RA_DRIVERS_FSP_UART_H_

#include "mpy_board_cfg.h"

int32_t fsp_uart_config(uart_ctrl_t* const ctrl, uart_cfg_t* const cfg);
int32_t fsp_uart_set_baudrate(uart_ctrl_t* const ctrl, uint32_t baudrate);
int32_t fsp_uart_close(uart_ctrl_t* const ctrl);
void fsp_uart_buf_init(uint8_t id);
uint32_t fsp_uart_rx_avail(uint8_t id);
bool fsp_uart_tx_complete(uart_ctrl_t* const ctrl);
uint32_t fsp_uart_read(uint8_t id, uint8_t *buffer, uint32_t len);
uint32_t fsp_uart_write(uart_ctrl_t* const ctrl, uint8_t *buffer, uint32_t len);

#endif /* MICROPYTHON_PORTS_RENESAS_RA_DRIVERS_FSP_UART_H_ */
