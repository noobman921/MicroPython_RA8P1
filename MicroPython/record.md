# MicroPython移植记录
## 添加文件
/ports/renesas
/py
/shared

## 添加头文件路径
/src/MicroPython
/src/MicroPython/ports/renesas-ra



## 编译器相关
设置全局头文件 mpconfigport.h


## 备忘录
/extmod/modmachine.h 104~115被注释
Type "help()" for more information.
>>> from machine import UART
>>> import time
>>> uart = UART(0, 115200)
>>> for i in range(8):