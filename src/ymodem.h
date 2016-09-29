/* ymodem for SmartFusion2 Bootloader
 *
 * copyright (c) 2015 Microsemi Inc
 *
 * based on ymodem for RTD Serial Recovery (rtdsr)
 * copyright (c) 2011 Pete B. <xtreamerdev@gmail.com>
 *
 * based on ymodem.h for bootldr, copyright (c) 2001 John G Dorsey
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * SVN $Revision: 8448 $
 * SVN $Date: 2016-06-26 15:09:04 +0100 (Sun, 26 Jun 2016) $
 *
 */

#if !defined(_YMODEM_H)
#define _YMODEM_H

#define PACKET_SEQNO_INDEX      (1)
#define PACKET_SEQNO_COMP_INDEX (2)

#define PACKET_HEADER           (3)     /* start, block, block-complement */
#define PACKET_TRAILER          (2)     /* CRC bytes */
#define PACKET_OVERHEAD         (PACKET_HEADER + PACKET_TRAILER)
#define PACKET_SIZE             (128)
#define PACKET_1K_SIZE          (1024)
#define PACKET_TIMEOUT          (1)

#define FILE_NAME_LENGTH (64)
#define FILE_SIZE_LENGTH (16)

/* ASCII control codes: */
#define SOH (0x01)      /* start of 128-byte data packet */
#define STX (0x02)      /* start of 1024-byte data packet */
#define EOT (0x04)      /* end of transmission */
#define ACK (0x06)      /* receive OK */
#define NAK (0x15)      /* receiver error; retry */
#define CAN (0x18)      /* two of these in succession aborts transfer */
#define CRC (0x43)      /* use in place of first NAK for CRC mode */

/* Number of consecutive receive errors before giving up: */
#define MAX_ERRORS    (5)

void sf2bl_ymodem_init(void);
void sf2bl_ymodem_deinit(void);
uint32_t ymodem_receive(uint8_t *buf, uint32_t length);
uint16_t sf2bl_crc16(const uint8_t *buf, uint32_t count);
void _putchar(int32_t data);
void _putstring(uint8_t *string);
void _put_wait(void);

#endif  /* !define(_YMODEM_H) */
