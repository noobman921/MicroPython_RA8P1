#include <stdint.h>
#include "mpy_board_cfg.h"
#include <stdio.h>
#include <string.h>

#include "py/builtin.h"
#include "py/compile.h"
#include "py/runtime.h"
#include "py/repl.h"
#include "py/gc.h"
#include "py/mperrno.h"
#include "shared/runtime/pyexec.h"
#include "gccollect.h"
#include "extmod/vfs_fat.h"

#include "drivers/fsp_qspi_flash.h"

static fs_user_mount_t qspi_flash_fs;
static const mp_obj_t mount_point = MP_OBJ_NEW_QSTR(MP_QSTR__slash_flash);

/* QSPI Flash 硬件验证测试 */
static int qspi_flash_test(void) {
    fsp_err_t err;
    uint32_t test_offset = 0x100000; /* 1MB偏移，避开可能的启动区域 */
    uint8_t wbuf[256];
    uint8_t rbuf[256];

    printf("\r\n========== QSPI Flash Test ==========\r\n");

    /* Step 1: 内存映射读（确认OSPI总线是否激活） */
    printf("[1/6] Memory-mapped read @ 0x90000000... ");
    printf("0x%08lX\r\n", (unsigned long)(*(volatile uint32_t *)0x90000000));

    /* Step 2: 读取Device ID */
    {
        spi_flash_direct_transfer_t transfer;
        extern spi_flash_direct_transfer_t g_ospi_b_direct_transfer[];
        printf("[2/6] Reading Device ID... ");
        memset(&transfer, 0, sizeof(transfer));
        transfer = g_ospi_b_direct_transfer[OSPI_B_TRANSFER_READ_DEVICE_ID_SPI];
        err = R_OSPI_B_DirectTransfer(&RA_OSPI_FLASH_CTRL, &transfer,
                                      SPI_FLASH_DIRECT_TRANSFER_DIR_READ);
        if (err != FSP_SUCCESS) {
            printf("FAIL (DirectTransfer err=%d)\r\n", err);
            return -1;
        }
        printf("0x%06lX (expect 0xEF4017 for W25Q64)\r\n",
               (unsigned long)(transfer.data & 0xFFFFFF));
    }

    /* Step 3: 擦除一个扇区 */
    printf("[3/6] Erasing sector @ 0x%lX... ", (unsigned long)test_offset);
    err = R_OSPI_B_Erase(&RA_OSPI_FLASH_CTRL,
                          (uint8_t *)(0x90000000 + test_offset), 4096);
    if (err != FSP_SUCCESS) {
        printf("FAIL (R_OSPI_B_Erase err=%d)\r\n", err);
        return -1;
    }
    R_BSP_SoftwareDelay(50, BSP_DELAY_UNITS_MILLISECONDS);
    err = QSPI_Flash_WaitForWriteEnd();
    if (err != FSP_SUCCESS) {
        printf("FAIL (wait timeout)\r\n");
        return -1;
    }
    printf("OK\r\n");

    /* Step 4: 验证擦除结果（全0xFF） */
    printf("[4/6] Verifying erase content (expect 0xFF)... ");
    memset(rbuf, 0, sizeof(rbuf));
    QSPI_Flash_Read(test_offset, rbuf, sizeof(rbuf));
    for (uint32_t i = 0; i < sizeof(rbuf); i++) {
        if (rbuf[i] != 0xFF) {
            printf("FAIL at byte %lu: 0x%02X\r\n",
                   (unsigned long)i, rbuf[i]);
            return -1;
        }
    }
    printf("OK\r\n");

    /* Step 5: 写入测试数据 */
    for (uint32_t i = 0; i < sizeof(wbuf); i++) {
        wbuf[i] = (uint8_t)(i + 0x10);
    }
    printf("[5/6] Writing test pattern (%lu bytes)... ",
           (unsigned long)sizeof(wbuf));
    if (QSPI_Flash_Write(test_offset, wbuf, sizeof(wbuf)) != 0) {
        printf("FAIL (write error)\r\n");
        return -1;
    }
    printf("OK\r\n");

    /* Step 6: 回读验证 */
    printf("[6/6] Read-back verify... ");
    memset(rbuf, 0, sizeof(rbuf));
    QSPI_Flash_Read(test_offset, rbuf, sizeof(rbuf));
    for (uint32_t i = 0; i < sizeof(wbuf); i++) {
        if (rbuf[i] != wbuf[i]) {
            printf("FAIL at byte %lu: expected 0x%02X, got 0x%02X\r\n",
                   (unsigned long)i, wbuf[i], rbuf[i]);
            return -1;
        }
    }
    printf("OK\r\n");

    printf("========== ALL TESTS PASSED ==========\r\n");
    return 0;
}

int qspi_flash_init_fs(void) {
//	QSPI_Flash_Erase(0, 1024 * 1024);
	fs_user_mount_t *vfs_fat = &qspi_flash_fs;
	vfs_fat->blockdev.flags = 0;
    qspi_flash_init_vfs(vfs_fat);

    FRESULT res = f_mount(&vfs_fat->fatfs);

    if (res == FR_NO_FILESYSTEM) {
		// no filesystem, so create a fresh one
		static uint8_t working_buf[FF_MAX_SS];
		res = f_mkfs(&vfs_fat->fatfs, FM_FAT, 0, working_buf, sizeof(working_buf));
		if (res == FR_OK) {
			// success creating fresh LFS
			FRESULT res = f_mount(&vfs_fat->fatfs);
			if(res != FR_OK){
				 mp_hal_stdout_tx_strn("FATAL ERROR: failed to mount after mk\n", 38);
			}
			if(res == FR_OK){
				mp_hal_stdout_tx_strn("Flash OK\n", 9);
			}
		} else {
			 mp_hal_stdout_tx_strn("FATAL ERROR: failed to create\n", 30);
		}
//		// create empty main.py
//		mptask_create_main_py();
	} else if (res == FR_OK) {
		mp_hal_stdout_tx_strn("Flash OK\n", 9);
		// mount successful
//		if (FR_OK != f_stat(&vfs_fat->fatfs, "/main.py", &fno)) {
//			// create empty main.py
//			mptask_create_main_py();
//		}
	} else {
		 mp_hal_stdout_tx_strn("FATAL ERROR: failed to mount\n", 29);
	}

    mp_vfs_mount_t *vfs = m_new_obj_maybe(mp_vfs_mount_t);
	if (vfs == NULL) {
		 mp_hal_stdout_tx_strn("FATAL ERROR: failed\n", 20);
	}
	vfs->str = "/flash";
	vfs->len = 6;
	vfs->obj = MP_OBJ_FROM_PTR(vfs_fat);
	vfs->next = NULL;
	MP_STATE_VM(vfs_mount_table) = vfs;
}

int mpmain() {

	// 外设初始化
	board_init();

	// REPL初始化
	mp_uart_repl_init();

	// Note: stack control relies on main thread being initialised above
	mp_cstack_init_with_top(&_estack, (char *)&_estack - (char *)&_sstack);

	// GC init
	gc_init(MICROPY_HEAP_START, MICROPY_HEAP_END);

	// mp内核初始化
    mp_init();

    // 启动systick
    SysTick_Config(SystemCoreClock / 1000);

//    /* ========== QSPI Flash Test ========== */
//    qspi_flash_test();
//    /* ===================================== */

    // 文件系统
//    fs_user_mount_t vfs;
//    qspi_flash_init_vfs(&vfs);
//	FRESULT res = f_mount(&vfs.fatfs);
    qspi_flash_init_fs();

    // 启动交互式REPL
    pyexec_friendly_repl();

    gc_sweep_all();
    mp_deinit();
    return 0;
}



mp_lexer_t *mp_lexer_new_from_file(qstr filename) {
    mp_raise_OSError(MP_ENOENT);
}

//mp_import_stat_t mp_import_stat(const char *path) {
//    return MP_IMPORT_STAT_NO_EXIST;
//}

void nlr_jump_fail(void *val) {
    printf("FATAL: uncaught exception %p\n", val);
    mp_obj_print_exception(&mp_plat_print, MP_OBJ_FROM_PTR(val));
    __fatal_error("");
}

void MP_NORETURN __fatal_error(const char *msg) {
	for (volatile uint delay = 0; delay < 1000000; delay++) {
	    }
	mp_hal_stdout_tx_strn("\nFATAL ERROR:\n", 14);
	mp_hal_stdout_tx_strn(msg, strlen(msg));
	for (uint i = 0;;) {
		for (volatile uint delay = 0; delay < 1000000; delay++) {
		}
		if (i >= 16) {
			// to conserve power
			__WFI();
		}
	}
}

#ifndef NDEBUG
void MP_WEAK __assert_func(const char *file, int line, const char *func, const char *expr) {
    (void)func;
    printf("Assertion '%s' failed, at file %s:%d\n", expr, file, line);
    __fatal_error("");
}
#endif

void abort(void) {
    __fatal_error("abort");
}

