/**
 @file EVE_HAL.c
 */
/*
 * ============================================================================
 * History
 * =======
 * 2017-03-15 : Created
 *
 * (C) Copyright Bridgetek Pte Ltd
 * ============================================================================
 *
 * This source code ("the Software") is provided by Bridgetek Pte Ltd
 * ("Bridgetek") subject to the licence terms set out
 * http://www.ftdichip.com/FTSourceCodeLicenceTerms.htm ("the Licence Terms").
 * You must read the Licence Terms before downloading or using the Software.
 * By installing or using the Software you agree to the Licence Terms. If you
 * do not agree to the Licence Terms then do not download or use the Software.
 *
 * Without prejudice to the Licence Terms, here is a summary of some of the key
 * terms of the Licence Terms (and in the event of any conflict between this
 * summary and the Licence Terms then the text of the Licence Terms will
 * prevail).
 *
 * The Software is provided "as is".
 * There are no warranties (or similar) in relation to the quality of the
 * Software. You use it at your own risk.
 * The Software should not be used in, or for, any medical device, system or
 * appliance. There are exclusions of Bridgetek liability for certain types of loss
 * such as: special loss or damage; incidental loss or damage; indirect or
 * consequential loss or damage; loss of income; loss of business; loss of
 * profits; loss of revenue; loss of contracts; business interruption; loss of
 * the use of money or anticipated savings; loss of information; loss of
 * opportunity; loss of goodwill or reputation; and/or loss of, damage to or
 * corruption of data.
 * There is a monetary cap on Bridgetek's liability.
 * The Software may have subsequently been amended by another user and then
 * distributed by that other user ("Adapted Software").  If so that user may
 * have additional licence terms that apply to those amendments. However, Bridgetek
 * has no liability in relation to those amendments.
 * ============================================================================
 */

#include <string.h>
#include <stdint.h> // for Uint8/16/32 and Int8/16/32 data types

#include "../include/HAL.h"
#include "../include/MCU.h"

// Used to navigate command ring buffer
static uint16_t writeCmdPointer = 0x0000;


void HAL_Init(void)
{
	MCU_Init();

	// Set Chip Select OFF
	MCU_CShigh();

	// Reset the display
	MCU_Delay_20ms();
	MCU_PDlow();
	MCU_Delay_20ms();
	MCU_PDhigh();
	MCU_Delay_20ms();

	// Set active
	HAL_CmdWrite(0, 0x00);

#ifndef FT81X_ENABLE
	//FT80x_selection - Uncomment these four lines below if using FT800 e.g. VM800B
	// Note that some demos in this code are not compatible with the FT80x series as
	// they use commands and features introduced on the FT81x.
	HAL_CmdWrite(0x44, 0x00); // 0x44 = HostCMD_CLKEXT
	HAL_CmdWrite(0x62, 0x00); // 0x64 = HostCMD_CLK48M
	HAL_CmdWrite(0x68, 0x00); // 0x68 = HostCMD_CORE RESET
#endif

	MCU_Delay_500ms();

	// Read REG_ID register (0x302000) until reads 0x7C
	while (HAL_MemRead8(REG_ID) != 0x7C)
	{
	}

	// Ensure CPUreset register reads 0 and so FT8xx is ready
	while (HAL_MemRead8(REG_CPURESET) != 0x00)
	{
	}

	// This function will not return unless and FT8xx device is present.
	MCU_Setup();
}

// --------------------- Chip Select line ----------------------------------
void HAL_ChipSelect(int8_t enable)
{
	if (enable)
		MCU_CSlow();
	else
		MCU_CShigh();
}

// -------------------------- Power Down line --------------------------------------
void HAL_PowerDown(int8_t enable)
{
	if (enable)
		MCU_PDhigh();
	else
		MCU_PDlow();
}

// ------------------ Send FT81x register address for writing ------------------
void HAL_SetWriteAddress(uint32_t address)
{
	// Send three bytes of a register address which has to be subsequently
	// written. Ignore return values as this is an SPI write only.
	// Send high byte of address with 'write' bits set.
	MCU_SPIWrite24(MCU_htobe32((address << 8) | (1 << 31)));
}

// ------------------ Send FT81x register address for reading ------------------
void HAL_SetReadAddress(uint32_t address)
{
	// Send three bytes of a register address which has to be subsequently read.
	// Ignore return values as this is an SPI write only.
	// Send high byte of address with 'read' bits set.
	MCU_SPIWrite32(MCU_htobe32((address << 8) | (0 << 31)));
}

// ------------------------ Send a block of data --------------------------
void HAL_Write(const uint8_t *buffer, uint32_t length)
{
	// Send multiple bytes of data after previously sending address. Ignore return
	// values as this is an SPI write only. Data must be the correct endianess
	// for the SPI bus.
	MCU_SPIWrite(buffer, length);
}

// ------------------------ Send a 32-bit data value --------------------------
void HAL_Write32(uint32_t val32)
{    
	// Send four bytes of data after previously sending address. Ignore return
	// values as this is an SPI write only.
	MCU_SPIWrite32(MCU_htole32(val32));
}

// ------------------------ Send a 16-bit data value --------------------------
void HAL_Write16(uint16_t val16)
{
	// Send two bytes of data after previously sending address. Ignore return
	// values as this is an SPI write only.
	MCU_SPIWrite16(MCU_htole16(val16));
}

// ------------------------ Send an 8-bit data value ---------------------------
void HAL_Write8(uint8_t val8)
{
	// Send one byte of data after previously sending address. Ignore return
	// values as this is an SPI write only.
	MCU_SPIWrite8(val8);
}

// ------------------------ Read a 32-bit data value --------------------------
uint32_t HAL_Read32(void)
{    
	// Read 4 bytes from a register has been previously addressed. Send dummy
	// 00 bytes as only the incoming value is important.
	uint32_t val32;

	// Read low byte of data first.
	val32 = MCU_SPIRead32();

	// Return combined 32-bit value
	return MCU_le32toh(val32);
}

// ------------------------ Read a 16-bit data value ---------------------------
uint16_t HAL_Read16(void)
{
	// Read 2 bytes from a register has been previously addressed. Send dummy
	// 00 bytes as only the incoming value is important.
	uint16_t val16;

	// Read low byte of data first.
	val16 = MCU_SPIRead16();

	// Return combined 16-bit value
	return MCU_le16toh(val16);
}

// ------------------------ Read an 8-bit data value ---------------------------
uint8_t HAL_Read8(void)
{
	// Read 1 byte from a register has been previously addressed. Send dummy
	// 00 byte as only the incoming value is important.
	uint8_t val8;

	val8 = MCU_SPIRead8();

	// Return 8-bit value read
	return val8;
}

// ################# COMBINED ADDRESSING AND DATA FUNCTIONS ####################

// This section has combined calls which carry out a full write or read cycle
// including chip select, address, and data transfer.
// This would often be used for register writes and reads. 

// -------------- Write a 32-bit value to specified address --------------------
void HAL_MemWrite32(uint32_t address, uint32_t val32)
{
	// CS low begins the SPI transfer
	MCU_CSlow();
	// Send address to be written
	HAL_SetWriteAddress(address);
	// Send the data value
	HAL_Write32(val32);
	// CS high terminates the SPI transfer
	MCU_CShigh();
}

// -------------- Write a 16-bit value to specified address --------------------
void HAL_MemWrite16(uint32_t address, uint16_t val16)
{
	// CS low begins the SPI transfer
	MCU_CSlow();
	// Send address to be written
	HAL_SetWriteAddress(address);
	// Send the data value
	HAL_Write16(val16);
	// CS high terminates the SPI transfer
	MCU_CShigh();
}

// -------------- Write an 8-bit value to specified address --------------------
void HAL_MemWrite8(uint32_t address, uint8_t val8)
{
	// CS low begins the SPI transfer
	MCU_CSlow();
	// Send address to be written
	HAL_SetWriteAddress(address);
	// Send the data value
	HAL_Write8(val8);
	// CS high terminates the SPI transfer
	MCU_CShigh();
}

// -------------- Read a 32-bit value from specified address --------------------
uint32_t HAL_MemRead32(uint32_t address)
{
	uint32_t val32;

	// CS low begins the SPI transfer
	MCU_CSlow();
	// Send address to be read
	HAL_SetReadAddress(address);
	// Read the data value
	val32 = HAL_Read32();
	// CS high terminates the SPI transfer
	MCU_CShigh();

	// Return 32-bit value read
	return val32;
}
// -------------- Read a 16-bit value from specified address --------------------
uint16_t HAL_MemRead16(uint32_t address)
{
	uint16_t val16;

	// CS low begins the SPI transfer
	MCU_CSlow();
	// Send address to be read
	HAL_SetReadAddress(address);
	// Read the data value
	val16 = HAL_Read16();
	// CS high terminates the SPI transfer
	MCU_CShigh();

	// Return 16-bit value read
	return val16;
}
// -------------- Read an 8-bit value from specified address --------------------
uint8_t HAL_MemRead8(uint32_t address)
{
	uint8_t val8;

	// CS low begins the SPI transfer
	MCU_CSlow();
	// Send address to be read
	HAL_SetReadAddress(address);
	// Read the data value
	val8 = HAL_Read8();
	// CS high terminates the SPI transfer
	MCU_CShigh();

	// Return 8-bit value read
	return val8;
}
// ############################# HOST COMMANDS #################################
// -------------------------- Write a host command -----------------------------
void HAL_CmdWrite(uint8_t cmd, uint8_t param)
{
	// CS low begins the SPI transfer
	MCU_CSlow();
	// Send command
	MCU_SPIWrite8(cmd);
	// followed by parameter
	MCU_SPIWrite8(param);
	// and a dummy 00 byte
	MCU_SPIWrite8(0x00);
	// CS high terminates the SPI transfer
	MCU_CShigh();
}
// ######################## SUPPORTING FUNCTIONS ###############################

// --------- Increment co-processor address offset counter --------------------
void HAL_IncCmdPointer(uint16_t commandSize)
{
	// Calculate new offset
	writeCmdPointer = (writeCmdPointer + commandSize) & (FT_CMD_FIFO_SIZE - 1);
}

// --------- Increment co-processor address offset counter --------------------
uint16_t HAL_GetCmdPointer(void)
{
	// Return new offset
	return writeCmdPointer;
}

void HAL_WriteCmdPointer(void)
{
	// and move write pointer to here
	HAL_MemWrite32(REG_CMD_WRITE, writeCmdPointer);
}

// ------ Wait for co-processor read and write pointers to be equal ------------
uint8_t HAL_WaitCmdFifoEmpty(void)
{
	uint32_t readCmdPointer;

	// Wait until the two registers match
	do
	{
		// Read the graphics processor read pointer
		readCmdPointer = HAL_MemRead32(REG_CMD_READ);

	} while ((writeCmdPointer != readCmdPointer) && (readCmdPointer != 0xFFF));


	if(readCmdPointer == 0xFFF)
	{
		// Return 0xFF if an error occurred
		return 0xFF;
	}
	else
	{
		// Return 0 if pointers became equal successfully
		return 0;
	}
}
// ------------ Check how much free space is available in CMD FIFO -------------
uint16_t HAL_CheckCmdFreeSpace(void)
{
	uint32_t readCmdPointer = 0;
	uint16_t Fullness, Freespace;

	// Check the graphics processor read pointer
	readCmdPointer = HAL_MemRead32(REG_CMD_READ);

	// Fullness is difference between MCUs current write pointer value and the FT81x's REG_CMD_READ
	Fullness = ((writeCmdPointer - (uint16_t)readCmdPointer) & (FT_CMD_FIFO_SIZE - 1));
	// Free Space is 4K - 4 - Fullness (-4 avoids buffer wrapping round)
	Freespace = (FT_CMD_FIFO_SIZE - 4) - Fullness;

	return Freespace;
}
