///*
// * flash.c
// *
// *  Created on: 2026年6月18日
// *      Author: qq292
// */
#include "py/runtime.h"
#include "lib/oofatfs/ff.h"
#include "lib/oofatfs/diskio.h"
#include "extmod/vfs_fat.h"

#include "drivers/fsp_qspi_flash.h"
DWORD get_fattime(void) {
    // 如果 RTC 未实现，返回固定值（如 0）
    // 格式: (year-1980)<<25 | month<<21 | day<<16 | hour<<11 | min<<5 | sec>>1
    return 0;
}

#if 0

extern const struct _mp_obj_type_t qspi_flash_type;

// 驱动
DRESULT flash_disk_read(BYTE *buff, DWORD sector, UINT count){
    uint32_t addr = sector * FLASH_SECTOR_SIZE;
    size_t size = count * FLASH_SECTOR_SIZE;
    // 调用你之前实现的 QSPI_Flash_Read 函数
    int ret = QSPI_Flash_Read(addr, buff, size);
    if (ret != 0) {
        return RES_ERROR;
    }
    return RES_OK;
}

DRESULT flash_disk_write(const BYTE *buff, DWORD sector, UINT count){
	uint32_t offset = sector * FLASH_SECTOR_SIZE;
	size_t size = count * FLASH_SECTOR_SIZE;
//	int ret = QSPI_Flash_Erase(offset, size);
//	if (ret != 0) {
//		return RES_ERROR;
//	}

	int ret = QSPI_Flash_Write(offset, buff, size);
	if (ret != 0) {
		return RES_ERROR;
	}
	return RES_OK;
}

void flash_disk_flush(void) {
    QSPI_Flash_WaitForWriteEnd();
}

int32_t flash_disk_erase(uint32_t offset, uint32_t size){
	return QSPI_Flash_Erase(offset, size);
}

/******************************************************************************/
// MicroPython bindings to expose the internal flash as an object with the
// block protocol.

// there is a singleton Flash object
static const mp_obj_base_t qspi_flash_obj = {&qspi_flash_type};

static mp_obj_t qspi_flash_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 0, 0, false);
    // return singleton object
    return (mp_obj_t)&qspi_flash_obj;
}

static mp_obj_t qspi_flash_readblocks(mp_obj_t self, mp_obj_t block_num, mp_obj_t buf) {
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf, &bufinfo, MP_BUFFER_WRITE);
    DRESULT res = flash_disk_read(bufinfo.buf, mp_obj_get_int(block_num), bufinfo.len / FLASH_SECTOR_SIZE);
    return MP_OBJ_NEW_SMALL_INT(res != RES_OK); // return of 0 means success
}
static MP_DEFINE_CONST_FUN_OBJ_3(qspi_flash_readblocks_obj, qspi_flash_readblocks);

static mp_obj_t qspi_flash_writeblocks(mp_obj_t self, mp_obj_t block_num, mp_obj_t buf) {
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf, &bufinfo, MP_BUFFER_READ);
    DRESULT res = flash_disk_write(bufinfo.buf, mp_obj_get_int(block_num), bufinfo.len / FLASH_SECTOR_SIZE);
    return MP_OBJ_NEW_SMALL_INT(res != RES_OK); // return of 0 means success
}
static MP_DEFINE_CONST_FUN_OBJ_3(qspi_flash_writeblocks_obj, qspi_flash_writeblocks);

static mp_obj_t qspi_flash_ioctl(mp_obj_t self, mp_obj_t cmd_in, mp_obj_t arg_in) {
    mp_int_t cmd = mp_obj_get_int(cmd_in);
    switch (cmd) {
        case MP_BLOCKDEV_IOCTL_INIT: return MP_OBJ_NEW_SMALL_INT(0);
        case MP_BLOCKDEV_IOCTL_DEINIT: return MP_OBJ_NEW_SMALL_INT(0);
        case MP_BLOCKDEV_IOCTL_SYNC: flash_disk_flush(); return MP_OBJ_NEW_SMALL_INT(0);
        case MP_BLOCKDEV_IOCTL_BLOCK_COUNT: return MP_OBJ_NEW_SMALL_INT(FLASH_SECTOR_COUNT);
        case MP_BLOCKDEV_IOCTL_BLOCK_SIZE: return MP_OBJ_NEW_SMALL_INT(FLASH_SECTOR_SIZE);
        case MP_BLOCKDEV_IOCTL_BLOCK_ERASE: {
			uint32_t offset = mp_obj_get_int(arg_in) * FLASH_SECTOR_SIZE;
			int ret = flash_disk_erase(offset, FLASH_SECTOR_SIZE);
			return MP_OBJ_NEW_SMALL_INT(ret);
		}
        default: return mp_const_none;
    }
}
static MP_DEFINE_CONST_FUN_OBJ_3(qspi_flash_ioctl_obj, qspi_flash_ioctl);

static const mp_rom_map_elem_t qspi_flash_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_readblocks), MP_ROM_PTR(&qspi_flash_readblocks_obj) },
    { MP_ROM_QSTR(MP_QSTR_writeblocks), MP_ROM_PTR(&qspi_flash_writeblocks_obj) },
    { MP_ROM_QSTR(MP_QSTR_ioctl), MP_ROM_PTR(&qspi_flash_ioctl_obj) },
};

static MP_DEFINE_CONST_DICT(qspi_flash_locals_dict, qspi_flash_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    qspi_flash_type,
    MP_QSTR_Flash,
    MP_TYPE_FLAG_NONE,
    make_new, qspi_flash_make_new,
    locals_dict, &qspi_flash_locals_dict
    );


void qspi_flash_init_vfs(struct _fs_user_mount_t * vfs) {
    vfs->base.type = &mp_fat_vfs_type;
    vfs->blockdev.flags |= MP_BLOCKDEV_FLAG_NATIVE | MP_BLOCKDEV_FLAG_HAVE_IOCTL;
    vfs->fatfs.drv = vfs;
    vfs->blockdev.readblocks[0] = (mp_obj_t)&qspi_flash_readblocks_obj;
    vfs->blockdev.readblocks[1] = (mp_obj_t)&qspi_flash_obj;
    vfs->blockdev.readblocks[2] = (mp_obj_t)flash_disk_read; // native version
    vfs->blockdev.writeblocks[0] = (mp_obj_t)&qspi_flash_writeblocks_obj;
    vfs->blockdev.writeblocks[1] = (mp_obj_t)&qspi_flash_obj;
    vfs->blockdev.writeblocks[2] = (mp_obj_t)flash_disk_write; // native version
    vfs->blockdev.u.ioctl[0] = (mp_obj_t)&qspi_flash_ioctl_obj;
    vfs->blockdev.u.ioctl[1] = (mp_obj_t)&qspi_flash_obj;
}

#endif
