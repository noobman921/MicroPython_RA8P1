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

static fs_user_mount_t qspi_flash_fs;
static const mp_obj_t mount_point = MP_OBJ_NEW_QSTR(MP_QSTR__slash_flash);

int qspi_flash_init_fs(void) {
	QSPI_Flash_Erase(0, 4096);
	fs_user_mount_t *vfs_fat = &qspi_flash_fs;
	vfs_fat->blockdev.flags = 0;
    qspi_flash_init_vfs(vfs_fat);

    FRESULT res = f_mount(&vfs_fat->fatfs);

    if (res == FR_NO_FILESYSTEM) {
		// no filesystem, so create a fresh one
		uint8_t working_buf[FF_MAX_SS];
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
//    // 文件系统
//    fs_user_mount_t vfs;
//    qspi_flash_init_vfs(&vfs);
//	FRESULT res = f_mount(&vfs.fatfs);
    //qspi_flash_init_fs();

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

