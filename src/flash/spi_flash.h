 /******************************************************************************
 *
 * Copyright (C) 2012 MICROSEMI Corporation.
 *
 * Company: Microsemi Corporation
 *
 * File: spi_flash.h
 * File history:
 *      Revision: 1.0 Date: May 4, 2010
 *      Revision: 1.1 Date: September 26, 2012
 *
 * Note:
 *
 * Revision 1.1 is a version for MPM 5.0 which addresses issues with interrupt
 * latency on A2F200 based systems and allows for auto selection of the SPI port
 * to use on DEV Kit and EVAL Kit boards.
 *
 * Peter Mc Shane 26/09/2012
 *
 * Description:
 *
 * Device driver for the on-board SPI flash for SmartFusion KITS Atmel AT25DF641
 * SPI flash driver API.
 *
 * Author: Upender Cherukupally
 *         Upender.Cherukupally@microsemi.com
 *         Corporate Applications Engineering
 *
 * SVN $Revision: 7338 $
 * SVN $Date: 2015-04-21 12:45:52 +0100 (Tue, 21 Apr 2015) $
 *
 *******************************************************************************/

#ifndef __SPI_FLASH_H_
#define __SPI_FLASH_H_

#include <stdint.h>
#include <stdlib.h>

/*
 * Configuration for the SPI Flash
 *
 */



/*******************************************************************************
 * Possible return values from functions on SPI FLASH.
 ******************************************************************************/
typedef enum {
    SPI_FLASH_SUCCESS = 0,
    SPI_FLASH_PROTECTION_ERROR,
    SPI_FLASH_WRITE_ERROR,
    SPI_FLASH_INVALID_ARGUMENTS,
    SPI_FLASH_INVALID_ADDRESS,
    SPI_FLASH_TIMEOUT,
    SPI_FLASH_VERIFY_FAIL,
    SPI_FLASH_UNSUCCESS
} spi_flash_status_t;

/*******************************************************************************
 * Possible HW Control commands on SPI FLASH.
 ******************************************************************************/
typedef enum {
    SPI_FLASH_SECTOR_UNPROTECT = 0,
    SPI_FLASH_SECTOR_PROTECT,
    SPI_FLASH_GLOBAL_UNPROTECT,
    SPI_FLASH_GLOBAL_PROTECT,
    SPI_FLASH_GET_STATUS,
    SPI_FLASH_4KBLOCK_ERASE,
    SPI_FLASH_32KBLOCK_ERASE,
    SPI_FLASH_64KBLOCK_ERASE,
    SPI_FLASH_CHIP_ERASE,
    SPI_FLASH_READ_DEVICE_ID,
    SPI_FLASH_READ_NV_CFG,
    SPI_FLASH_WRITE_NV_CFG,
    SPI_FLASH_READ_V_CFG,
    SPI_FLASH_WRITE_V_CFG,
    SPI_FLASH_RESET
/*
    SPI_FLASH_SECTOR_LOCKDOWN,
    SPI_FLASH_FREEZE_SECTOR_LOCKDOWN
*/
} spi_flash_control_hw_t;

typedef struct spi_dev_Info{
    uint8_t manufacturer_id;
    uint8_t device_id;
} spi_dev_info_t;

/*******************************************************************************
 * This function initialises the SPI peripheral and PDMA for data transfer
 ******************************************************************************/
spi_flash_status_t
spi_flash_init
(
    void
);


/*******************************************************************************
 * This function shuts down the SPI peripheral and PDMA
 ******************************************************************************/
void spi_flash_deinit
(
    void
);


/******************************************************************************
 * This function performs the various operations on the serial Flash
 * based on the command passed as first parameter.
 * The operation of the each command is explained below.
 *
 * @param operation
 *        The operations supported are as per the enum spi_flash_control_hw_t
 *        defined above. The functionality is as follows:
 *
 *        1. SPI_FLASH_SECTOR_UNPROTECT: Every 64KBytes are represented
 *           in sectors. There is a corresponding bits set for protection
 *           of that sector. To do modify operations like write and erase
 *           we need to call this operation to unprotect the block.
 *           The second parameter 'peram1' for this function is the block
 *           address to unprotect.
 *
 *        2. SPI_FLASH_SECTOR_PROTECT :  Every 64KBytes are represented
 *           in sectors. There is a corresponding bits set for protection
 *           of that sector. To protect from the modify operations like
 *           write and erase we need to call this operation
 *           to protect the block. The second parameter 'peram1' for this
 *           function is the block address to protect.
 *
 *        3. SPI_FLASH_GLOBAL_UNPROTECT: This command is used to unprotect
 *           the entire flash for modify operations.
 *
 *        4. SPI_FLASH_GLOBAL_PROTECT: This command is used to protect/lock
 *           the entire flash from modify operations.
 *
 *        5. SPI_FLASH_GET_STATUS: This function used to get the SPI Flash
 *           status register content - for more details of the status bits
 *           refer the page36 of flash data sheet AT25DF641.
 *           The status is written to the uint8_t pointed to by ptrPeram;
 *
 *        6. SPI_FLASH_4KBLOCK_ERASE: This command is used to erase the block
 *           starting at 4KB boundary. The starting address of the 4K Block is
 *           passed in the second parameter peram1 of this API.
 *
 *        7. SPI_FLASH_32KBLOCK_ERASE: This command is used to erase the block
 *           starting at 32KB boundary. The starting address of the 32K Block
 *           is passed in the second parameter peram1 of this API.
 *
 *        8. SPI_FLASH_64KBLOCK_ERASE: This command is used to erase the block
 *           starting at 64KB boundary. The starting address of the 64K Block
 *           is passed in the second parameter peram1 of this API.
 *
 *        9. SPI_FLASH_CHIP_ERASE This command is used to erase the entire flash
 *           chip.
 *
 *       10. SPI_FLASH_READ_DEVICE_ID: This command is used to read the
 *           device properties. The values are filled in the third parameter
 *           'ptrPeram' of this API which should point to an spi_dev_info_t
 *           structure.
 *
 *       11. SPI_FLASH_RESET: In some cases it may be necessary to prematurely
 *           terminate a program or erase cycle early rather than wait the
 *           hundreds of microseconds or milliseconds necessary for the program
 *           or erase operation to complete normally.
 *           The Reset command allows a program or erase operation in progress
 *           to be ended abruptly and returns the device to an idle state.
 *
 * @param peram1        The peram1 usage is explained in the above description
 *                      according to the command in use.
 * @param ptrPeram      The ptrPeram usage is explained in the above description
  *                     according to command in use.
 * @return              The return value indicates if the write was successful.
 *                      Possible values are:
 *                      SPI_FLASH_SUCCESS,
 *                      SPI_FLASH_PROTECTION_ERROR,
 *                      SPI_FLASH_INVALID_ARGUMENTS,
 *                      SPI_FLASH_INVALID_ADDRESS,
 *						SPI_FLASH_TIMEOUT,
 *                      SPI_FLASH_UNSUCCESS
 *
 *                      SPI_FLASH_SUCCESS: describes the SPI Flash operation is
 *                      correct and complete
 *
 *                      SPI_FLASH_PROTECTION_ERROR: The sector is protected and
 *                      not allowing the operation.
 *                      We need to do the unprotect and do the operation
 *
 *                      SPI_FLASH_INVALID_ARGUMENTS: describes that function has
 *                      received Invalid arguments
 *
 *                      SPI_FLASH_INVALID_ADDRESS: describes that function has
 *                      received Invalid address
 *
 *                      SPI_FLASH_UNSUCCESS: describes the SPI Flash operation
 *                      is incomplete
 *
 *                      SPI_FLASH_TIMEOUT: The device busy flag did not clear
 *                      within the expected time frame.
 */

spi_flash_status_t
spi_flash_control_hw
(
    spi_flash_control_hw_t operation,
    uint32_t peram1,
    void *   ptrPeram
);

/*******************************************************************************
 * This function reads the content from the serial Flash.
 * The data is read from the memory location specified by the first parameter.
 * This address is ranges from 0 to SPI Flash Size. This address range is not
 * the processors absolute range.
 *
 * @param start_addr    This is the address at which data will be read.
 *                      This address is ranges from 0 to SPI Flash Size.
 *                      This address range is not the processors absolute range.
 * @param p_data        This is a pointer to the buffer for holding the read data.
 * @param nb_bytes      This is the number of bytes to be read from SPI Flash.
 * @return              The return value indicates if the write was successful.
 *                      Possible values are:
 *                      SPI_FLASH_SUCCESS,
 *                      SPI_FLASH_PROTECTION_ERROR,
 *                      SPI_FLASH_INVALID_ARGUMENTS,
 *                      SPI_FLASH_INVALID_ADDRESS,
 *                      SPI_FLASH_UNSUCCESS
 *
 *                      SPI_FLASH_SUCCESS: Describes the SPI Flash operation is
 *                      correct and complete
 *
 *                      SPI_FLASH_INVALID_ARGUMENTS: Describes that function has received
 *                      Invalid arguments
 *
 *                      SPI_FLASH_INVALID_ADDRESS: Describes that function has received
 *                      Invalid address
 *
 *                      SPI_FLASH_UNSUCCESS: Describes the SPI Flash operation is
 *                      incomplete
 */
spi_flash_status_t
spi_flash_read
(
    uint32_t address,
    uint8_t * rx_buffer,
    size_t size_in_bytes
);

/*******************************************************************************
 * This function writes the content of the buffer passed as parameter to
 * Serial Flash through SPI. The data is written from the memory location specified
 * by the first parameter.
 * This address is ranges from 0 to SPI Flash Size. This address range is not
 * the processors absolute range
 *
 * @param start_addr    This is the address at which data will be written.
 *                      This address is ranges from 0 to SPI Flash Size.
 *                      This address range is not the processors absolute range
 * @param p_data        This is a pointer to the buffer holding the data to be
 *                      written into Serial Flash.
 * @param nb_bytes      This is the number of bytes to be written into Serial Flash.
 * @return              The return value indicates if the write was successful.
 *                      Possible values are:
 *                      SPI_FLASH_SUCCESS,
 *                      SPI_FLASH_PROTECTION_ERROR,
 *                      SPI_FLASH_WRITE_ERROR,
 *                      SPI_FLASH_INVALID_ARGUMENTS,
 *                      SPI_FLASH_INVALID_ADDRESS,
 *                      SPI_FLASH_UNSUCCESS
 *
 *                      SPI_FLASH_SUCCESS: describes the SPI Flash operation is
 *                      correct and complete
 *
 *                      SPI_FLASH_PROTECTION_ERROR: The sector is under protected and
 *                      not allowing the operation.
 *                      We need to do the unprotect and do the operation
 *
 *                      SPI_FLASH_WRITE_ERROR: describes the SPI Flash write operation is
 *                      failed
 *
 *                      SPI_FLASH_INVALID_ARGUMENTS: describes that function has received
 *                      Invalid arguments
 *
 *                      SPI_FLASH_INVALID_ADDRESS: describes that function has received
 *                      Invalid address. Address range should be between 0 to 8 MB
 *
 *                      SPI_FLASH_UNSUCCESS: describes the SPI Flash operation is
 *                      incomplete
 */

spi_flash_status_t
spi_flash_write
(
    uint32_t address,
    uint8_t * write_buffer,
    size_t size_in_bytes,
    uint32_t format
);

#endif
