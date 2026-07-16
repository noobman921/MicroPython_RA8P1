from machine import Pin
import time
led = Pin(0, Pin.OUT)
while True:
    led.value(1)
    time.sleep(1)
    led.value(0)
    time.sleep(1)

from machine import UART
import time
uart = UART(0, 115200)
for i in range(8):
    uart.write(b'Hello, World!\n')
    time.sleep(1)