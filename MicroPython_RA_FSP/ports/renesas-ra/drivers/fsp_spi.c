/*
 * fsp_spi.c
 *
 *  Created on: 2026年6月17日
 *      Author: qq292
 */
#include "fsp_spi.h"
#if RA_SPI_NUM
#define timeout 1000

static volatile bool spi_transfer_done = false;
/* Callback function */
void mpy_spi_callback(spi_callback_args_t *p_args)
{
	/* TODO: add your own code here */
	if(p_args->event == SPI_EVENT_TRANSFER_COMPLETE){
		spi_transfer_done = true;
	}
}

int32_t fsp_spi_config(spi_ctrl_t* const ctrl, spi_cfg_t* const cfg){
//	fsp_err_t err = R_SPI_B_Close(ctrl);
//	if(err != FSP_SUCCESS){
//		return -1;
//	}
	fsp_err_t err = R_SPI_B_Open(ctrl, cfg);
	if(err != FSP_SUCCESS){
		return -1;
	}
	return 0;
}

int32_t fsp_spi_close(spi_ctrl_t* const ctrl){
	fsp_err_t err = R_SPI_B_Close(ctrl);
	if(err != FSP_SUCCESS){
		return -1;
	}
	return 0;
}

int32_t fsp_spi_transfer(spi_ctrl_t* const ctrl, size_t len, const uint8_t *src, uint8_t *dest, uint8_t bitwidth){
	spi_transfer_done = false;
	fsp_err_t err = R_SPI_B_WriteRead(ctrl, src, dest, len, bitwidth);
	if(err != FSP_SUCCESS){
		return -1;
	}
	while(!spi_transfer_done){
		// 后续添加超时
	}
	return 0;
}
#endif
