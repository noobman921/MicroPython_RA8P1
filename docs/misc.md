fsp.ld:


__MP_RAM_END = RAM_START + RAM_LENGTH;

_etext      = __flash_readonly$$Limit;
_sidata     = __ram_from_flash$$Load;
_ram_start  = RAM_START;
_sdata      = __ram_from_flash$$Base;
_edata      = __ram_from_flash$$Limit;
_sbss       = __ram_zero$$Base;
_ebss       = __ram_zero$$Limit;
_heap_start = __ram_thread_stack$$Limit;
_heap_end   = __MP_RAM_END;
_sstack     = g_main_stack;
_estack     = g_main_stack + 0x1000;
_ram_end    = __MP_RAM_END;

REPL:
Name mpy_repl_uart
Callback mpy_repl_uart_callback

UART:
Callback mpy_uart_callback

SPI:
Callback mpy_spi_callback