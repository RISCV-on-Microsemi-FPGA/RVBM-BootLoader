/*******************************************************************************
 * (c) Copyright 2015 Microsemi SoC Products Group.  All rights reserved.
 *
 * SmartFusion2 Bootloader definitions and structures.
 *
 * SVN $Revision: 8450 $
 * SVN $Date: 2016-06-26 15:18:18 +0100 (Sun, 26 Jun 2016) $
 */

#ifndef SF2BL_DEFS_H_
#define SF2BL_DEFS_H_

#define SF2BL_VERSION_STRING "SmartFusion2 BootLoader v0.1.102"

typedef struct version_block
{
    uint16_t check1;      /* 0xAA55 */
    uint8_t  id[6];       /* "SF2BLV" */
    uint32_t flags;
    uint16_t vn_major;
    uint16_t vn_minor;
    uint16_t vn_sub;
    uint16_t min_major;
    uint16_t min_minor;
    uint16_t min_sub;
    uint8_t  reserved[6];
    uint16_t check2;      /* 0x55AA */
} version_block_t;

typedef struct img_hdr_block
{
    uint32_t valid;
    uint16_t version;
    uint16_t sequence;
    uint32_t flags;
    version_block_t vblock;
    uint32_t size;
    uint32_t crc32;
    uint32_t n_chunks;
    uint8_t  name[64];
    uint8_t  reserved[6];
    uint16_t crc16;
} img_hdr_block_t;

typedef struct img_chunk_hdr
{
    uint32_t base;   /* Linear address of start of up to 64K chunk */
    uint32_t offset; /* Offset of first data in chunk */
    uint32_t len;    /* Length of data in chunk */
    uint32_t index;  /* Chunk number starting from 0 */
} img_chunk_hdr_t;

/*
 * Defines used to manage bitmask which specifies which of the drivers have been
 * initialised by the bootloader. This is important to ensure that drivers are
 * not initialised more than once and for tracking which drivers we need to
 * deinit at the end.
 */

#define SF2BL_DRIVER_MSS_SPI0  0x00000001u
#define SF2BL_DRIVER_MSS_SPI1  0x00000002u
#define SF2BL_DRIVER_MSS_UART0 0x00000004u
#define SF2BL_DRIVER_MSS_UART1 0x00000008u
#define SF2BL_DRIVER_MSS_GPIO  0x00000010u
#define SF2BL_DRIVER_CORE_UART 0x00000020u
#define SF2BL_DRIVER_MSS_PDMA  0x00000040u
#define SF2BL_DRIVER_MSS_WD    0x00000080u
#define SF2BL_DRIVER_SYSTICK   0x00000100u

#define SF2BL_NO_IMAGE 0xFFFFFFFF

/*
 * State values for the bootloader to show what operation is required for this
 * boot.
 */
typedef enum {
    SF2BL_BOOT_EXEC = 0,       /* Normal boot from FLASH operation - image not selected yet */
    SF2BL_BOOT_EXEC_1,         /* Normal boot from FLASH operation - image 1 */
    SF2BL_BOOT_EXEC_2,         /* Normal boot from FLASH operation - image 2 */
    SF2BL_BOOT_EXEC_GOLDEN,    /* Boot from golden image */
    SF2BL_BOOT_COPY_GOLDEN,    /* Copy golden image to normal FLASH image 1 slot and execute */
    SF2BL_BOOT_DOWNLOAD,       /* Flash update requested - image not selected yet */
    SF2BL_BOOT_DOWNLOAD_1,     /* Attempt to update FLASH image 1 and execute */
    SF2BL_BOOT_DOWNLOAD_2,     /* Attempt to update FLASH image 2 and execute */
    SF2BL_BOOT_DOWNLOAD_GOLDEN /* Attempt to update FLASH golden image and execute */
} sf2bl_boot_mode_t;

/*
 * possible statuses for an image header.
 */
typedef enum {
    SF2BL_HDR_BLANK = 0,      /* Sector with header seems to be blank */
    SF2BL_HDR_INVALID,        /* Image valid flag is all 0s */
    SF2BL_HDR_BAD_CRC,        /* Image header CRC check failed */
    SF2BL_HDR_BAD_DATA,       /* Internal consistency checks failed. */
    SF2BL_HDR_READ_FAIL,      /* Read from FLASH operation failed */
    SF2BL_HDR_OK              /* Everything looks good... */
} sf2bl_hdr_status_t;


#define SF2BL_IMG_HDR_BLANK   0xFFFFFFFF /* Blank or update not complete */
#define SF2BL_IMG_HDR_VALID   0xAA55AA55 /* Update complete, active image */
#define SF2BL_IMG_HDR_INVALID 0x00000000 /* Invalid image - probably superseded */

/* Defines for FLASH memory device selection */

#define SF2BL_FLASH_DEV_AT25DF641        0
#define SF2BL_FLASH_DEV_W25Q64FVSSIG     1
#define SF2BL_FLASH_DEV_N25Q00AA13GSF40G 2
#define SF2BL_FLASH_DEV_S25FL128SDPBHICO 3

extern uint32_t g_driver_init;

extern uint8_t *g_rx_base;
extern uint8_t *g_bin_base;
extern uint32_t g_rx_size;
extern uint32_t g_n_images;

extern sf2bl_boot_mode_t g_boot_mode;

extern volatile uint32_t g_10ms_count;


#if defined(SF2BL_USER_HOOK_UPDATE)
int32_t SF2BL_USER_HOOK_UPDATE(void);
#endif

#if defined(SF2BL_USER_HOOK_USE_GOLDEN)
int32_t SF2BL_USER_HOOK_USE_GOLDEN(void);
#endif

#if defined(SF2BL_USER_HOOK_COPY_GOLDEN)
int32_t SF2BL_USER_HOOK_COPY_GOLDEN(void);
#endif

#if defined(SF2BL_USER_HOOK_INIT1)
void SF2BL_USER_HOOK_INIT1(void);
#endif

#if defined(SF2BL_USER_HOOK_INIT2)
void SF2BL_USER_HOOK_INIT2(void);
#endif

#if defined(SF2BL_USER_HOOK_DEINIT)
void SF2BL_USER_HOOK_DEINIT(void);
#endif

#if defined(SF2BL_USER_HOOK_PERIODIC1)
void SF2BL_USER_HOOK_PERIODIC1(void);
#endif

#if defined(SF2BL_USER_HOOK_PERIODIC2)
void SF2BL_USER_HOOK_PERIODIC2(void);
#endif

#if defined(SF2BL_USER_HOOK_STATUS)
void SF2BL_USER_HOOK_STATUS(int32_t state, int32_t percent);
#endif

#endif /* SF2BL_DEFS_H_ */
