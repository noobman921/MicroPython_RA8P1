/*
 * fsp_qspi_flash.h
 *
 *  Created on: 2026年6月18日
 *      Author: qq292
 */

#ifndef MICROPYTHON_PORTS_RENESAS_RA_DRIVERS_FSP_QSPI_FLASH_H_
#define MICROPYTHON_PORTS_RENESAS_RA_DRIVERS_FSP_QSPI_FLASH_H_

#include "mpy_board_cfg.h"


// 读写配置
#define TIME_OUT_VAL                    (0x00)

/* Macro for OSPI command code */
#define OSPI_B_COMMAND_WRITE_ENABLE_SPI             (0x06)

#define OSPI_B_COMMAND_READ_STATUS_SPI              (0x05)

#define OSPI_B_COMMAND_READ_DEVICE_ID_SPI           (0x90)
#define OSPI_B_COMMAND_READ_DEVICE_ID_QPI           (0x94)

#define OSPI_B_COMMAND_WRITE_SR2_SPI                (0x31)
#define OSPI_B_COMMAND_READ_SR2_SPI                 (0x35)

/* Macro for OSPI command length */
#define OSPI_B_COMMAND_LENGTH_SPI                   (1U)
#define OSPI_B_COMMAND_LENGTH_OPI                   (2U)

/* Macro for OSPI transfer address */
#define OSPI_B_ADDRESS_DUMMY                        (0U)
#define OSPI_B_ADDRESS_LENGTH_ZERO                  (0U)
#define OSPI_B_ADDRESS_LENGTH_THREE                 (3U)
#define OSPI_B_ADDRESS_LENGTH_FOUR                  (4U)

/* Macro for OSPI transfer data */
#define OSPI_B_DATA_DUMMY                           (0U)
#define OSPI_B_DATA_LENGTH_ZERO                     (0U)
#define OSPI_B_DATA_LENGTH_ONE                      (1U)
#define OSPI_B_DATA_LENGTH_TWO                      (2U)
#define OSPI_B_DATA_LENGTH_THREE                    (3U)
#define OSPI_B_DATA_LENGTH_FOUR                     (4U)

/* Macro for OSPI transfer dummy cycles */
#define OSPI_B_DUMMY_CYCLE_WRITE_SPI                (0U)
#define OSPI_B_DUMMY_CYCLE_WRITE_OPI                (0U)
#define OSPI_B_DUMMY_CYCLE_READ_STATUS_SPI          (0U)
#define OSPI_B_DUMMY_CYCLE_READ_STATUS_OPI          (4U)
#define OSPI_B_DUMMY_CYCLE_READ_REGISTER_SPI        (1U)
#define OSPI_B_DUMMY_CYCLE_READ_REGISTER_OPI        (4U)
#define OSPI_B_DUMMY_CYCLE_READ_MEMORY_SPI          (3U)
#define OSPI_B_DUMMY_CYCLE_READ_MEMORY_OPI          (10U)

/* Macro for flash device register address */
#define OSPI_B_ADDRESS_STR1V_REGISTER               (0x00800000)
#define OSPI_B_ADDRESS_STR2V_REGISTER               (0x00800001)
#define OSPI_B_ADDRESS_CFR1V_REGISTER               (0x00800002)
#define OSPI_B_ADDRESS_CFR2V_REGISTER               (0x00800003)
#define OSPI_B_ADDRESS_CFR3V_REGISTER               (0x00800004)
#define OSPI_B_ADDRESS_CFR4V_REGISTER               (0x00800005)
#define OSPI_B_ADDRESS_CFR5V_REGISTER               (0x00800006)

/* Macros for configure flash device */
#define OSPI_B_DATA_CFR2V_REGISTER                  (0x83)
#define OSPI_B_DATA_CFR3V_REGISTER                  (0x40)
#define OSPI_B_DATA_SET_SPI_CFR5V_REGISTER          (0x40)
#define OSPI_B_DATA_SET_OPI_CFR5V_REGISTER          (0x43)

typedef enum e_ospi_b_transfer
{
    OSPI_B_TRANSFER_WRITE_ENABLE_SPI = 0,
    OSPI_B_TRANSFER_READ_STATUS_SPI,
    OSPI_B_TRANSFER_READ_DEVICE_ID_SPI,
    OSPI_B_TRANSFER_WRITE_SR2_SPI,
    OSPI_B_TRANSFER_READ_SR2_SPI,
    OSPI_B_TRANSFER_READ_DEVICE_ID_OPI,
    OSPI_B_TRANSFER_MAX
} ospi_b_transfer_t;

/* Flash device status bit */
#define OSPI_B_WEN_BIT_MASK                 (0x00000002)
#define OSPI_B_BUSY_BIT_MASK                (0x00000001)

fsp_err_t QSPI_Flash_Init(void);
fsp_err_t QSPI_Flash_WriteEnable(void);
fsp_err_t QSPI_Flash_WaitForWriteEnd(void);
int32_t QSPI_Flash_Erase(uint32_t offset, uint32_t size);
int32_t QSPI_Flash_Read(long offset, uint8_t *buf, uint32_t size);
int32_t QSPI_Flash_Write(long offset, const uint8_t *buf, uint32_t size);

struct _fs_user_mount_t;
void qspi_flash_init_vfs(struct _fs_user_mount_t *vfs);

#endif /* MICROPYTHON_PORTS_RENESAS_RA_DRIVERS_FSP_QSPI_FLASH_H_ */
