#include <unistd.h>
#include "py/mpconfig.h"

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

	while(fifo->len == 0){
//		return -1;
	}

	uint32_t state = __get_PRIMASK();
	__disable_irq();

	uint8_t c = fifo->bufp[fifo->tail];
	fifo->tail = (fifo->tail + 1) % fifo->size;
	fifo->len--;

	__set_PRIMASK(state);

    return (int)c;
}

// Send string of given length
mp_uint_t mp_hal_stdout_tx_strn(const char *str, mp_uint_t len) {
	const uint8_t *buf = (const uint8_t *)str;
	volatile size_t sent = 0;
	while (sent < len) {
		R_SCI_B_UART_Write(&RA_REPL_CTRL, &buf[sent], 1);
		sci_b_uart_instance_ctrl_t *ctrl = &RA_REPL_CTRL;
		R_SCI_B0_Type *sci = (R_SCI_B0_Type *)ctrl->p_reg;
		while ((sci->CSR_b.TDRE == 0) || (sci->CSR_b.TEND == 0)) {
			// 等待硬件真正发送完成
		}
		sent++;
	}
	return len;
}
