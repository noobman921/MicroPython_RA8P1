/*
 * fsp_i2c.h
 *
 *  Created on: 2026年7月1日
 *      Author: qq292
 */

#ifndef MICROPYTHON_PORTS_RENESAS_RA_DRIVERS_FSP_I2C_H_
#define MICROPYTHON_PORTS_RENESAS_RA_DRIVERS_FSP_I2C_H_

#include "mpy_board_cfg.h"

int32_t fsp_i2c_config(i2c_master_ctrl_t* const ctrl, i2c_master_cfg_t* const cfg);
int32_t fsp_i2c_setAddress(i2c_master_ctrl_t* const ctrl, uint32_t slave_address,  i2c_master_addr_mode_t mode);
int32_t fsp_i2c_write(i2c_master_ctrl_t* const ctrl, uint8_t* buf, size_t len, bool restart);
int32_t fsp_i2c_read(i2c_master_ctrl_t* const ctrl, uint8_t* buf, size_t len, bool restart);
int32_t fsp_i2c_close(i2c_master_ctrl_t* const ctrl);

#endif /* MICROPYTHON_PORTS_RENESAS_RA_DRIVERS_FSP_I2C_H_ */
