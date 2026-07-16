#ifndef MICROPYTHON_PORTS_RENESAS_RA_MOCONFIGBOARD_H_
#define MICROPYTHON_PORTS_RENESAS_RA_MOCONFIGBOARD_H_
#include "hal_data.h"

#define MICROPY_HW_BOARD_NAME "Titan-Mini"
#define MICROPY_HW_MCU_NAME   "RA8P1"

#include "mpy_board_cfg.h"
// 通用核心设置

// repl的串口句柄
#define MICROPY_HW_ENABLE_UART_REPL (1)


// Heap start / end definitions
#ifndef MICROPY_HEAP_START
#define MICROPY_HEAP_START &_heap_start
#endif
#ifndef MICROPY_HEAP_END
#define MICROPY_HEAP_END &_heap_end
#endif


// machine设置

#define MICROPY_PY_MACHINE          (1)
#define MICROPY_PY_MACHINE_INCLUDEFILE "ports/renesas-ra/modmachine.c"
#if RA_SPI_NUM
#define MICROPY_PY_MACHINE_SPI      (1)	// SPI
#define MICROPY_PY_MACHINE_SOFTSPI	(0) // SoftSPI
#endif
//#define MP_HAL_PIN_FMT              "%q"
#if RA_UART_NUM // UART
#define MICROPY_PY_MACHINE_UART		(1) // UART
#define MICROPY_PY_MACHINE_UART_INCLUDEFILE "ports/renesas-ra/machine_uart.c"
#define MICROPY_PY_MACHINE_UART_CLASS_CONSTANTS
#endif
#if RA_I2C_MASTER_NUM // I2C
#define MICROPY_PY_MACHINE_I2C		(1)
#define MICROPY_PY_MACHINE_SOFTI2C	(0)
#endif
#if RA_PWM_NUM  // PWM
#define MICROPY_PY_MACHINE_PWM      (1)
#define MICROPY_PY_MACHINE_PWM_INCLUDEFILE "ports/renesas-ra/machine_pwm.c"
#define MICROPY_ENABLE_FINALISER (1)
#endif

#define MICROPY_HW_ENABLE_INTERNAL_FLASH_STORAGE (0)

// OS模块

// 文件系统
#define MICROPY_VFS			(1)		// 启用VFS
#define MICROPY_VFS_FAT		(1)		// 选择FAT
#define MICROPY_FATFS_RPATH (2)
#define FFCONF_H			"lib/oofatfs/ffconf.h"
#define MICROPY_FATFS_MAX_SS	(4096)
// 外部Flash 未修改
// External SPI Flash config
//#if !MICROPY_HW_ENABLE_INTERNAL_FLASH_STORAGE
//
//#define MICROPY_HW_SPI_IS_RESERVED(id)  (id == 1) // Reserve SPI flash bus.
//#define MICROPY_HW_SPIFLASH_SIZE_BITS (16 * 1024 * 1024) // 16 Mbit (2 MByte)
//
//#define MICROPY_HW_SPIFLASH_CS      (MICROPY_HW_SPI1_NSS)
//#define MICROPY_HW_SPIFLASH_SCK     (MICROPY_HW_SPI1_SCK)
//#define MICROPY_HW_SPIFLASH_MISO    (MICROPY_HW_SPI1_MISO)
//#define MICROPY_HW_SPIFLASH_MOSI    (MICROPY_HW_SPI1_MOSI)
//
//extern const struct _mp_spiflash_config_t spiflash_config;
//extern struct _spi_bdev_t spi_bdev;
//#define MICROPY_HW_SPIFLASH_ENABLE_CACHE (0)
//#define MICROPY_HW_BDEV_SPIFLASH    (&spi_bdev)
//#define MICROPY_HW_BDEV_SPIFLASH_CONFIG (&spiflash_config)
//#define MICROPY_HW_BDEV_SPIFLASH_SIZE_BYTES (MICROPY_HW_SPIFLASH_SIZE_BITS / 8)
//#define MICROPY_HW_BDEV_SPIFLASH_EXTENDED (&spi_bdev) // for extended block protocol

#endif
