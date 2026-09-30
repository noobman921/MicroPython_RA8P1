# 代码框架介绍
```text
MicroPython/
├── ports/                  # 移植层
│   ├── drivers/            # 移植层驱动，主要服务于machine模块
│   ├── mpconfigboard.h     # mpy板级头文件，在本项目中无需关注
│   ├── mpconfigport.h      # mpy移植层头文件，在本项目中无需关注
│   ├── mpy_board_cfg.c     # ⭐本项目的设置文件，移植时需重点关注
│   └── mpy_board_cfg.h     # ⭐本项目的设置文件，移植时需重点关注
├── extmod/                 # 额外模块（如果强依赖于某些移植层模块，在裁剪时需要排除编译）
├── drivers/                # 驱动文件
├── genhdr/                 # 生成头文件，当前项目中由gen_qstr.py生成
├── py/                     # mpy核心文件
├── shared/                 # ports共享的代码文件
├── lib/                    # 外部库
├── user/                   # 用户文件
└── gen_qstr.py             # 头文件生成脚本
```


# 目前已支持的API
**可能有部分api已支持但未包含在本列表，具体请使用help(xxx)查询**
***
## Machine模块
**from machine import xxx**
### Pin模块
1. Pin(id, mode, value)
构造函数
id:int类型
mode: Pin.IN, Pin.OUT
value:bool类型
2. pin.init(mode, value)
初始化引脚
mode: Pin.IN, Pin.OUT
value:bool类型
3. pin.value([x])
无参数: 读取引脚
&nbsp;&nbsp;返回引脚的值
有参数: 设置引脚
&nbsp;&nbsp;x:bool类型
4. pin.on() Pin.high() 拉高引脚
5. pin.off() Pin.low() 拉低引脚

### RTC模块
1. RTC()
构造函数
2. rtc.datetime([datetimetuple])
无参数: 读取时间
&nbsp;&nbsp;返回8元组
有参数: 设置时间
&nbsp;&nbsp;datetimetuple:8元组 （年、月、日、小时、分钟、秒、保留、 保留）
3. rtc.init([datetimetuple])
初始化时间
datetimetuple:8元组 （年、月、日、小时、分钟、秒、保留、 保留）
4. rtc.deinit()
复位时间

### SPI模块
1. SPI(id, polarity, phase, bits, firstbit)
构造函数
id:int类型
polarity:int类型 空闲时钟线所在的电平
phase:int类型 0 或 1 分别在第一个或第二个时钟沿采样数据
bits:int类型 传输的位宽度
firstbit:SPI.MSB或SPI.LSB 高低位有效
2. spi.init(polarity, phase, bits, firstbit)
polarity:int类型 空闲时钟线所在的电平
phase:int类型 0 或 1 分别在第一个或第二个时钟沿采样数据
bits:int类型 传输的位宽度
firstbit:SPI.MSB或SPI.LSB 高低位有效
3. 其他接口
spi.deinit()
spi.read()
spi.readinto()
spi.write()
spi.write_readinto()
见http://micropython.com.cn/en/latet/library/machine.SPI.html

### UART模块
1. UART(id, baudrate, bits, parity, stop)
构造函数
id:int类型
baudrate:int类型 波特率
bits:int类型  字符位数
parity:int类型 奇偶校验 0无 1奇 2偶
stop:int类型 停止位
2. uart.init(baudrate, bits, parity, stop)
初始化串口
baudrate:int类型 波特率
bits:int类型  字符位数
parity:int类型 奇偶校验 0无 1奇 2偶
stop:int类型 停止位
3. 其他接口
uart.deinit()
uart.any()
uart.read()
uart.readinto()
uart.readline()
uart.write()
见http://micropython.com.cn/en/latet/library/machine.UART.html

### I2C模块
1. I2C(id, freq)
构造函数
id:int类型
freq:int类型 频率
2. 其他接口
i2c.readfrom()
i2c.readfrom_into()
i2c.writeto()
i2c.writevto()

### PWM模块
1. PWM(id, freq, duty_u16, duty_ns)
构造函数
id:int类型
freq:int类型 频率
duty_u16:int类型 占空比
duty_ns:int类型 占空比
2. pwm.init(freq, duty_u16, duty_ns)
freq:int类型 频率
duty_u16:int类型 占空比
duty_ns:int类型 占空比
3. 其他接口
pwm.freq()
pwm.duty_u16()
pwm.duty_ns()
占空比设置以最后一次为准。
***

### time模块
1.time.sleep(seconds)
休眠指定秒
seconds:int类型
2.time.sleep_ms(ms)
休眠指定毫秒
ms:int类型

# 如何添加自定义模块
模板文件:MicroPython/user/mpy_gpio.c  
下面进行详细的介绍。  
```c
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
```
通过宏将模块对象注册到mpy后，就可以在mpy中import对应的模块。模块字典就表示了模块包含的接口。模块的接口则需要通过宏去将c函数转化成mpy可识别的模式。整体流程就如上所示。该模板中  
```c
static mp_obj_t py_led_off(void) {
	led_check_initialized();
	R_IOPORT_PinWrite(&g_ioport_ctrl, BSP_IO_PORT_01_PIN_08, BSP_IO_LEVEL_HIGH);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(led_off_obj, py_led_off);

```  
这部分需要根据函数类型去修改。  
| 宏名称 | 参数个数 | 典型 C 函数签名 |
|------|------|------|
| MP_DEFINE_CONST_FUN_OBJ_0 | 0 | mp_obj_t f(void) |
| MP_DEFINE_CONST_FUN_OBJ_1 | 1 | mp_obj_t (mp_obj_t a) |
| MP_DEFINE_CONST_FUN_OBJ_2 | 2 | mp_obj_t f(mp_obj_t a, mp_obj_t b) |
|MP_DEFINE_CONST_FUN_OBJ_3 | 3 | mp_obj_t f(mp_obj_t a, mp_obj_t b, mp_obj_t c) |
| MP_DEFINE_CONST_FUN_OBJ_VAR | ≥ N | mp_obj_t f(size_t n, mp_obj_t *args) |
| MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN | 范围 | mp_obj_t f(size_t n, mp_obj_t *args) |
| MP_DEFINE_CONST_FUN_OBJ_KW | ≥ N + 关键字 | mp_obj_t f(size_t n, mp_obj_t *args, mp_map_t *kw) |