/*
 * fsp_i2c.h
 *
 *  Created on: 2026年7月1日
 *      Author: qq292
 */

#ifndef MICROPYTHON_PORTS_RENESAS_RA_DRIVERS_FSP_I2C_H_
#define MICROPYTHON_PORTS_RENESAS_RA_DRIVERS_FSP_I2C_H_

#include "mpy_board_cfg.h"

#if RA_I2C_MASTER_NUM

int32_t fsp_i2c_config(i2c_master_ctrl_t* const ctrl, i2c_master_cfg_t* const cfg);
int32_t fsp_i2c_setAddress(i2c_master_ctrl_t* const ctrl, uint32_t slave_address,  i2c_master_addr_mode_t mode);
int32_t fsp_i2c_write(i2c_master_ctrl_t* const ctrl, uint8_t* buf, size_t len, bool restart);
int32_t fsp_i2c_read(i2c_master_ctrl_t* const ctrl, uint8_t* buf, size_t len, bool restart);
int32_t fsp_i2c_close(i2c_master_ctrl_t* const ctrl);

#else

// 缺少 FSP I2C 依赖时的空实现
int32_t fsp_i2c_config(void *ctrl, void *cfg);
int32_t fsp_i2c_setAddress(void *ctrl, uint32_t slave_address, uint32_t mode);
int32_t fsp_i2c_write(void *ctrl, uint8_t* buf, size_t len, bool restart);
int32_t fsp_i2c_read(void *ctrl, uint8_t* buf, size_t len, bool restart);
int32_t fsp_i2c_close(void *ctrl);

#endif

#endif /* MICROPYTHON_PORTS_RENESAS_RA_DRIVERS_FSP_I2C_H_ */
