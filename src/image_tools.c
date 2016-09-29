/*******************************************************************************
 * (c) Copyright 2015 Microsemi SoC Products Group.  All rights reserved.
 *
 * SmartFusion2 Bootloader Application iimage support routines.
 *
 * SVN $Revision: 8447 $
 * SVN $Date: 2016-06-26 15:07:36 +0100 (Sun, 26 Jun 2016) $
 */

#include <stdint.h>
#include <string.h>

#include "sf2_bl_defs.h"
#include "sf2_bl_options.h"
#include "spi_flash.h"
#include "image_tools.h"
#include "crc32.h"
#include "ymodem.h"

/*
 * Length of the buffer we allocate for intermediate buffering of data for FLASH
 * transfers. Not necessarily a FLASH sector sized buffer.
 */
#define SECTOR_BUF_LEN 4096

uint8_t g_sector_buffer[SECTOR_BUF_LEN];

/*
 * Holding places in RAM for working on image headers.
 */
img_hdr_block_t    g_img1_header;
uint32_t           g_img1_offset;
sf2bl_hdr_status_t g_img1_status;

uint16_t           g_current_sequence;

#if defined(SF2BL_2ND_IMAGE)
img_hdr_block_t    g_img2_header;
uint32_t           g_img2_offset;
sf2bl_hdr_status_t g_img2_status;
#endif

#if defined(SF2BL_GOLDEN)
img_hdr_block_t    g_golden_img_header;
uint32_t           g_golden_img_offset;
sf2bl_hdr_status_t g_golden_img_status;
#endif


/***************************************************************************//**
 * Support routines for reading, writing and verifying application images in the
 * SPI FLASH device.
 *
 * The following options are supported:
 *
 * 1. Single FLASH image with no redundancy or fall back. Flash write failures
 *    will result in automatic boot into update mode (this is true of all the
 *    other options as well if there are no valid images detected).
 * 2. Golden FLASH image plus single FLASH image. in this case the first image
 *    written to a blank FLASH device is not over writable and is the GOLDEN
 *    image if the other image(s) are missing or corrupted. This provides a
 *    measure of protection in that there is a valid image that can be used if
 *    the firmware upgrade operation fails.
 * 3. Dual FLASH images used in a ping pong fashion. This allows fall back to
 *    the previous image (if there is one) if the firmware upgrade operation
 *    fails.
 * 4. Golden FLASH image plus dual FLASH image.
 */

/***************************************************************************//**
 * Write the current image file in DDR to the SPI FLASH device.
 *
 * Image is located at g_bin_base, is written starting at address and is size
 * bytes long.
 *
 * Returns 0 for success or -1 for error. (Currently no error checking but once
 * SPI FLASH driver is modified to support timeouts I'll update this function.
 *
 */

int32_t sf2bl_wr_flash_image(uint32_t address, uint32_t size)

{
    uint32_t index;
    uint32_t temp;
    uint32_t block_count;
    spi_flash_status_t flash_result = SPI_FLASH_SUCCESS;

    if(0 != size)
    {
        /* Calculate number of blocks required by rounding up. */
        block_count = (size + (SF2BL_FLASH_IMAGE_GRANUALARITY-1)) / SF2BL_FLASH_IMAGE_GRANUALARITY;

        /* Erase each 4K block */
        for(index = 0; (SPI_FLASH_SUCCESS == flash_result) && (index < block_count); index++)
        {
#if (SF2BL_FLASH_DEVICE == SF2BL_FLASH_DEV_AT25DF641)
            flash_result = spi_flash_control_hw(SPI_FLASH_SECTOR_UNPROTECT, address + (index * SF2BL_FLASH_IMAGE_GRANUALARITY), 0);
            if(SPI_FLASH_SUCCESS == flash_result)
            {
                flash_result = spi_flash_control_hw(SPI_FLASH_4KBLOCK_ERASE, address + (index * SF2BL_FLASH_IMAGE_GRANUALARITY), 0);
            }
#else
            flash_result = spi_flash_control_hw(SPI_FLASH_4KBLOCK_ERASE, address + (index * SF2BL_FLASH_IMAGE_GRANUALARITY), 0);
#endif
        }

        /* Recalculate number of blocks required in case sector buffer is not same size as FLASH erase block. */
        block_count = (size + (SECTOR_BUF_LEN-1)) / SECTOR_BUF_LEN;
        for(index = 0; (SPI_FLASH_SUCCESS == flash_result) && (index < block_count); index++)
        {
            /*
             * We use an intermediate buffer in eSRAM as the PDMA does not work
             * properly with DDR and the HPDMA does not provide the ping pong
             * buffer functionality that the SPI FLASH driver uses.
             */
            memcpy(g_sector_buffer, &g_bin_base[index * SECTOR_BUF_LEN], SECTOR_BUF_LEN);
            if(index != block_count - 1) /* Last block handled separately... */
            {
                flash_result = spi_flash_write(address +(index * SECTOR_BUF_LEN), g_sector_buffer, SECTOR_BUF_LEN, 0);
            }
            else
            {
                /*
                 * The last block may be less than a full buffer in size so we
                 * will only write what is required. It does not really make a
                 * lot of difference as we are allocating a full sector in the
                 * FLASH for it but this is more consistent.
                 */
                temp = size % SECTOR_BUF_LEN;
                if(0 == temp) /* Ended exactly on a buffer length boundary! */
                {
                    temp = SECTOR_BUF_LEN;
                }
                flash_result = spi_flash_write(address + (index * SECTOR_BUF_LEN), g_sector_buffer, temp, 0);
                if(SPI_FLASH_SUCCESS == flash_result)
                {
                    /*
                     * Mark image as valid. We already calculated the header CRC
                     * based on a valid block so we only have to update the
                     * header to make it so.
                     */
                    temp = SF2BL_IMG_HDR_VALID;
                    flash_result = spi_flash_write(address, (uint8_t *)&temp, sizeof(temp), 0);
                }
            }
        }
    }

    return(SPI_FLASH_SUCCESS == flash_result ? 0 : -1);
}


/***************************************************************************//**
 * load image from SPI FLASH into DDR at the correct locations. This function
 * implicitly assumes that DDR mapped to 0x00000000 is involved as the target
 * for code sections as it writes any data in the 0x00000000-0x1FFFFFFF range to
 * the 0xA0000000-0xBFFFFFFF range as we are assumed to be running in eNVM which
 * is mapped to the 0x00000000 range.
 *
 * g_img1_header is loaded with the image header.
 *
 * Returns 0 for success or -1 for error.
 */

int32_t sf2bl_rd_flash_image(uint32_t address)
{
    spi_flash_status_t flash_result;
    uint32_t block_count;
    uint32_t index;
    uint32_t img_src_offset;
    uint8_t *img_dst_offset;
    uint32_t temp;
    uint32_t crc;

    flash_result = spi_flash_read(address, (uint8_t *)&g_img1_header, sizeof(g_img1_header));
    img_src_offset = address + sizeof(g_img1_header);
    crc = 0xFFFFFFFF;

    for(index = 0; (SPI_FLASH_SUCCESS == flash_result) && (index < g_img1_header.n_chunks) ; index++)
    {
        img_chunk_hdr_t chunk_hdr;
        flash_result = spi_flash_read(img_src_offset, (uint8_t *)&chunk_hdr, sizeof(chunk_hdr));
        if(SPI_FLASH_SUCCESS == flash_result)
        {
            crc = sf2bl_calc_crc32(crc, (uint8_t *)&chunk_hdr, sizeof(chunk_hdr));
            img_src_offset += sizeof(chunk_hdr);
            img_dst_offset = (uint8_t *)(chunk_hdr.base + chunk_hdr.offset);
            if(img_dst_offset < (uint8_t *)0x20000000)
            {
                /* data for the code space should be written through the DDR primary
                 * addresses and not the cached code addresses as we (in eNVM) are
                 * currently mapped into the cached code ares.
                 */
                img_dst_offset += 0xA0000000;
            }

            block_count = (chunk_hdr.len + 4095) / 4096;
             /* Deal with guaranteed full blocks first */
            while((block_count > 1) && (SPI_FLASH_SUCCESS == flash_result))
            {
                flash_result = spi_flash_read(img_src_offset, g_sector_buffer, SECTOR_BUF_LEN);
                crc = sf2bl_calc_crc32(crc, g_sector_buffer, SECTOR_BUF_LEN);
                memcpy(img_dst_offset, g_sector_buffer, SECTOR_BUF_LEN);
                img_src_offset += SECTOR_BUF_LEN;
                img_dst_offset += SECTOR_BUF_LEN;
                block_count--;
            }

            if(SPI_FLASH_SUCCESS == flash_result)
            {
                /* Check for last block being exactly one sector long */
                temp = chunk_hdr.len % SECTOR_BUF_LEN;
                if(0 == temp)
                {
                    temp = SECTOR_BUF_LEN;
                }

                flash_result = spi_flash_read(img_src_offset, (uint8_t *)&g_sector_buffer, temp);
                crc = sf2bl_calc_crc32(crc, g_sector_buffer, temp);
                memcpy(img_dst_offset, g_sector_buffer, temp);
                img_src_offset += temp;
            }
        }
    }

    /* If there was a SPI FLASH error, force invalid CRC result */
    if(SPI_FLASH_SUCCESS != flash_result)
    {
        crc = ~g_img1_header.crc32;
    }

    return(crc == g_img1_header.crc32 ? 0 : -1);
}


/***************************************************************************//**
 * load header and image from SPI FLASH into DDR at binary file build location.
 * This is used when copying the golden image to perform a reset of the flash
 * images.
 *
 * Returns length of image or -1 for error.
 */

int32_t sf2bl_raw_rd_flash_image(uint32_t address)
{
    spi_flash_status_t flash_result;
    uint32_t block_count;
    uint32_t index;
    uint32_t img_src_offset;
    uint8_t *img_dst_offset;
    uint32_t temp;

    /* Fetch copy of image header */
    flash_result = spi_flash_read(address, (uint8_t *)&g_img1_header, sizeof(g_img1_header));
    if(SF2BL_HDR_OK == sf2bl_check_img_header(&g_img1_header))
    {
        /* Calculate number of blocks to read */
        block_count = (g_img1_header.size + (SECTOR_BUF_LEN-1)) / SECTOR_BUF_LEN;

        img_src_offset = address;
        img_dst_offset = g_bin_base;
        for(index = 0; (SPI_FLASH_SUCCESS == flash_result) && (index < block_count); index++)
        {
            /*
             * We use an intermediate buffer in eSRAM as the PDMA does not work
             * properly with DDR and the HPDMA does not provide the ping pong
             * buffer functionality that the SPI FLASH driver uses.
             */

            if(index != block_count - 1) /* Last block handled separately... */
            {
                flash_result = spi_flash_read(img_src_offset, g_sector_buffer, SECTOR_BUF_LEN);
                memcpy(img_dst_offset, g_sector_buffer, SECTOR_BUF_LEN);
                img_src_offset += SECTOR_BUF_LEN;
                img_dst_offset += SECTOR_BUF_LEN;
            }
            else
            {
                /*
                 * The last block may be less than a full buffer in size so we
                 * will only read what is required. It does not really make a
                 * lot of difference as we have allocated a full sector in the
                 * FLASH for it but this is more consistent.
                 */
                temp = g_img1_header.size % SECTOR_BUF_LEN;
                if(0 == temp) /* Ended exactly on a buffer length boundary! */
                {
                    temp = SECTOR_BUF_LEN;
                }
                flash_result = spi_flash_read(img_src_offset, g_sector_buffer, temp);
                memcpy(img_dst_offset, g_sector_buffer, SECTOR_BUF_LEN);
            }
        }
    }
    else
    {
        /* set up for error response */
        flash_result = SPI_FLASH_UNSUCCESS;
    }

    return((SPI_FLASH_SUCCESS == flash_result) ? g_img1_header.size : -1);
}


/***************************************************************************//**
 * Check an image header and return it's status based on the result of the
 * checks.
 *
 *   SF2BL_HDR_BLANK
 *   SF2BL_HDR_INVALID
 *   SF2BL_HDR_BAD_CRC
 *   SF2BL_HDR_BAD_DATA
 *   SF2BL_HDR_OK
 */

sf2bl_hdr_status_t sf2bl_check_img_header(img_hdr_block_t *this_block)
{
    sf2bl_hdr_status_t return_val;

    if(SF2BL_IMG_HDR_BLANK == this_block->valid)
    {
        /*
         * Either blank or write operation did not complete and so the image is
         * free for overwriting.
         */
        return_val = SF2BL_HDR_BLANK;
    }
    else if(SF2BL_IMG_HDR_INVALID == this_block->valid)
    {
        /*
         * Image was probably in use previously but has been superseded by a
         * more recent image.
         */
        return_val = SF2BL_HDR_INVALID;
    }
    else if(SF2BL_IMG_HDR_VALID == this_block->valid)
    {
        /* looks good so far but just in case... */
        if(this_block->crc16 != sf2bl_crc16((uint8_t *)this_block, sizeof(img_hdr_block_t) - 2))
        {
            return_val = SF2BL_HDR_BAD_CRC;
        }
        else
        {
            return_val = SF2BL_HDR_OK;
        }
    }
    else
    {
        /* Unexpected value so just say its bad */
        return_val = SF2BL_HDR_BAD_DATA;
    }

    return(return_val);
}

/***************************************************************************//**
 * Examine the FLASH device and based on the bootloader configuration, determine
 * the following:
 *
 *   1. Is there a valid golden image and if so what offset is it at in the
 *      FLASH?
 *   2. Is there a valid normal image and if so what offset is it at in the
 *      FLASH?
 *   3. Is there a valid second image and if so what offset is it at in the
 *      FLASH?
 *
 */

void sf2bl_check_flash(void)
{
    spi_flash_status_t flash_result;

#if defined(SF2BL_GOLDEN)
    /*
     * Golden image is always the first in the FLASH and is placed straight
     * after the log and configuration storage areas.
     */
    g_golden_img_offset = SF2BL_FLASH_BASE + (SF2BL_LOG_SIZE * 4096) + (SF2BL_CONFIG_SIZE * 4096);
    flash_result = spi_flash_read(g_golden_img_offset, (uint8_t *)&g_golden_img_header, sizeof(g_golden_img_header));
    if(SPI_FLASH_SUCCESS == flash_result)
    {
        g_golden_img_status =  sf2bl_check_img_header(&g_golden_img_header);
    }
    else
    {
        g_golden_img_status = SF2BL_HDR_READ_FAIL;
    }

    g_img1_offset = g_golden_img_offset + SF2BL_IMAGE_SIZE;
#else
    /*
     * No golden image so first image is placed straight after the log and
     * configuration storage areas.
     */
    g_img1_offset = SF2BL_FLASH_BASE + (SF2BL_LOG_SIZE * 4096) + (SF2BL_CONFIG_SIZE * 4096);
#endif

    flash_result = spi_flash_read(g_img1_offset, (uint8_t *)&g_img1_header, sizeof(g_img1_header));
    if(SPI_FLASH_SUCCESS == flash_result)
    {
        g_img1_status =  sf2bl_check_img_header(&g_img1_header);
    }
    else
    {
        g_img1_status = SF2BL_HDR_READ_FAIL;
    }

#if defined(SF2BL_2ND_IMAGE)
    g_img2_offset = g_img1_offset + SF2BL_IMAGE_SIZE;
    flash_result = spi_flash_read(g_img2_offset, (uint8_t *)&g_img2_header, sizeof(g_img2_header));
    if(SPI_FLASH_SUCCESS == flash_result)
    {
        g_img2_status =  sf2bl_check_img_header(&g_img2_header);
    }
    else
    {
        g_img2_status = SF2BL_HDR_READ_FAIL;
    }
#endif
}
