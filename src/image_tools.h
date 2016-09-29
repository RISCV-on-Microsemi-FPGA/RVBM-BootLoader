/*******************************************************************************
 * (c) Copyright 2015 Microsemi SoC Products Group.  All rights reserved.
 *
 * SmartFusion2 Bootloader Image file file support header.
 *
 * SVN $Revision: 7330 $
 * SVN $Date: 2015-04-21 11:01:23 +0100 (Tue, 21 Apr 2015) $
 */

#ifndef IMAGE_TOOLS_H_
#define IMAGE_TOOLS_H_

extern uint8_t g_sector_buffer[4096];

extern img_hdr_block_t    g_img1_header;
extern uint32_t           g_img1_offset;
extern sf2bl_hdr_status_t g_img1_status;

extern uint16_t           g_current_sequence;

#if defined(SF2BL_2ND_IMAGE)
extern img_hdr_block_t    g_img2_header;
extern uint32_t           g_img2_offset;
extern sf2bl_hdr_status_t g_img2_status;
#endif

#if defined(SF2BL_GOLDEN)
extern img_hdr_block_t    g_golden_img_header;
extern uint32_t           g_golden_img_offset;
extern sf2bl_hdr_status_t g_golden_img_status;
#endif

int32_t sf2bl_wr_flash_image(uint32_t address, uint32_t processed);
int32_t sf2bl_rd_flash_image(uint32_t address);
int32_t sf2bl_raw_rd_flash_image(uint32_t address);
void sf2bl_check_flash(void);
sf2bl_hdr_status_t sf2bl_check_img_header(img_hdr_block_t *this_block);

#endif /* IMAGE_TOOLS_H_ */
