# 使用指导

## 归档文件

请使用e2studio导入完整的solution项目

## MicroPython软件包

添加到任意项目中，需要包含fsp对应的头文件。
根据以下要求配置项目。

#### 编译设置
排除下列文件的编译:
ports/renesas-ra/machine_pwm.c
ports/renesas-ra/machine_uart.c
  
添加头文件设置:
直接导入include.xml
或
手动添加下列路径
>   MicroPython
    MicroPython/ports/renesas-ra
    MicroPython/lib/tinyusb/hw
    MicroPython/lib/tinyusb/src
    MicroPython/shared/tinyusb

构建设置:
Optimization Level: -O0
Include files: ports/renesas-ra/mpconfigport.h

ld文件:
在ld文件中添加misc.txt中的fsp.ld相关内容

设置以下模块回调函数(函数名参考misc.txt)：
repl, uart, spi

## 修改外设接口
ports/renesas-ra/mpy_board.cfg.h 此文件中设置外设的数量
ports/renesas-ra/mpy_board.cfg.c 此文件中设置外设的句柄，需要注意init函数中需要设置cfg句柄，全局变量中设置ctrl句柄