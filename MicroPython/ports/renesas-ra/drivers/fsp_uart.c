#include "fsp_uart.h"

// 波特率误差
#define ERROR_MAX  (5*1000)

// 缓存区大小
#define UART_BUF_SIZE	 16


typedef struct _sci_uart_fifo {
    volatile uint32_t tail, head, len, busy;
    uint8_t *bufp;
    uint32_t size;
} sci_uart_fifo;

static uint8_t uart_buf[RA_UART_NUM][UART_BUF_SIZE];
static volatile sci_uart_fifo uart_fifo[RA_UART_NUM];

/* Callback function */
void mpy_uart_callback(uart_callback_args_t *p_args)
{
	/* TODO: add your own code here */
	if(p_args->event == UART_EVENT_RX_CHAR){
		uint8_t id = *(uint8_t*)p_args->p_context;

		volatile sci_uart_fifo *fifo = &uart_fifo[id];
		if(fifo->len < fifo->size){
		   fifo->bufp[fifo->head] = p_args->data;
		   fifo->head = (fifo->head + 1) % fifo->size;
		   fifo->len++;
		}
	}
}

int32_t fsp_uart_config(uart_ctrl_t* const ctrl, uart_cfg_t* const cfg){
	fsp_err_t err = R_SCI_B_UART_Open(ctrl, cfg);
	if(err != FSP_SUCCESS){
		return -1;
	}
	return 0;
}

int32_t fsp_uart_set_baudrate(uart_ctrl_t* const ctrl, uint32_t baudrate){
	sci_b_baud_setting_t baud_setting;
	uint32_t             baud_rate                 = baudrate;
	bool                 enable_bitrate_modulation = false;
	uint32_t             error_rate_x_1000         = ERROR_MAX;
	// 计算波特率
	fsp_err_t err = R_SCI_B_UART_BaudCalculate(baud_rate, enable_bitrate_modulation, error_rate_x_1000, &baud_setting);
	if(FSP_SUCCESS != err){
		return -1;
	}
	// 设置波特率
	err = R_SCI_B_UART_BaudSet(ctrl, (void *) &baud_setting);
	if(FSP_SUCCESS != err){
		return -1;
	}
	return 0;
}

int32_t fsp_uart_close(uart_ctrl_t* const ctrl){
	fsp_err_t err = R_SCI_B_UART_Close(ctrl);
	if(err != FSP_SUCCESS){
		return -1;
	}
	return 0;
}

void fsp_uart_buf_init(uint8_t id) {
	uart_fifo[id].tail = 0;
	uart_fifo[id].head = 0;
	uart_fifo[id].len  = 0;
	uart_fifo[id].bufp = uart_buf[id];
	uart_fifo[id].size = UART_BUF_SIZE;
}

uint32_t fsp_uart_rx_avail(uint8_t id) {
	return uart_fifo[id].len;
}

bool fsp_uart_tx_complete(uart_ctrl_t* const ctrl) {
	sci_b_uart_instance_ctrl_t *ins_ctrl = ctrl;
	R_SCI_B0_Type *sci = (R_SCI_B0_Type *)ins_ctrl->p_reg;
	return (sci->CSR_b.TDRE == 1) && (sci->CSR_b.TEND == 1);
}

uint32_t fsp_uart_read(uint8_t id, uint8_t *buffer, uint32_t len) {
	volatile sci_uart_fifo *fifo = &uart_fifo[id];
	if (len == 0 || fifo->len == 0) {
		return 0;
	}
	uint32_t available = fifo->len;
	if (available > len) available = len;
	uint32_t i;
	// 禁用中断以保护 FIFO 操作
	__disable_irq();
	for (i = 0; i < available; i++) {
		buffer[i] = fifo->bufp[fifo->tail];
		fifo->tail = (fifo->tail + 1) % fifo->size;
		fifo->len--;
	}
	__enable_irq();
	return available;
}

uint32_t fsp_uart_write(uart_ctrl_t* const ctrl, uint8_t *buffer, uint32_t len) {
	volatile uint32_t sent = 0;
	while (sent < len) {
		R_SCI_B_UART_Write(ctrl, &buffer[sent], 1);
		sci_b_uart_instance_ctrl_t *u_ctrl = ctrl;
		R_SCI_B0_Type *sci = (R_SCI_B0_Type *)u_ctrl->p_reg;
		while ((sci->CSR_b.TDRE == 0) || (sci->CSR_b.TEND == 0)) {
			// 等待硬件真正发送完成
		}
		sent++;
	}
	return sent;
}
