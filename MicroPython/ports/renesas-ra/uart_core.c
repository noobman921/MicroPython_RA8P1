#include <unistd.h>
#include "py/mpconfig.h"
#include "py/runtime.h"
#include "shared/tinyusb/mp_usbd_cdc.h"
#include "mphalport.h"

typedef struct _sci_fifo {
    volatile uint32_t tail, head, len, busy;
    uint8_t *bufp;
    uint32_t size;
} sci_fifo;

#define SCI_RX_BUF_SIZE 256

static uint8_t repl_rx_buf[SCI_RX_BUF_SIZE];
static volatile sci_fifo repl_rx_fifo;

// REPL串口

void mpy_repl_uart_callback(uart_callback_args_t *p_args){
	if (p_args->event == UART_EVENT_RX_CHAR) {
	   // 收到一个字节
	   volatile sci_fifo *fifo = &repl_rx_fifo;
	   if(fifo->len < fifo->size){
		   fifo->bufp[fifo->head] = p_args->data;
		   fifo->head = (fifo->head + 1) % fifo->size;
		   fifo->len++;
	   }
   }
}

void mp_uart_repl_init(void){
	repl_rx_fifo.tail = 0;
	repl_rx_fifo.head = 0;
	repl_rx_fifo.len  = 0;
	repl_rx_fifo.bufp = repl_rx_buf;
	repl_rx_fifo.size = SCI_RX_BUF_SIZE;
}

// Receive single character
int mp_hal_stdin_rx_chr(void) {
	volatile sci_fifo *fifo = &repl_rx_fifo;
	for(;;){
	    if(fifo->len != 0){
	        uint32_t state = __get_PRIMASK();
            __disable_irq();

            uint8_t c = fifo->bufp[fifo->tail];
            fifo->tail = (fifo->tail + 1) % fifo->size;
            fifo->len--;

            __set_PRIMASK(state);
            return (int)c;
	    }
        #if MICROPY_HW_USB_CDC && MICROPY_HW_TINYUSB_STACK
        mp_usbd_cdc_poll_interfaces(0);
        int c = ringbuf_get(&stdin_ringbuf);
        if (c != -1) {
            return c;
        }
        #endif
        MICROPY_EVENT_POLL_HOOK;
	}
}

// Send string of given length
mp_uint_t mp_hal_stdout_tx_strn(const char *str, mp_uint_t len) {
    mp_uint_t ret = len;
	const uint8_t *buf = (const uint8_t *)str;
	sci_b_uart_instance_ctrl_t *ctrl = &RA_REPL_CTRL;
	R_SCI_B0_Type *sci = (R_SCI_B0_Type *)ctrl->p_reg;

	for (mp_uint_t i = 0; i < len; i++) {
		// 等TDRE为1，TDR寄存器为空
		while (!(sci->CSR & R_SCI_B0_CSR_TDRE_Msk)) { }
		// 直接写数据到TDR，硬件自动发送
		sci->TDR = buf[i];
	}
	// 等最后一个字节完全发出
	while (!(sci->CSR & R_SCI_B0_CSR_TEND_Msk)) { }

    #if MICROPY_HW_USB_CDC && MICROPY_HW_TINYUSB_STACK
    mp_uint_t cdc_res = mp_usbd_cdc_tx_strn(str, len);
    if (cdc_res > 0) {
        ret = MIN(cdc_res, ret);
    }
    #endif
	return ret;
}
