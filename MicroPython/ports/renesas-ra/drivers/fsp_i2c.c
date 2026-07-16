/*
 * fsp_i2c.c
 *
 *  Created on: 2026年7月1日
 *      Author: qq292
 */
#include "fsp_i2c.h"

// 空实现
void g_iic_b_master0_callback(i2c_master_callback_args_t *p_args){

}

int32_t fsp_i2c_config(i2c_master_ctrl_t* const ctrl, i2c_master_cfg_t* const cfg){
	fsp_err_t err = R_IIC_B_MASTER_Open(ctrl, cfg);
	if(err != FSP_SUCCESS){
		return -1;
	}
	return 0;
}

int32_t fsp_i2c_setAddress(i2c_master_ctrl_t* const ctrl, uint32_t slave_address,  i2c_master_addr_mode_t mode){
	// 丢弃当前传输，设置新的从机地址
	fsp_err_t err = R_IIC_B_MASTER_Abort(ctrl);
	if(err != FSP_SUCCESS){
		return -1;
	}
	err = R_IIC_B_MASTER_SlaveAddressSet(ctrl, slave_address, mode);
	if(err != FSP_SUCCESS){
		return -1;
	}
	return 0;
}

int32_t fsp_i2c_write(i2c_master_ctrl_t* const ctrl, uint8_t* buf, size_t len, bool restart){
	fsp_err_t err = R_IIC_B_MASTER_Write(ctrl, buf, len, restart);
	if(err != FSP_SUCCESS){
		return -1;
	}
	// 阻塞
	iic_b_master_instance_ctrl_t* p_ctrl = (iic_b_master_instance_ctrl_t*)ctrl;
	R_I3C0_Type *sci = (R_I3C0_Type *)p_ctrl->p_reg;
	while ((sci->BST_b.TENDF == 0)) {
		// 等待硬件真正发送完成
		// 后续考虑超时
	}
	return 0;
}

int32_t fsp_i2c_read(i2c_master_ctrl_t* const ctrl, uint8_t* buf, size_t len, bool restart){
	fsp_err_t err = R_IIC_B_MASTER_Read(ctrl, buf, len, restart);
	if(err != FSP_SUCCESS){
		return -1;
	}
	// 阻塞
	iic_b_master_instance_ctrl_t* p_ctrl = (iic_b_master_instance_ctrl_t*)ctrl;
	R_I3C0_Type *sci = (R_I3C0_Type *)p_ctrl->p_reg;
	while ((sci->BST_b.TENDF == 0)) {
		// 等待硬件真正发送完成
		// 后续考虑超时
	}
	return 0;
}

int32_t fsp_i2c_close(i2c_master_ctrl_t* const ctrl){
	fsp_err_t err = R_IIC_B_MASTER_Close(ctrl);
	if(err != FSP_SUCCESS){
		return -1;
	}
	return 0;
}
