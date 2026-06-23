/*
 * fsp_spi.h
 *
 *  Created on: 2026年6月17日
 *      Author: qq292
 */

#ifndef MICROPYTHON_PORTS_RENESAS_RA_DRIVERS_FSP_SPI_H_
#define MICROPYTHON_PORTS_RENESAS_RA_DRIVERS_FSP_SPI_H_

#if RA_SPI_NUM
#include "mpy_board_cfg.h"

int32_t fsp_spi_config(spi_ctrl_t* const ctrl, spi_cfg_t* const cfg);
int32_t fsp_spi_transfer(spi_ctrl_t* const ctrl, size_t len, const uint8_t *src, uint8_t *dest, uint8_t bitwidth);
int32_t fsp_spi_close(spi_ctrl_t* const ctrl);
#endif

#endif /* MICROPYTHON_PORTS_RENESAS_RA_DRIVERS_FSP_SPI_H_ */
