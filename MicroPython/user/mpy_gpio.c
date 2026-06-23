// modled.c
#include "py/runtime.h"
#include "py/mphal.h"
#include "hal_data.h"


// 静态变量：存储 LED 引脚号和当前状态
static int led_pin = -1;      // -1 表示未初始化


// 辅助函数：检查 LED 是否已初始化，否则抛出异常
static void led_check_initialized(void) {
    if (led_pin == -1) {
        mp_raise_ValueError(MP_ERROR_TEXT("LED not initialized"));
    }
}

// led.init()
static mp_obj_t py_led_init(void) {
	R_IOPORT_Open(&g_ioport_ctrl, &g_bsp_pin_cfg);
	led_pin = 0;
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(led_init_obj, py_led_init);

// led.on()
static mp_obj_t py_led_on(void) {
    led_check_initialized();
    R_IOPORT_PinWrite(&g_ioport_ctrl, BSP_IO_PORT_01_PIN_08, BSP_IO_LEVEL_LOW);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(led_on_obj, py_led_on);

// led.off()
static mp_obj_t py_led_off(void) {
	led_check_initialized();
	R_IOPORT_PinWrite(&g_ioport_ctrl, BSP_IO_PORT_01_PIN_08, BSP_IO_LEVEL_HIGH);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(led_off_obj, py_led_off);


// 模块字典
static const mp_rom_map_elem_t led_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_led) },
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&led_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_on),    MP_ROM_PTR(&led_on_obj) },
    { MP_ROM_QSTR(MP_QSTR_off),   MP_ROM_PTR(&led_off_obj) },
};
static MP_DEFINE_CONST_DICT(led_module_globals, led_module_globals_table);

// 模块对象
const mp_obj_module_t mp_module_led = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&led_module_globals,
};

// 注册模块（需配合条件编译宏）
MP_REGISTER_MODULE(MP_QSTR_led, mp_module_led);

