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