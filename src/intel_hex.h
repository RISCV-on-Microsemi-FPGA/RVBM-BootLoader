/*******************************************************************************
 * (c) Copyright 2015 Microsemi SoC Products Group.  All rights reserved.
 *
 * SmartFusion2 Bootloader Intel Hex file support header.
 *
 * SVN $Revision: 7330 $
 * SVN $Date: 2015-04-21 11:01:23 +0100 (Tue, 21 Apr 2015) $
 */

#ifndef INTEL_HEX_H_
#define INTEL_HEX_H_

#define IHEX_DATA       0x00
#define IHEX_EOF        0x01
#define IHEX_EXSEG_ADDR 0x02
#define IHEX_START_SEG  0x03
#define IHEX_EXLIN_ADDR 0x04
#define IHEX_START_ADDR 0x05

#define IHEX_MAX_DATA   255

typedef struct hex_rec
{
    int32_t  type;                /* Record type - 0 for */
    int32_t  count;               /* Data byte count */
    uint32_t offset;              /* Address offset field */
    uint32_t address;             /* Segment/linear address */
    uint8_t  data[IHEX_MAX_DATA]; /* Raw data */
} hex_rec_t;

extern hex_rec_t g_hex_record;
extern uint8_t   g_hex_checksum;

int32_t  sf2bl_get_hex_byte(uint8_t *data, int32_t remaining);
int32_t  sf2bl_get_hex_word(uint8_t *data, int32_t remaining);
int32_t  sf2bl_hex_record(uint8_t *data, int32_t remaining);
uint32_t sf2bl_process_hex_file(int32_t received);

#endif /* INTEL_HEX_H_ */
