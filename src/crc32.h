/*******************************************************************************
 * (c) Copyright 2015 Microsemi SoC Products Group.  All rights reserved.
 *
 * SmartFusion2 Bootloader CRC32 routine header.
 *
 * SVN $Revision: 7330 $
 * SVN $Date: 2015-04-21 11:01:23 +0100 (Tue, 21 Apr 2015) $
 */


#ifndef CRC32_H_
#define CRC32_H_

uint32_t sf2bl_calc_crc32(uint32_t crc, const uint8_t *data, uint32_t len);

#endif /* CRC32_H_ */
