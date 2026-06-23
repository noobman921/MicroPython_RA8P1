/*
 * fsp_qspi_flash.c
 *
 *  Created on: 2026年6月18日
 *      Author: qq292
 */
#include "fsp_qspi_flash.h"

#include "stdio.h"

#if 0
// fsp flash设置

#define OSPI_B_CS1_START_ADDRESS            (0x90000000)
#define OSPI_B_APP_ADDRESS(sector_no)       ((uint8_t *)(OSPI_B_CS1_START_ADDRESS + ((sector_no) * OSPI_B_SECTOR_SIZE_4K)))

#define RENESAS_FLASH_START_ADDRESS         OSPI_B_CS1_START_ADDRESS

#define RENESAS_FLASH_SIZE                  FLASH_SIZE /* 8MB for W25Q64 flash */
#define RENESAS_FLASH_SECTOR_SIZE           OSPI_B_SECTOR_SIZE_4K /* 4KB sectors */
#define RENESAS_FLASH_END_ADDRESS           (RENESAS_FLASH_START_ADDRESS + RENESAS_FLASH_SIZE)


// 默认transfer
spi_flash_direct_transfer_t g_ospi_b_direct_transfer [OSPI_B_TRANSFER_MAX] =
{
 /* Transfer structure for SPI mode */
    [OSPI_B_TRANSFER_WRITE_ENABLE_SPI] =
    {
    .command        = OSPI_B_COMMAND_WRITE_ENABLE_SPI,
    .address        = OSPI_B_ADDRESS_DUMMY,
    .data           = OSPI_B_DATA_DUMMY,
    .command_length = OSPI_B_COMMAND_LENGTH_SPI,
    .address_length = OSPI_B_ADDRESS_LENGTH_ZERO,
    .data_length    = OSPI_B_DATA_LENGTH_ZERO,
    .dummy_cycles   = OSPI_B_DUMMY_CYCLE_WRITE_SPI
    },
    [OSPI_B_TRANSFER_READ_STATUS_SPI] =
    {
    .command        = OSPI_B_COMMAND_READ_STATUS_SPI,
    .address        = OSPI_B_ADDRESS_DUMMY,
    .data           = OSPI_B_DATA_DUMMY,
    .command_length = OSPI_B_COMMAND_LENGTH_SPI,
    .address_length = OSPI_B_ADDRESS_LENGTH_ZERO,
    .data_length    = OSPI_B_DATA_LENGTH_ONE,
    .dummy_cycles   = OSPI_B_DUMMY_CYCLE_READ_STATUS_SPI
    },
    [OSPI_B_TRANSFER_READ_DEVICE_ID_SPI] =
    {
    .command        = OSPI_B_COMMAND_READ_DEVICE_ID_SPI,
    .address        = OSPI_B_ADDRESS_LENGTH_ZERO,
    .data           = OSPI_B_DATA_DUMMY,
    .command_length = OSPI_B_COMMAND_LENGTH_SPI,
    .address_length = OSPI_B_ADDRESS_LENGTH_ZERO,
    .data_length    = OSPI_B_DATA_LENGTH_THREE,
    .dummy_cycles   = 16
    },
    [OSPI_B_TRANSFER_WRITE_SR2_SPI] =
    {
    .command        = OSPI_B_COMMAND_WRITE_SR2_SPI,
    .address        = OSPI_B_ADDRESS_LENGTH_ZERO,
    .data           = OSPI_B_DATA_DUMMY,
    .command_length = OSPI_B_COMMAND_LENGTH_SPI,
    .address_length = OSPI_B_ADDRESS_LENGTH_ZERO,
    .data_length    = OSPI_B_DATA_LENGTH_ONE,
    .dummy_cycles   = OSPI_B_DUMMY_CYCLE_READ_STATUS_SPI
    },
    [OSPI_B_TRANSFER_READ_SR2_SPI] =
    {
    .command        = OSPI_B_COMMAND_READ_SR2_SPI,
    .address        = OSPI_B_ADDRESS_LENGTH_ZERO,
    .data           = OSPI_B_DATA_DUMMY,
    .command_length = OSPI_B_COMMAND_LENGTH_SPI,
    .address_length = OSPI_B_ADDRESS_LENGTH_ZERO,
    .data_length    = OSPI_B_DATA_LENGTH_ONE,
    .dummy_cycles   = OSPI_B_DUMMY_CYCLE_READ_STATUS_SPI
    },
    /* Transfer structure for OPI mode */
    [OSPI_B_TRANSFER_READ_DEVICE_ID_OPI] =
    {
    .command        = OSPI_B_COMMAND_READ_DEVICE_ID_QPI,
    .address        = 0x000000,
    .data           = OSPI_B_DATA_DUMMY,
    .command_length = OSPI_B_COMMAND_LENGTH_SPI,
    .address_length = OSPI_B_ADDRESS_LENGTH_THREE,
    .data_length    = OSPI_B_DATA_LENGTH_TWO,
    .dummy_cycles   = 6
    },
};



fsp_err_t QSPI_Flash_Init(void)
{
	fsp_err_t err = FSP_SUCCESS;
	err = R_OSPI_B_Open(&RA_OSPI_FLASH_CTRL, &RA_OSPI_FLASH_CFG);
	if (FSP_SUCCESS != err)
	{
		return err;
	}
	err = R_OSPI_B_SpiProtocolSet(&RA_OSPI_FLASH_CTRL, SPI_FLASH_PROTOCOL_EXTENDED_SPI);
	if (FSP_SUCCESS != err)
	{
		return err;
	}
	spi_flash_direct_transfer_t transfer = {0};
    transfer = g_ospi_b_direct_transfer[OSPI_B_TRANSFER_READ_DEVICE_ID_SPI];
    err = R_OSPI_B_DirectTransfer(&RA_OSPI_FLASH_CTRL, &transfer, SPI_FLASH_DIRECT_TRANSFER_DIR_READ);
    if (err != FSP_SUCCESS)
    {
    }
    R_BSP_SoftwareDelay(10u, BSP_DELAY_UNITS_MILLISECONDS);
    return err;
}

fsp_err_t QSPI_Flash_WriteEnable(void)
{

	fsp_err_t err = FSP_SUCCESS;
	spi_flash_direct_transfer_t transfer = {0};

	transfer = g_ospi_b_direct_transfer[OSPI_B_TRANSFER_WRITE_ENABLE_SPI];
	err = R_OSPI_B_DirectTransfer(&RA_OSPI_FLASH_CTRL, &transfer, SPI_FLASH_DIRECT_TRANSFER_DIR_WRITE);
	if (FSP_SUCCESS != err)
	{
		return err;
	}

	transfer = g_ospi_b_direct_transfer[OSPI_B_TRANSFER_READ_STATUS_SPI];
	err = R_OSPI_B_DirectTransfer(&RA_OSPI_FLASH_CTRL, &transfer, SPI_FLASH_DIRECT_TRANSFER_DIR_READ);
	if (FSP_SUCCESS != err)
	{
		return err;
	}

	/* Check Write Enable bit in Status Register */
	if (OSPI_B_WEN_BIT_MASK != (transfer.data & OSPI_B_WEN_BIT_MASK))
	{
		return FSP_ERR_ABORTED;
	}
	return err;
}

fsp_err_t QSPI_Flash_WaitForWriteEnd(void)
{
	uint32_t timeout = (INT32_MAX);
	fsp_err_t err = FSP_SUCCESS;
	spi_flash_status_t status = {0};

	status.write_in_progress = true;
	while (status.write_in_progress)
	{
		/* Get device status */
		R_OSPI_B_StatusGet(&RA_OSPI_FLASH_CTRL, &status);
		if (FSP_SUCCESS != err)
		{
			return err;
		}
		if (TIME_OUT_VAL == timeout)
		{
			return FSP_ERR_TIMEOUT;
		}
		timeout--;
	}
	return err;
}

// offset需要是扇区的开始
int32_t QSPI_Flash_Erase(uint32_t offset, uint32_t size)
{
	fsp_err_t err;
	uint32_t addr = RENESAS_FLASH_START_ADDRESS + offset;
	uint32_t end_addr = addr + size;
	uint32_t sector_count;
	uint32_t sector_no;

	if (end_addr > RENESAS_FLASH_END_ADDRESS)
	{
		return -1;
	}

	/* Calculate starting sector and number of sectors to erase */
	sector_no = offset / RENESAS_FLASH_SECTOR_SIZE;
	sector_count = (size + RENESAS_FLASH_SECTOR_SIZE - 1) / RENESAS_FLASH_SECTOR_SIZE; /* Ceiling division */

	for (uint32_t i = 0; i < sector_count; i++)
	{
		uint32_t sector_addr = (uint32_t)OSPI_B_APP_ADDRESS(sector_no + i);

		/* Perform sector erase */
		err = R_OSPI_B_Erase(&RA_OSPI_FLASH_CTRL, (uint8_t *)sector_addr, RENESAS_FLASH_SECTOR_SIZE);
		if (err != FSP_SUCCESS)
		{
			return -1;
		}

		/* Wait for erase completion */
		err = QSPI_Flash_WaitForWriteEnd();
		if (err != FSP_SUCCESS)
		{
			return -1;
		}
	}
	return 0;
}


int32_t QSPI_Flash_Read(long offset, uint8_t *buf, uint32_t size)
{
	uint32_t addr = RENESAS_FLASH_START_ADDRESS + offset;
	if ((addr + size) > RENESAS_FLASH_END_ADDRESS)
	{
		return -1;
	}
	memcpy(buf, (void *)addr, size);
	return 0;
}

int32_t QSPI_Flash_Write(long offset, const uint8_t *buf, uint32_t size)
{
	fsp_err_t err;
	uint32_t addr = RENESAS_FLASH_START_ADDRESS + offset;
	int32_t remaining = size;
	uint8_t *p_buf = (uint8_t *)buf;
	const uint32_t chunk_size = 4; // Fixed 4-byte write size

	// Input validation
	if (!buf || size == 0)
	{
		return -1;
	}

	if ((addr + size) > RENESAS_FLASH_END_ADDRESS)
	{
		return -1;
	}

	// Write data in 4-byte chunks
	while (remaining > 0)
	{
		// Calculate current chunk size (up to 4 bytes or remaining bytes)
		uint32_t current_size = (remaining >= chunk_size) ? chunk_size : remaining;

		// Perform write operation
		err = R_OSPI_B_Write(&RA_OSPI_FLASH_CTRL, p_buf, (uint8_t *)addr, current_size);
		if (err != FSP_SUCCESS)
		{
			return -1;
		}

		// Wait for write operation to complete
		err = QSPI_Flash_WaitForWriteEnd();
		if (err != FSP_SUCCESS)
		{
			return -1;
		}

		// Update address, buffer pointer, and remaining bytes
		addr += current_size;
		p_buf += current_size;
		remaining -= current_size;
	}

	return 0;
}
#endif
