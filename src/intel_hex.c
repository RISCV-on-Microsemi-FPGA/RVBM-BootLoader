/*******************************************************************************
 * (c) Copyright 2015 Microsemi SoC Products Group.  All rights reserved.
 *
 * SmartFusion2 Bootloader Intel Hex file support routines.
 *
 * SVN $Revision: 8447 $
 * SVN $Date: 2016-06-26 15:07:36 +0100 (Sun, 26 Jun 2016) $
 */

#include <stdint.h>
#include <ctype.h>
#include <string.h>

#include "intel_hex.h"
#include "ymodem.h"
#include "crc32.h"
#include "sf2_bl_defs.h"
#include "image_tools.h"

/*
 * The following macro controls display of debug information on the serial port
 * to help see what records have been processed int the hex file.
 *
 * #define HEX_DEBUG(x) _putchar(x);
 */

#define HEX_DEBUG(x)

hex_rec_t g_hex_record;
uint8_t   g_hex_checksum;

/***************************************************************************//**
 * return positive value 0-255 if ok or negative value if error.
 */

int32_t sf2bl_get_hex_byte(uint8_t *data, int32_t remaining)
{
    int32_t return_val = 0;
    int32_t index;

    if(remaining > 1)
    {
        for(index = 0; index < 2; index++, data++)
        {
            return_val <<= 4;
            if(isdigit(*data))
            {
                return_val |= *data - '0';
            }
            else if(isxdigit(*data))
            {
                return_val |= (tolower(*data) - 'a') + 10;
            }
            else /* Not expecting this... */
            {
                return_val = -1; /* Force -ve response */
            }
        }
    }
    else
    {
        return_val = -1;
    }

    g_hex_checksum += (uint8_t)return_val;
    return(return_val);
}

/***************************************************************************//**
 * return positive value 0-65535 if ok or negative value if error.
 */

int32_t sf2bl_get_hex_word(uint8_t *data, int32_t remaining)
{
    int32_t temp1;
    int32_t temp2;

    temp1 = sf2bl_get_hex_byte(data, remaining);
    if(temp1 >= 0)
    {
        data      += 2;
        remaining -= 2;
        temp2 = sf2bl_get_hex_byte(data, remaining);
        if(temp2 >= 0)
        {
            temp1 = (temp1 << 8) | temp2;
        }
    }

    return(temp1);
}

/***************************************************************************//**
 * Decodes an intel hex record at data into the g_hex_record structure.
 * Returns the amount of data consumed or < 0 for error.
 */
int32_t sf2bl_hex_record(uint8_t *data, int32_t remaining)
{
    int32_t return_val = -1;
    int32_t consumed = 0;
    int32_t index;
    int32_t temp1;
    int32_t temp2;
    uint8_t temp_checksum;

    if(':' == *data) /* Looks like an Intel Hex record so far... */
    {
        g_hex_checksum = 0; /* Start out with checksum reset */
        data++;
        remaining--;

        g_hex_record.count = sf2bl_get_hex_byte(data, remaining);
        data      += 2;
        remaining -= 2;

        if(g_hex_record.count >= 0)
        {
            g_hex_record.offset = sf2bl_get_hex_word(data, remaining);
            data      += 4;
            remaining -= 4;

            if(g_hex_record.offset >= 0)
            {
                g_hex_record.type = sf2bl_get_hex_byte(data, remaining);
                data      += 2;
                consumed   = 9; /* Fixed overhead to this point */
                remaining -= 2;

                if((g_hex_record.type >= 0) && (remaining >= (g_hex_record.count * 2 + 2)))
                {
                    switch(g_hex_record.type)
                    {
                    case IHEX_DATA:
                        temp1 = 0;
                        for(index = 0; (index < g_hex_record.count) && (temp1 >= 0); index++)
                        {
                            temp1 = sf2bl_get_hex_byte(data, remaining);
                            g_hex_record.data[index] = (uint8_t)temp1;
                            data      += 2;
                            consumed  += 2;
                            remaining -= 2;
                        }

                        if(temp1 >= 0)
                        {
                            return_val = 0; /* looking good so far */
                        }
                        break;

                    case IHEX_EOF:
                        return_val = 0; /* looking good so far */
                        break;

                    case IHEX_EXSEG_ADDR:
                    case IHEX_EXLIN_ADDR:
                        g_hex_record.address = sf2bl_get_hex_word(data, remaining);
                        data      += 4;
                        consumed  += 4;
                        remaining -= 4;
                        if(g_hex_record.address >= 0)
                        {
                            return_val = 0; /* looking good so far */
                        }
                        break;

                    case IHEX_START_SEG:
                    case IHEX_START_ADDR:
                        temp1 = sf2bl_get_hex_word(data, remaining);
                        data      += 4;
                        consumed  += 4;
                        remaining -= 4;
                        if(temp1 >= 0)
                        {
                            temp2 = sf2bl_get_hex_word(data, remaining);
                            data      += 4;
                            consumed  += 4;
                            remaining -= 4;
                            if(temp2 >= 0)
                            {
                                temp1 = (temp1 << 16) | temp2;
                                return_val = 0; /* looking good so far */
                            }
                        }
                        break;

                    default:
                        break;
                    }
                }

                if(0 == return_val) /* Last thing is the checksum if all good so far */
                {
                    temp_checksum = ~g_hex_checksum; /* Ones complement */
                    temp_checksum++;                 /* Now twos complement */

                    temp1 = sf2bl_get_hex_byte(data, remaining);
                    consumed  += 2;
                    remaining -= 2;
                    if((temp1 >= 0) && (temp1 == temp_checksum))
                    {
                        return_val = consumed;
                    }
                    else /* Oops, fell at the last hurdle */
                    {
                        return_val = -1;
                    }
                }
            }
        }
    }

    return(return_val);
}

/***************************************************************************//**
 *
 */

uint32_t sf2bl_process_hex_file(int32_t received)
{
    uint8_t *data;
    uint8_t *dest;
    int32_t remaining;
    int32_t temp;
    int32_t done;
    img_hdr_block_t *pheader;
    img_chunk_hdr_t *pchunk_hdr;
    img_chunk_hdr_t *pchunk_last;
    uint32_t expected_offset;
    uint32_t n_chunks;
    int32_t return_val;


    return_val = 0;

    data = g_rx_base;
    pheader    = (img_hdr_block_t *)g_bin_base;
    pchunk_hdr = (img_chunk_hdr_t *)(g_bin_base + sizeof(img_hdr_block_t));
    dest = g_bin_base + sizeof(img_hdr_block_t) + sizeof(img_chunk_hdr_t);

    remaining = received;
    n_chunks = 1;
    done = 0;

    memset((uint8_t *)pheader, 0, sizeof(img_hdr_block_t));
    pheader->valid = SF2BL_IMG_HDR_BLANK;
    pheader->version = 0x00000001;
    pheader->crc16 = 0xFFFF;
    pheader->sequence = 0; /* Will over write if not golden image downloaded */

    pchunk_hdr->base   = 0;
    pchunk_hdr->offset = 0;
    pchunk_hdr->len    = 0;
    expected_offset    = 0;

    while(!done)
    {
        temp = sf2bl_hex_record(data, remaining);
        if(temp >= 0)
        {
            data += temp;
            remaining -= temp;

            switch(g_hex_record.type)
            {
            case IHEX_DATA:
                /*
                 * We track the next expected offset so that non contiguous
                 * records trigger the generation of a new chunk. The Intel Hex
                 * format does not guarantee that there are not gaps in the data
                 * but our chunks have to contain contiguous data.
                 */
                if(g_hex_record.offset != expected_offset) /* Start of new chunk? */
                {
                    if(0 == pchunk_hdr->len) /* Current chunk empty? */
                    {
                        pchunk_hdr->offset = g_hex_record.offset; /* Just update offset */
                        expected_offset = g_hex_record.offset;
                    }
                    else /* need new chunk based on old one */
                    {
                        pchunk_last        = pchunk_hdr;
                        pchunk_hdr         = (img_chunk_hdr_t *)dest;
                        dest              += sizeof(img_chunk_hdr_t);
                        pchunk_hdr->base   = pchunk_last->base;
                        pchunk_hdr->offset = g_hex_record.offset;
                        pchunk_hdr->len    = 0;
                        pchunk_hdr->index  = n_chunks;
                        n_chunks++;
                        expected_offset    = g_hex_record.offset;
                    }
                }

                memcpy(dest, g_hex_record.data, g_hex_record.count);

                dest             += g_hex_record.count;
                pchunk_hdr->len  += g_hex_record.count;
                expected_offset  += g_hex_record.count;

                HEX_DEBUG('0')
                break;

            case IHEX_EOF:
                if((pchunk_hdr == (img_chunk_hdr_t *)(g_bin_base + sizeof(img_hdr_block_t))) &&
                   (0u == pchunk_hdr->len))
                {
                    /* Empty hex file, with no data */
                    pheader->valid = SF2BL_IMG_HDR_INVALID;
                    pheader->crc16 = sf2bl_crc16((uint8_t *)pheader, sizeof(img_hdr_block_t) - 2u);
                }
                else
                {
                    /* Set the valid flag temporarily so CRC is correct */
                    pheader->valid = SF2BL_IMG_HDR_VALID;
                    /* Update sequence number if not golden image */
                    if(SF2BL_BOOT_DOWNLOAD_GOLDEN != g_boot_mode)
                    {
                        pheader->sequence = g_current_sequence + 1u;
                    }
                    pheader->n_chunks = n_chunks;
                    pheader->size = (uint32_t)(dest - (uint8_t *)pheader);
                    pheader->crc32 = sf2bl_calc_crc32(0xFFFFFFFF, (uint8_t *)(g_bin_base + sizeof(img_hdr_block_t)), pheader->size - sizeof(img_hdr_block_t));
                    pheader->crc16 = sf2bl_crc16((uint8_t *)pheader, sizeof(img_hdr_block_t) - 2u);
                    /* Reset valid flag until we have finished writing to FLASH */
                    pheader->valid = SF2BL_IMG_HDR_BLANK;
                    return_val = pheader->size + sizeof(img_hdr_block_t);
                }
                done = 1;
                HEX_DEBUG('1')
                break;

            case IHEX_EXSEG_ADDR:
            case IHEX_EXLIN_ADDR:
                if(0 != pchunk_hdr->len) /* Current chunk empty? */
                {
                    pchunk_hdr         = (img_chunk_hdr_t *)dest;
                    dest              += sizeof(img_chunk_hdr_t);
                    pchunk_hdr->len    = 0;
                    pchunk_hdr->index  = n_chunks;
                    n_chunks++;
                }

                if(IHEX_EXSEG_ADDR == g_hex_record.type)
                {
                    pchunk_hdr->base   = g_hex_record.address * 16u;
                    HEX_DEBUG('2')
                }
                else
                {
                    pchunk_hdr->base   = g_hex_record.address << 16;
                    HEX_DEBUG('4')
                }

                pchunk_hdr->offset = 0u;
                expected_offset    = 0u;

                break;

            case IHEX_START_SEG:
                HEX_DEBUG('3')
                break;

            case IHEX_START_ADDR:
                HEX_DEBUG('5')
                break;

            default:
                HEX_DEBUG('!')
                HEX_DEBUG('!')
                HEX_DEBUG('!')
                done = 1;
                break;
            }

            if(!done)
            {
                data += 2;
                remaining -= 2;
                if(remaining <= 0)
                {
                    HEX_DEBUG('-')
                    done = 1;
                }
            }
        }
        else
        {
            HEX_DEBUG('!')
            HEX_DEBUG('!')
            HEX_DEBUG('!')
            done = 1;
        }
    }

    return(return_val);
}
