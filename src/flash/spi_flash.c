/*******************************************************************************
*
* Copyright (C) 2015 MICROSEMI Corporation.
*
* Company: Microsemi Corporation
*
* File: spi_flash.c
* File history:
*      Revision: 1.0 Date: May 4, 2010
*      Revision: 1.1 Date: September 26, 2012
*      Revision: 1.2 Date: February xx, 2015
*
* Note:
*
* Revision 1.2 is a version for Smartfusion2 Bootloader supporting multiple
* types of SPI FLASH.
*
* Description:
*
* Device driver for the on-board SPI flash for SmartFusion2 KITS supporting the
* following devices:
*
*   Atmel    AT25DF641
*   Winbond  W25Q64FVSSIG
*   Micron   N25Q00AA13GSF40G
*   Spansion S25FL128SDPBHICO
*
* SPI flash driver implementation.
*
* SVN $Revision: 8217 $
* SVN $Date: 2016-01-30 14:44:59 +0000 (Sat, 30 Jan 2016) $
*
*******************************************************************************/

#include <string.h>

#include "spi_flash.h"
#include "sf2_bl_options.h"
#if defined(SF2BL_USE_CORESPI)
#include "hw_platform.h"
#include "core_spi.h"
#else
#include "mss_spi.h"
#include "mss_gpio.h"
#endif
#include "sf2_bl_options.h"
#include "sf2_bl_defs.h"

#ifdef  SF2BL_FLASH_USE_DMA
#include "mss_pdma.h"
#endif


#define READ_ARRAY_OPCODE         0x0B
#define DEVICE_ID_READ            0x9F

#define WRITE_ENABLE_CMD          0x06
#define WRITE_DISABLE_CMD         0x04
#define PROGRAM_PAGE_CMD          0x02
#define WRITE_STATUS1_OPCODE      0x01
#define CHIP_ERASE_OPCODE         0x60
#define ERASE_4K_BLOCK_OPCODE     0x20
#define ERASE_32K_BLOCK_OPCODE    0x52
#define ERASE_64K_BLOCK_OPCODE    0xD8
#define READ_STATUS               0x05
#define PROGRAM_RESUME_CMD        0xD0
#define ENABLE_RESET_CMD          0x66
#define RESET_CMD                 0x99


#define READY_BIT_MASK            0x01
#define PROTECT_SECTOR_OPCODE     0x36
#define UNPROTECT_SECTOR_OPCODE   0x39

#define DONT_CARE                    0

#define NB_BYTES_PER_PAGE          256
#define NB_BYTES_PER_SECTOR        4096

#define BLOCK_ALIGN_MASK_4K      0xFFFFF000
#define BLOCK_ALIGN_MASK_32K     0xFFFF8000
#define BLOCK_ALIGN_MASK_64K     0xFFFF0000

/*
 * SPI device and DMA channel specific defines
 */

#if SF2BL_SPI_PORT == SF2BL_MSS_SPI0
#define SPI_INSTANCE    &g_mss_spi0
#define SPI_SLAVE       MSS_SPI_SLAVE_0
#define DMA_TO_PERI     PDMA_TO_SPI_0
#define PERI_TO_DMA     PDMA_FROM_SPI_0
#define SPI_DEST_TXBUFF PDMA_SPI0_TX_REGISTER
#define SPI_SRC_RXBUFF  PDMA_SPI0_RX_REGISTER
#elif SF2BL_SPI_PORT == SF2BL_MSS_SPI1
#define SPI_INSTANCE    &g_mss_spi1
#define SPI_SLAVE       MSS_SPI_SLAVE_1
#define DMA_TO_PERI     PDMA_TO_SPI_1
#define PERI_TO_DMA     PDMA_FROM_SPI_1
#define SPI_DEST_TXBUFF PDMA_SPI1_TX_REGISTER
#define SPI_SRC_RXBUFF  PDMA_SPI1_RX_REGISTER
#elif SF2BL_SPI_PORT == SF2BL_CORE_SPI
#define SPI_INSTANCE    &g_flash_core_spi
#define SPI_SLAVE       SPI_SLAVE_0
#endif

/*
 *  Comment out the following to use non DMA reads
 *  This will result in slower data transfers and may result in read
 *  failures in systems using a lot of interrupts.
 */
/* #define SF2BL_FLASH_USE_DMA */

#ifdef SF2BL_FLASH_USE_DMA
#define SPI_TRANS_BLOCK spi_flash_transfer_block
#define IRQS_OFF
#define IRQS_ON
#else
#if defined(RISCV_PLATFORM)
#define SPI_TRANS_BLOCK SPI_transfer_block
/*<CJ>TODO: created RISC-V equivalent.*/
#define IRQS_OFF
#define IRQS_ON
#else
#define SPI_TRANS_BLOCK MSS_SPI_transfer_block
#define IRQS_OFF         __disable_irq();
#define IRQS_ON          __enable_irq();
#endif
#endif

#define SF2BL_FLASH_DEV_AT25DF641        0
#define SF2BL_FLASH_DEV_W25Q64FVSSIG     1
#define SF2BL_FLASH_DEV_N25Q00AA13GSF40G 2
#define SF2BL_FLASH_DEV_S25FL128SDPBHICO 3


#if (SF2BL_FLASH_DEVICE == SF2BL_FLASH_DEV_AT25DF641)
/*
 * millisecond timeouts for various operations, based on ADESTO AT25DF641
 * maximums + 10ms and rounded up to the next 10ms
 */

#define FLASH_TIMEOUT_WR_STATUS  30
#define FLASH_TIMEOUT_WR_BYTE    30
#define FLASH_TIMEOUT_WR_PAGE    30
#define FLASH_TIMEOUT_ERASE_4K   220
#define FLASH_TIMEOUT_ERASE_32K  620
#define FLASH_TIMEOUT_ERASE_64K  970
#define FLASH_TIMEOUT_ERASE_CHIP 112000 /* Yes, 112 seconds! */
#define FLASH_TIMEOUT_MISC       50

/*
 * Maximum bytes required for command including opcode,
 * address and any dummy bytes.
 */
#define FLASH_MAX_CMD_BYTES 6

#endif

#if (SF2BL_FLASH_DEVICE == SF2BL_FLASH_DEV_W25Q64FVSSIG)
/*
 * millisecond timeouts for various operations, based on Winbond W25Q64FV
 * maximums + 10ms and rounded up to the next 10ms
 */

#define FLASH_TIMEOUT_WR_STATUS  30
#define FLASH_TIMEOUT_WR_BYTE    30
#define FLASH_TIMEOUT_WR_PAGE    30
#define FLASH_TIMEOUT_ERASE_4K   420
#define FLASH_TIMEOUT_ERASE_32K  1620
#define FLASH_TIMEOUT_ERASE_64K  2020
#define FLASH_TIMEOUT_ERASE_CHIP 100000 /* Yes, 100 seconds! */
#define FLASH_TIMEOUT_MISC       50

/*
 * Maximum bytes required for command including opcode,
 * address and any dummy bytes.
 */
#define FLASH_MAX_CMD_BYTES 6

#endif

#if (SF2BL_FLASH_DEVICE == SF2BL_FLASH_DEV_N25Q00AA13GSF40G)
/*
 * millisecond timeouts for various operations, based on Micron N25Q00AA
 * maximums + 10ms and rounded up to the next 10ms
 */

#define FLASH_TIMEOUT_WR_STATUS  30
#define FLASH_TIMEOUT_WR_BYTE    30
#define FLASH_TIMEOUT_WR_PAGE    30
#define FLASH_TIMEOUT_ERASE_4K   820
#define FLASH_TIMEOUT_ERASE_32K  3020
#define FLASH_TIMEOUT_ERASE_64K  3020
#define FLASH_TIMEOUT_ERASE_CHIP 480000 /* Yes, 480 seconds! For one of the 4 die... */
#define FLASH_TIMEOUT_MISC       50

/*
 * Maximum bytes required for command including opcode,
 * address and any dummy bytes.
 */
#define FLASH_MAX_CMD_BYTES 7

/*
 * Local global to indicate if the FLASH device is configured for extended 4
 * byte address in the non volatile configuration register. This determines if
 * we need to add preamble and postamble calls to enter and exit 4 byte
 * addressing before any operations that need an address.
 */
static int32_t g_4_byte_addr_enabled = 0;

/* Non volatile config bit masks */

#define NV_CFG_DUMMY_CLOCKS   0xF000
#define NV_CFG_XIP_MODE       0x0E00
#define NV_CFG_DRIVE_STRENGTH 0x01C0
#define NV_CFG_RESERVED          0x0020
#define NV_CFG_RESET_HOLD     0x0010
#define NV_CFG_QUAD_IO        0x0008
#define NV_CFG_DUAL_IO        0x0004
#define NV_CFG_128MB_SELECT   0x0002
#define NV_CFG_ADDRESS_BYTES  0x0001

/* Volatile config bit masks */

#define V_CFG_DUMMY_CLOCKS    0xF0
#define V_CFG_XIP             0x08
#define V_CFG_RESERVED        0x04
#define V_CFG_WRAP            0x03

/* Flag Status register bit masks */

#define FSR_WRITE_BUSY        0x80
#define FSR_ERASE_SUSPEND     0x40
#define FSR_ERASE_ERROR       0x20
#define FSR_PROGRAM_ERROR     0x10
#define FSR_VPP_ERROR         0x08
#define FSR_PROGRAM_SUSPEND   0x04
#define FSR_PROTECTION_ERROR  0x02
#define FSR_4_BYTE_ADDRESS    0x01


/* Additional commands */

#define READ_NV_CFG_REG_CMD   0xB5 /* 2 data bytes */
#define WRITE_NV_CFG_REG_CMD  0xB1 /* 2 data bytes */
#define READ_V_CFG_REG_CMD    0x85 /* 1 data byte */
#define WRITE_V_CFG_REG_CMD   0x81 /* 1 data byte */
#define ENTER_4_BYTE_ADR_MODE 0xB7
#define EXIT_4_BYTE_ADR_MODE  0xE9
#define READ_FLAG_STATUS_REG  0x70
#define CLEAR_FLAG_STATUS_REG 0x50

#define ENABLE_4_BYTE_ADDR  1
#define DISABLE_4_BYTE_ADDR 0
static spi_flash_status_t spi_flash_4_byte_addr(int32_t enable);
#endif

#if (SF2BL_FLASH_DEVICE == SF2BL_FLASH_DEV_S25FL128SDPBHICO)
#endif

#if defined(SF2BL_FLASH_WRITE_VERIFY)
/* Somewhere to read back up to 1 page of data to verify last write operation */
static uint8_t verify_buffer[NB_BYTES_PER_PAGE];
#endif

static uint8_t wait_ready(uint32_t timeout);
static int32_t spi_flash_insert_cmd_addr(uint8_t *dest, uint32_t command, uint32_t address);

#if defined(SF2BL_USE_CORESPI)
spi_instance_t g_flash_core_spi;
#endif

/******************************************************************************
 *For more details please refer the spi_flash.h file
 ******************************************************************************/
spi_flash_status_t spi_flash_init( void )
{
#if (SF2BL_FLASH_DEVICE == SF2BL_FLASH_DEV_N25Q00AA13GSF40G)
    spi_flash_status_t status;
    int16_t nv_cfg;
    int8_t  v_cfg;
#endif

    /*--------------------------------------------------------------------------
     * Configure MSS_SPI.
     */
#if SF2BL_SPI_PORT == SF2BL_CORE_SPI
	SPI_init( SPI_INSTANCE, FLASH_CORE_SPI_BASE, 32 );
	SPI_configure_master_mode( SPI_INSTANCE );
	SPI_set_slave_select( SPI_INSTANCE, SPI_SLAVE );
#else
    MSS_SPI_init(SPI_INSTANCE);
    MSS_SPI_configure_master_mode
    (
        SPI_INSTANCE,
        SPI_SLAVE,
        MSS_SPI_MODE3,
#ifdef SF2BL_FLASH_USE_DMA
        4,                  /* For MPM on SmartFusion 2 we currently use a 80MHz BCLK */
                            /* Which gives a 20MHz SPI CLK */
#else
        /*
         *  Note: MSS_SPI_PCLK_DIV_16 was ok on the A2F200 but would
         *  not work on the A2F500.
         */
      32,
#endif
        MSS_SPI_BLOCK_TRANSFER_FRAME_SIZE
    );

    if(SPI_INSTANCE == &g_mss_spi1)
    {
        g_driver_init |=  SF2BL_DRIVER_MSS_SPI1;
    }
    else
    {
        g_driver_init |=  SF2BL_DRIVER_MSS_SPI0;
    }
#endif

#ifdef SF2BL_FLASH_USE_DMA
    /*--------------------------------------------------------------------------
     * Configure DMA channel used as part of this MSS_SPI Flash driver.
     */
    PDMA_init();
    g_driver_init |= SF2BL_DRIVER_MSS_PDMA;

    PDMA_configure
    (
        SF2BL_FLASH_TX_DMA_CHAN,
        DMA_TO_PERI,
        PDMA_LOW_PRIORITY | PDMA_BYTE_TRANSFER | PDMA_INC_SRC_ONE_BYTE,
        PDMA_DEFAULT_WRITE_ADJ
    );

    PDMA_configure
    (
        SF2BL_FLASH_RX_DMA_CHAN,
        PERI_TO_DMA,
        PDMA_LOW_PRIORITY | PDMA_BYTE_TRANSFER | PDMA_INC_DEST_ONE_BYTE,
        PDMA_DEFAULT_WRITE_ADJ
    );
#endif

#if 0
    MSS_GPIO_init();
    MSS_GPIO_config(MSS_GPIO_0 , MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(MSS_GPIO_1 , MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(MSS_GPIO_2 , MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(MSS_GPIO_3 , MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(MSS_GPIO_4 , MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(MSS_GPIO_5 , MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(MSS_GPIO_6 , MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(MSS_GPIO_7 , MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(MSS_GPIO_8 , MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_config(MSS_GPIO_9 , MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_config(MSS_GPIO_10, MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_config(MSS_GPIO_11, MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_config(MSS_GPIO_12, MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_config(MSS_GPIO_13, MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_config(MSS_GPIO_14, MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_config(MSS_GPIO_15, MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_config(MSS_GPIO_17, MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_config(MSS_GPIO_18, MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_config(MSS_GPIO_24, MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_config(MSS_GPIO_25, MSS_GPIO_OUTPUT_MODE);
#endif

#if SF2BL_NO_PIN != SF2BL_SPI_WP
    MSS_GPIO_config(SF2BL_SPI_WP, MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_set_output(SF2BL_SPI_WP , 1u);
#endif

#if SF2BL_NO_PIN != SF2BL_SPI_RESET
    MSS_GPIO_config(SF2BL_SPI_RESET, MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_set_output(SF2BL_SPI_RESET , 1u);
#endif

#if (SF2BL_FLASH_DEVICE == SF2BL_FLASH_DEV_N25Q00AA13GSF40G)
    /* Read non volatile config register and check for 4 byte address mode */
    status = spi_flash_control_hw(SPI_FLASH_READ_NV_CFG, 0, (void *)&nv_cfg);
    if(SPI_FLASH_SUCCESS == status)
    {
        g_4_byte_addr_enabled = 0 == (nv_cfg & NV_CFG_ADDRESS_BYTES) ? 1 : 0;

        /*
         * Ensure single dummy byte selected for fast read operations which will
         * suffice for up to 90MHz SPI clock speed.
         */
        status = spi_flash_control_hw(SPI_FLASH_READ_V_CFG, 0, (void *)&v_cfg);
        if(SPI_FLASH_SUCCESS == status)
        {
            v_cfg = (v_cfg & ~V_CFG_DUMMY_CLOCKS) | 0x80;
            status = spi_flash_control_hw(SPI_FLASH_WRITE_V_CFG, (uint32_t)v_cfg, 0);
        }
    }

    return(status);
#else
    return(SPI_FLASH_SUCCESS);
#endif
}

void spi_flash_deinit( void )
{
    /*--------------------------------------------------------------------------
     * Configure MSS_SPI.
     */
#if SF2BL_SPI_PORT == SF2BL_CORE_SPI
    SPI_init( SPI_INSTANCE, FLASH_CORE_SPI_BASE, 32 );
    /*<CJ>TODO: signal SPI driver deinit through g_driver_init.*/
#else
    MSS_SPI_deinit(SPI_INSTANCE);
    
    if(SPI_INSTANCE == &g_mss_spi1)
    {
        g_driver_init &= ~SF2BL_DRIVER_MSS_SPI1;
    }
    else
    {
        g_driver_init &= ~SF2BL_DRIVER_MSS_SPI0;
    }
#endif

    /*
     * PDMA_init() resets the PDMA hardware and disables all interrupts and
     * shuts down any DMA transfers.
     * This means a separate deinit is not needed.
     */
#ifdef SF2BL_FLASH_USE_DMA
    PDMA_init();
    NVIC_DisableIRQ(DMA_IRQn); /* Kill ints at NVIC as well */
    g_driver_init &= ~SF2BL_DRIVER_MSS_PDMA;
#endif
}
/*
 *  The following have been borrowed from the MSS SPI driver as they are
 *  defined in the .c file instead of a header where they would be
 *  available for use here
 */

#define RX_OVERFLOW_MASK        0x00000004u
#define TX_FIFO_RESET_MASK      0x00000008u
#define RX_FIFO_RESET_MASK      0x00000004u
#define RX_DATA_READY_MASK      0x00000002u
/*
 * Transfer a command to the FLASH device and read the response using 2 DMA
 * channels and two buffers to handle the read/write transfers which are
 * required to keep the SPI interface happy.
 */
#ifdef SF2BL_FLASH_USE_DMA
static void spi_flash_transfer_block
(
#if SF2BL_SPI_PORT == SF2BL_CORE_SPI
    spi_instance_t * this_spi,
#else
    mss_spi_instance_t * this_spi,
#endif
    const uint8_t * cmd_buffer,
    uint16_t cmd_byte_size,
    uint8_t * rd_buffer,
    uint16_t rd_byte_size
)
{
    static uint8_t dummy_sink[FLASH_MAX_CMD_BYTES]; /* Safe place to discard bytes */
    uint32_t transfer_size;
    transfer_size = cmd_byte_size + rd_byte_size;

    /* Flush the Tx and Rx FIFOs. Please note this does not have any effect on A2F200. */
    this_spi->hw_reg->COMMAND |= (TX_FIFO_RESET_MASK | RX_FIFO_RESET_MASK);

#if 0
    { /* Make this a block so local variables can be defined here and not at the top of the fn */
        volatile uint32_t dummy;
        uint32_t rx_ready;
        /* Flush Rx FIFO. */
        rx_ready = this_spi->hw_reg->STATUS & RX_DATA_READY_MASK;
        while(rx_ready)
        {
            dummy = this_spi->hw_reg->RX_DATA;
            dummy = dummy;  /* Prevent Lint warning. */
            rx_ready = this_spi->hw_reg->STATUS & RX_DATA_READY_MASK;
        }
    }
#endif
    /*
     * The following code shouldn't be needed with PDMA as receive overflow
     * should not happen. It is copied from MSS_SPI_transfer_block(). If
     * you do need to use it you will also have to change the mss_spi.c
     * recover_from_rx_overflow() to non static and add a function prototype
     * to provide access to it.
     */
#if 0
    {
        uint32_t rx_overflow;

        /* Recover from receive overflow. */
        rx_overflow = this_spi->hw_reg->STATUS & RX_OVERFLOW_MASK;
        if(rx_overflow)
        {
            /*recover_from_rx_overflow(this_spi);*/
            rx_overflow++;
        }
    }
#endif

    MSS_SPI_disable(this_spi);
    MSS_SPI_set_transfer_byte_count(this_spi, transfer_size);

    /*
     * First point second DMA channel at safe place to discard
     * bytes into when we write the command.
     */

    PDMA_start(SF2BL_FLASH_RX_DMA_CHAN, SPI_SRC_RXBUFF, (uint32_t)&dummy_sink,
        cmd_byte_size);

    /*
     * Next set up DMA channel to write command to SPI port.
     */
    PDMA_start(SF2BL_FLASH_TX_DMA_CHAN, (uint32_t)cmd_buffer, SPI_DEST_TXBUFF,
        cmd_byte_size);

    /* If expecting a response, set up DMA for doing that. */
    if(rd_byte_size)
    {
        /*
         * Set up DMA channel to transfer response from SPI to memory
         */
        PDMA_load_next_buffer(SF2BL_FLASH_RX_DMA_CHAN, SPI_SRC_RXBUFF,
            (uint32_t)rd_buffer, rd_byte_size);

        /*
         * Ideally we would write 0s to the SPI interface for the dummy
         * bytes we have to send as we read from the SPI interface but that
         * requires reserving a block of 0s. Instead, we will simply use our
         * receive buffer on the basis that it has enough bytes in it and it
         * doesn't matter that we are recycling the data the device has already
         * supplied to us.
         */
        PDMA_load_next_buffer(SF2BL_FLASH_TX_DMA_CHAN, (uint32_t)rd_buffer,
            SPI_DEST_TXBUFF, rd_byte_size);
    }

    MSS_SPI_enable(this_spi);
    while (!MSS_SPI_tx_done(this_spi))
    {
        ;
    }
}
#endif

/******************************************************************************
 *For more details please refer the spi_flash.h file
 ******************************************************************************/
spi_flash_status_t
spi_flash_control_hw
(
    spi_flash_control_hw_t operation,
    uint32_t peram1,
    void *   ptrPeram
)
{
    spi_flash_status_t return_val = SPI_FLASH_SUCCESS;
    uint8_t cmd_buffer[FLASH_MAX_CMD_BYTES];
    uint32_t timeout = 0; /* Exit timeout, 0 for not required */
    int32_t cmd_len;

#if SF2BL_SPI_PORT == SF2BL_CORE_SPI
    SPI_set_slave_select(SPI_INSTANCE, SPI_SLAVE);
#else
    MSS_SPI_set_slave_select(SPI_INSTANCE, SPI_SLAVE);
#endif

    switch(operation){
        case SPI_FLASH_READ_DEVICE_ID:
        {
            uint8_t read_buffer[2];

            /* Check for completion of any prior command */
            if(0 != wait_ready(FLASH_TIMEOUT_MISC))
            {
                return_val = SPI_FLASH_TIMEOUT;
            }
            else
            {
                cmd_buffer[0] = DEVICE_ID_READ;
                IRQS_OFF
                SPI_TRANS_BLOCK(SPI_INSTANCE, cmd_buffer, 1, read_buffer,
                                sizeof(read_buffer));
                IRQS_ON

                ((spi_dev_info_t *)ptrPeram)->manufacturer_id = read_buffer[0];
                ((spi_dev_info_t *)ptrPeram)->device_id       = read_buffer[1];
            }
        }
        break;

#if (SF2BL_FLASH_DEVICE == SF2BL_FLASH_DEV_AT25DF641)
        case SPI_FLASH_SECTOR_PROTECT:
        {
            /* Check for completion of any prior command */
            if(0 != wait_ready(FLASH_TIMEOUT_MISC))
            {
                return_val = SPI_FLASH_TIMEOUT;
            }
            else
            {
                /* Send Write Enable command */
                cmd_buffer[0] = WRITE_ENABLE_CMD;
                IRQS_OFF
                SPI_TRANS_BLOCK(SPI_INSTANCE, cmd_buffer, 1, 0, 0);
                IRQS_ON
                if(0 != wait_ready(FLASH_TIMEOUT_MISC))
                {
                    return_val = SPI_FLASH_TIMEOUT;
                }
                else
                {
                    /* Safe to protect sector now */
                	cmd_len = spi_flash_insert_cmd_addr(cmd_buffer, PROTECT_SECTOR_OPCODE, peram1);
                    IRQS_OFF
                    SPI_TRANS_BLOCK(SPI_INSTANCE, cmd_buffer, cmd_len, 0, 0);
                    IRQS_ON

                    timeout = FLASH_TIMEOUT_MISC;
                }
            }
        }
        break;

        case SPI_FLASH_SECTOR_UNPROTECT:
        {
            /* Check for completion of any prior command */
            if(0 != wait_ready(FLASH_TIMEOUT_MISC))
            {
                return_val = SPI_FLASH_TIMEOUT;
            }
            else
            {
                /* Send Write Enable command */
                cmd_buffer[0] = WRITE_ENABLE_CMD;
                IRQS_OFF
                SPI_TRANS_BLOCK(SPI_INSTANCE, cmd_buffer, 1, 0, 0);
                IRQS_ON

                if(0 != wait_ready(FLASH_TIMEOUT_MISC))
                {
                    return_val = SPI_FLASH_TIMEOUT;
                }
                else
                {
                    /* Now safe to unprotect sector */
                	cmd_len = spi_flash_insert_cmd_addr(cmd_buffer, UNPROTECT_SECTOR_OPCODE, peram1);
                    IRQS_OFF
                    SPI_TRANS_BLOCK(SPI_INSTANCE, cmd_buffer, cmd_len, 0, 0);
                    IRQS_ON

                    timeout = FLASH_TIMEOUT_MISC;
                }
            }
        }
        break;

        case SPI_FLASH_GLOBAL_PROTECT:
        case SPI_FLASH_GLOBAL_UNPROTECT:
        {
            if(0 != wait_ready(FLASH_TIMEOUT_MISC))
            {
                return_val = SPI_FLASH_TIMEOUT;
            }
            else
            {
                /* Send Write Enable command */
                cmd_buffer[0] = WRITE_ENABLE_CMD;
                IRQS_OFF
                SPI_TRANS_BLOCK(SPI_INSTANCE, cmd_buffer, 1, 0, 0);
                IRQS_ON

                if(0 != wait_ready(FLASH_TIMEOUT_MISC))
                {
                    return_val = SPI_FLASH_TIMEOUT;
                }
                else
                {
                    /* Send Chip Erase command */
                    cmd_buffer[0] = WRITE_STATUS1_OPCODE;
                    cmd_buffer[1] = 0;

                    IRQS_OFF
                    SPI_TRANS_BLOCK(SPI_INSTANCE, cmd_buffer, 2, 0, 0);
                    IRQS_ON

                    timeout = FLASH_TIMEOUT_MISC;
                }
            }
        }
        break;
#endif
        case SPI_FLASH_CHIP_ERASE:
        {
            if(0 != wait_ready(FLASH_TIMEOUT_MISC))
            {
                return_val = SPI_FLASH_TIMEOUT;
            }
            else
            {
                /* Send Write Enable command */
                cmd_buffer[0] = WRITE_ENABLE_CMD;
                IRQS_OFF
                SPI_TRANS_BLOCK(SPI_INSTANCE, cmd_buffer, 1, 0, 0);
                IRQS_ON

                if(0 != wait_ready(FLASH_TIMEOUT_MISC))
                {
                    return_val = SPI_FLASH_TIMEOUT;
                }
                else
                {
                    /* Send Chip Erase command */
                    cmd_buffer[0] = CHIP_ERASE_OPCODE;

                    IRQS_OFF
                    SPI_TRANS_BLOCK(SPI_INSTANCE, cmd_buffer, 1, 0, 0);
                    IRQS_ON

                    timeout = FLASH_TIMEOUT_ERASE_CHIP;
                }
            }
        }
        break;

        case SPI_FLASH_RESET:
        {
            /*
             * Reset is a two step operation and if you really want to do it,
             * you will not test for completion of previous command as you are
             * probably not concerned about that!
             *
             * It's a two step process so send Enable Reset command first
             */
            cmd_buffer[0] = ENABLE_RESET_CMD;
            IRQS_OFF
            SPI_TRANS_BLOCK(SPI_INSTANCE, cmd_buffer, 1, 0, 0);
            IRQS_ON

            /* Followed immediately by Reset command */
            cmd_buffer[0] = ENABLE_RESET_CMD;
            IRQS_OFF
            SPI_TRANS_BLOCK(SPI_INSTANCE, cmd_buffer, 1, 0, 0);
            IRQS_ON

            timeout = FLASH_TIMEOUT_MISC;
        }
        break;

        case SPI_FLASH_4KBLOCK_ERASE:
        {
            if(0 != wait_ready(FLASH_TIMEOUT_MISC))
            {
                return_val = SPI_FLASH_TIMEOUT;
            }
            else
            {
				/* Select block start by masking off non significant bits */
				peram1 &= BLOCK_ALIGN_MASK_4K;

#if (SF2BL_FLASH_DEVICE == SF2BL_FLASH_DEV_N25Q00AA13GSF40G)
				return_val = spi_flash_4_byte_addr(ENABLE_4_BYTE_ADDR);
				if(SPI_FLASH_SUCCESS == return_val)
				{
					/* Send Write Enable command */
					cmd_buffer[0] = WRITE_ENABLE_CMD;
					IRQS_OFF
					SPI_TRANS_BLOCK(SPI_INSTANCE, cmd_buffer, 1, 0, 0);
					IRQS_ON

					if(0 != wait_ready(FLASH_TIMEOUT_MISC))
					{
						return_val = SPI_FLASH_TIMEOUT;
					}
				}
				if(SPI_FLASH_SUCCESS == return_val)
				{
					/* Send 4K Block Erase command */
					cmd_len = spi_flash_insert_cmd_addr(cmd_buffer, ERASE_4K_BLOCK_OPCODE, peram1);
					IRQS_OFF
					SPI_TRANS_BLOCK(SPI_INSTANCE, cmd_buffer, cmd_len, 0, 0);
					IRQS_ON

					if(0 != wait_ready(FLASH_TIMEOUT_ERASE_4K))
					{
						return_val = SPI_FLASH_TIMEOUT;
					}
				}
				if(SPI_FLASH_SUCCESS == return_val)
				{
					return_val = spi_flash_4_byte_addr(DISABLE_4_BYTE_ADDR);
				}
#else
				if(SPI_FLASH_SUCCESS == return_val)
				{
					/* Send Write Enable command */
					cmd_buffer[0] = WRITE_ENABLE_CMD;
					IRQS_OFF
					SPI_TRANS_BLOCK(SPI_INSTANCE, cmd_buffer, 1, 0, 0);
					IRQS_ON

					if(0 != wait_ready(FLASH_TIMEOUT_MISC))
					{
						return_val = SPI_FLASH_TIMEOUT;
					}
				}
				if(SPI_FLASH_SUCCESS == return_val)
				{
					/* Send 4K Block Erase command */
					cmd_len = spi_flash_insert_cmd_addr(cmd_buffer, ERASE_4K_BLOCK_OPCODE, peram1);
					IRQS_OFF
					SPI_TRANS_BLOCK(SPI_INSTANCE, cmd_buffer, cmd_len, 0, 0);
					IRQS_ON

					timeout = FLASH_TIMEOUT_ERASE_4K;
				}
#endif
            }
        }
        break;

#if (SF2BL_FLASH_DEVICE != SF2BL_FLASH_DEV_N25Q00AA13GSF40G)
        case SPI_FLASH_32KBLOCK_ERASE:
        {
            if(0 != wait_ready(FLASH_TIMEOUT_MISC))
            {
                return_val = SPI_FLASH_TIMEOUT;
            }
            else
            {
                /* Send Write Enable command */
                cmd_buffer[0] = WRITE_ENABLE_CMD;
                IRQS_OFF
                SPI_TRANS_BLOCK(SPI_INSTANCE, cmd_buffer, 1, 0, 0);
                IRQS_ON

                if(0 != wait_ready(FLASH_TIMEOUT_MISC))
                {
                    return_val = SPI_FLASH_TIMEOUT;
                }
                else
                {
                    /* Select block start by masking off non significant bits */
                    peram1 &= BLOCK_ALIGN_MASK_32K;

                    /* Send 32K Block Erase command */
                	cmd_len = spi_flash_insert_cmd_addr(cmd_buffer, ERASE_32K_BLOCK_OPCODE, peram1);

                    IRQS_OFF
                    SPI_TRANS_BLOCK(SPI_INSTANCE, cmd_buffer, cmd_len, 0, 0);
                    IRQS_ON

                    timeout = FLASH_TIMEOUT_ERASE_32K;
                }
            }
        }
        break;
#endif
        case SPI_FLASH_64KBLOCK_ERASE:
        {
            if(0 != wait_ready(FLASH_TIMEOUT_MISC))
            {
                return_val = SPI_FLASH_TIMEOUT;
            }
            else
            {
				/* Select block start by masking off non significant bits */
				peram1 &= BLOCK_ALIGN_MASK_64K;

#if (SF2BL_FLASH_DEVICE == SF2BL_FLASH_DEV_N25Q00AA13GSF40G)
				return_val = spi_flash_4_byte_addr(ENABLE_4_BYTE_ADDR);
				if(SPI_FLASH_SUCCESS == return_val)
				{
					/* Send Write Enable command */
					cmd_buffer[0] = WRITE_ENABLE_CMD;
					IRQS_OFF
					SPI_TRANS_BLOCK(SPI_INSTANCE, cmd_buffer, 1, 0, 0);
					IRQS_ON

					if(0 != wait_ready(FLASH_TIMEOUT_MISC))
					{
						return_val = SPI_FLASH_TIMEOUT;
					}
				}
				if(SPI_FLASH_SUCCESS == return_val)
				{
					/* Send 64K Block Erase command */
					cmd_len = spi_flash_insert_cmd_addr(cmd_buffer, ERASE_64K_BLOCK_OPCODE, peram1);
					IRQS_OFF
					SPI_TRANS_BLOCK(SPI_INSTANCE, cmd_buffer, cmd_len, 0, 0);
					IRQS_ON

					if(0 != wait_ready(FLASH_TIMEOUT_ERASE_64K))
					{
						return_val = SPI_FLASH_TIMEOUT;
					}
				}
				if(SPI_FLASH_SUCCESS == return_val)
				{
					return_val = spi_flash_4_byte_addr(DISABLE_4_BYTE_ADDR);
				}
#else
				if(SPI_FLASH_SUCCESS == return_val)
				{
					/* Send Write Enable command */
					cmd_buffer[0] = WRITE_ENABLE_CMD;
					IRQS_OFF
					SPI_TRANS_BLOCK(SPI_INSTANCE, cmd_buffer, 1, 0, 0);
					IRQS_ON

					if(0 != wait_ready(FLASH_TIMEOUT_MISC))
					{
						return_val = SPI_FLASH_TIMEOUT;
					}
				}
				if(SPI_FLASH_SUCCESS == return_val)
				{
					/* Send 64K Block Erase command */
					cmd_len = spi_flash_insert_cmd_addr(cmd_buffer, ERASE_64K_BLOCK_OPCODE, peram1);
					IRQS_OFF
					SPI_TRANS_BLOCK(SPI_INSTANCE, cmd_buffer, cmd_len, 0, 0);
					IRQS_ON

					timeout = FLASH_TIMEOUT_ERASE_64K;
				}
#endif
            }
        }
        break;

#if (SF2BL_FLASH_DEVICE == SF2BL_FLASH_DEV_N25Q00AA13GSF40G)
        case SPI_FLASH_READ_NV_CFG:
        {
            uint8_t read_buffer[2];

            /* Check for completion of any prior command */
            if(0 != wait_ready(FLASH_TIMEOUT_MISC))
            {
                return_val = SPI_FLASH_TIMEOUT;
            }
            else
            {
                cmd_buffer[0] = READ_NV_CFG_REG_CMD;
                IRQS_OFF
                SPI_TRANS_BLOCK(SPI_INSTANCE, cmd_buffer, 1, read_buffer, 2);
                IRQS_ON

                *(uint16_t *)ptrPeram = (read_buffer[0] << 8) + read_buffer[1];
            }
        }
        break;

        case SPI_FLASH_WRITE_NV_CFG:
        {
            /* Check for completion of any prior command */
            if(0 != wait_ready(FLASH_TIMEOUT_MISC))
            {
                return_val = SPI_FLASH_TIMEOUT;
            }
            else
            {
                /* Send Write Enable command */
                cmd_buffer[0] = WRITE_ENABLE_CMD;
                IRQS_OFF
                SPI_TRANS_BLOCK(SPI_INSTANCE, cmd_buffer, 1, 0, 0);
                IRQS_ON
                if(0 != wait_ready(FLASH_TIMEOUT_MISC))
                {
                    return_val = SPI_FLASH_TIMEOUT;
                }
                else
                {
                    /* Safe to modify register now */
                    cmd_buffer[0] = WRITE_NV_CFG_REG_CMD;
                    cmd_buffer[1] = (uint8_t)(peram1 >> 8);
                    cmd_buffer[2] = (uint8_t)peram1;
                    IRQS_OFF
                    SPI_TRANS_BLOCK(SPI_INSTANCE, cmd_buffer, 3, 0, 0);
                    IRQS_ON

                    timeout = FLASH_TIMEOUT_MISC;
                }
            }
        }
        break;

        case SPI_FLASH_READ_V_CFG:
        {
            uint8_t read_buffer;

            /* Check for completion of any prior command */
            if(0 != wait_ready(FLASH_TIMEOUT_MISC))
            {
                return_val = SPI_FLASH_TIMEOUT;
            }
            else
            {
                cmd_buffer[0] = READ_V_CFG_REG_CMD;
                IRQS_OFF
                SPI_TRANS_BLOCK(SPI_INSTANCE, cmd_buffer, 1, &read_buffer, 1);
                IRQS_ON

                *(uint8_t *)ptrPeram = read_buffer;
            }
        }
        break;

        case SPI_FLASH_WRITE_V_CFG:
        {
            /* Check for completion of any prior command */
            if(0 != wait_ready(FLASH_TIMEOUT_MISC))
            {
                return_val = SPI_FLASH_TIMEOUT;
            }
            else
            {
                /* Send Write Enable command */
                cmd_buffer[0] = WRITE_ENABLE_CMD;
                IRQS_OFF
                SPI_TRANS_BLOCK(SPI_INSTANCE, cmd_buffer, 1, 0, 0);
                IRQS_ON
                if(0 != wait_ready(FLASH_TIMEOUT_MISC))
                {
                    return_val = SPI_FLASH_TIMEOUT;
                }
                else
                {
                    /* Safe to modify register now */
                    cmd_buffer[0] = WRITE_V_CFG_REG_CMD;
                    cmd_buffer[1] = (uint8_t)peram1;
                    IRQS_OFF
                    SPI_TRANS_BLOCK(SPI_INSTANCE, cmd_buffer, 2, 0, 0);
                    IRQS_ON

                    timeout = FLASH_TIMEOUT_MISC;
                }
            }
        }
        break;
#endif
        default:
              return_val = SPI_FLASH_INVALID_ARGUMENTS;
        break;
    }

    if(0 != timeout)
    {
        if(0 != wait_ready(timeout))
        {
            return_val = SPI_FLASH_TIMEOUT;
        }
    }

#if SF2BL_SPI_PORT == SF2BL_CORE_SPI
    SPI_clear_slave_select(SPI_INSTANCE, SPI_SLAVE);
#else
    MSS_SPI_clear_slave_select(SPI_INSTANCE, SPI_SLAVE);
#endif

    return(return_val);
}


/******************************************************************************
 *For more details please refer the spi_flash.h file
 ******************************************************************************/
spi_flash_status_t
spi_flash_read
(
    uint32_t address,
    uint8_t * rx_buffer,
    size_t size_in_bytes
)
{
    spi_flash_status_t return_val = SPI_FLASH_SUCCESS;
    uint8_t cmd_buffer[FLASH_MAX_CMD_BYTES];
    int32_t cmd_len;

#if SF2BL_SPI_PORT == SF2BL_CORE_SPI
    SPI_set_slave_select(SPI_INSTANCE, SPI_SLAVE);
#else
    MSS_SPI_set_slave_select(SPI_INSTANCE, SPI_SLAVE);
#endif
    
    if(0 != wait_ready(FLASH_TIMEOUT_MISC))
    {
        return_val = SPI_FLASH_TIMEOUT;
    }
    else
    {
#if (SF2BL_FLASH_DEVICE == SF2BL_FLASH_DEV_N25Q00AA13GSF40G)
		return_val = spi_flash_4_byte_addr(ENABLE_4_BYTE_ADDR);
		if(SPI_FLASH_SUCCESS == return_val)
		{
			cmd_len = spi_flash_insert_cmd_addr(cmd_buffer, READ_ARRAY_OPCODE, address);
			cmd_buffer[cmd_len] = DONT_CARE;

			IRQS_OFF
			SPI_TRANS_BLOCK(SPI_INSTANCE, cmd_buffer, cmd_len + 1, rx_buffer, size_in_bytes);
			IRQS_ON
		}
		if(SPI_FLASH_SUCCESS == return_val)
		{
			return_val = spi_flash_4_byte_addr(DISABLE_4_BYTE_ADDR);
		}
#else
		cmd_len = spi_flash_insert_cmd_addr(cmd_buffer, READ_ARRAY_OPCODE, address);
		cmd_buffer[cmd_len] = DONT_CARE;

		IRQS_OFF
		SPI_TRANS_BLOCK(SPI_INSTANCE, cmd_buffer, cmd_len + 1, rx_buffer, size_in_bytes);
		IRQS_ON
#endif
    }

#if SF2BL_SPI_PORT == SF2BL_CORE_SPI
    SPI_clear_slave_select(SPI_INSTANCE, SPI_SLAVE);
#else
    MSS_SPI_clear_slave_select(SPI_INSTANCE, SPI_SLAVE);
#endif
    return(return_val);
}


/*******************************************************************************
 * This function sends the command and data on SPI
 */
#ifdef SF2BL_FLASH_USE_DMA
static void write_cmd_data
(
    mss_spi_instance_t * this_spi,
    const uint8_t * cmd_buffer,
    uint16_t cmd_byte_size,
    uint8_t * data_buffer,
    uint16_t data_byte_size
)
{
    uint32_t transfer_size;

    transfer_size = cmd_byte_size + data_byte_size;

    MSS_SPI_disable(this_spi);
    MSS_SPI_set_transfer_byte_count(this_spi, transfer_size);

    PDMA_start
        (
            SF2BL_FLASH_TX_DMA_CHAN,  /* channel_id */
            (uint32_t)cmd_buffer,   /* src_addr */
            SPI_DEST_TXBUFF,             /* dest_addr */
            cmd_byte_size           /* transfer_count */
        );

    PDMA_load_next_buffer
        (
            SF2BL_FLASH_TX_DMA_CHAN,  /* channel_id */
            (uint32_t)data_buffer,  /* src_addr */
            SPI_DEST_TXBUFF,             /* dest_addr */
            data_byte_size          /* transfer_count */
        );
    MSS_SPI_enable(this_spi);
    while(!MSS_SPI_tx_done(this_spi))
    {
        ;
    }
}
#else
static void write_cmd_data
(
    spi_instance_t * this_spi,
    const uint8_t * cmd_buffer,
    uint16_t cmd_byte_size,
    uint8_t * data_buffer,
    uint16_t data_byte_size
)
{
    uint8_t tx_buffer[512];
    uint32_t tx_size;
    uint32_t inc;
    
    if ((cmd_byte_size + data_byte_size) <= 512)
    {
        for(inc = 0; inc < cmd_byte_size; ++inc)
        {
            tx_buffer[inc] = cmd_buffer[inc];
        }
        
        for(inc = 0; inc < data_byte_size; ++inc)
        {
            tx_buffer[cmd_byte_size + inc] = data_buffer[inc];
        }

        IRQS_OFF
        SPI_TRANS_BLOCK(SPI_INSTANCE, tx_buffer, cmd_byte_size + data_byte_size, 0, 0);
        IRQS_ON
    }
    else
    {
        //<CJ>: replace with assertion once assertion tested.
        _exit(12);
    }
}
#endif

/******************************************************************************
 *For more details please refer the spi_flash.h file
 ******************************************************************************/
spi_flash_status_t
spi_flash_write
(
    uint32_t address,
    uint8_t * write_buffer,
    size_t size_in_bytes,
    uint32_t format
)
{
    spi_flash_status_t return_val = SPI_FLASH_SUCCESS;
    uint8_t cmd_buffer[FLASH_MAX_CMD_BYTES];
    int32_t cmd_len;
    uint32_t in_buffer_idx;
    uint32_t nb_bytes_to_write;
    uint32_t target_addr;
    uint32_t current_sector;
    uint32_t size_left;

    /* make sure current_sector is not the same as starting sector */
    current_sector = (address + NB_BYTES_PER_SECTOR) & BLOCK_ALIGN_MASK_4K;

#if SF2BL_SPI_PORT == SF2BL_CORE_SPI
    SPI_set_slave_select(SPI_INSTANCE, SPI_SLAVE);
#else
    MSS_SPI_set_slave_select(SPI_INSTANCE, SPI_SLAVE);
#endif
    if(0 != wait_ready(FLASH_TIMEOUT_MISC))
    {
        return_val = SPI_FLASH_TIMEOUT;
    }

    in_buffer_idx     = 0;
    nb_bytes_to_write = size_in_bytes;
    target_addr       = address;

    while((in_buffer_idx < size_in_bytes) && (SPI_FLASH_SUCCESS == return_val))
    {
        /* See if we have moved to a new sector and need to format it */
        if((current_sector != (target_addr & BLOCK_ALIGN_MASK_4K)) &&
           (0 != format))
        {
			/* Select block start by masking off non significant bits */
			 current_sector = target_addr & BLOCK_ALIGN_MASK_4K;

#if (SF2BL_FLASH_DEVICE == SF2BL_FLASH_DEV_N25Q00AA13GSF40G)
			return_val = spi_flash_4_byte_addr(ENABLE_4_BYTE_ADDR);
			if(SPI_FLASH_SUCCESS == return_val)
			{
				/* Send Write Enable command */
				cmd_buffer[0] = WRITE_ENABLE_CMD;
				IRQS_OFF
				SPI_TRANS_BLOCK(SPI_INSTANCE, cmd_buffer, 1, 0, 0);
				IRQS_ON

				if(0 != wait_ready(FLASH_TIMEOUT_MISC))
				{
					return_val = SPI_FLASH_TIMEOUT;
				}
				else
				{
					/* Send 4K Sector Erase command */
					cmd_len = spi_flash_insert_cmd_addr(cmd_buffer, ERASE_4K_BLOCK_OPCODE, current_sector);

					IRQS_OFF
					SPI_TRANS_BLOCK(SPI_INSTANCE, cmd_buffer, cmd_len, 0, 0);
					IRQS_ON

					if(0 != wait_ready(FLASH_TIMEOUT_ERASE_4K))
					{
						return_val = SPI_FLASH_TIMEOUT;
					}
				}
			}

			if(SPI_FLASH_SUCCESS == return_val)
			{
				return_val = spi_flash_4_byte_addr(DISABLE_4_BYTE_ADDR);
			}
#else
#if (SF2BL_FLASH_DEVICE == SF2BL_FLASH_DEV_AT25DF641)
			/* Send Write Enable command */
			cmd_buffer[0] = WRITE_ENABLE_CMD;
			IRQS_OFF
			SPI_TRANS_BLOCK(SPI_INSTANCE, cmd_buffer, 1, 0, 0);
			IRQS_ON

			if(0 != wait_ready(FLASH_TIMEOUT_MISC))
			{
				return_val = SPI_FLASH_TIMEOUT;
			}
			else
			{
				/* Send 4K Sector unprotect command */
				cmd_len = spi_flash_insert_cmd_addr(cmd_buffer, UNPROTECT_SECTOR_OPCODE, current_sector);

				IRQS_OFF
				SPI_TRANS_BLOCK(SPI_INSTANCE, cmd_buffer, cmd_len, 0, 0);
				IRQS_ON

				if(0 != wait_ready(FLASH_TIMEOUT_MISC))
				{
					return_val = SPI_FLASH_TIMEOUT;
				}
			}

			if(SPI_FLASH_SUCCESS == return_val)
			{
#endif /* (SF2BL_FLASH_DEVICE == SF2BL_FLASH_DEV_AT25DF641) */
			/* Send Write Enable command */
			cmd_buffer[0] = WRITE_ENABLE_CMD;
			IRQS_OFF
			SPI_TRANS_BLOCK(SPI_INSTANCE, cmd_buffer, 1, 0, 0);
			IRQS_ON

			if(0 != wait_ready(FLASH_TIMEOUT_MISC))
			{
				return_val = SPI_FLASH_TIMEOUT;
			}
			else
			{
				/* Send 4K Sector Erase command */
				cmd_len = spi_flash_insert_cmd_addr(cmd_buffer, ERASE_4K_BLOCK_OPCODE, current_sector);

				IRQS_OFF
				SPI_TRANS_BLOCK(SPI_INSTANCE, cmd_buffer, cmd_len, 0, 0);
				IRQS_ON

				if(0 != wait_ready(FLASH_TIMEOUT_ERASE_4K))
				{
					return_val = SPI_FLASH_TIMEOUT;
				}
			}
#if (SF2BL_FLASH_DEVICE == SF2BL_FLASH_DEV_AT25DF641)
			}
#endif
#endif
        }

        if(SPI_FLASH_SUCCESS == return_val)
        {
            /*
             * When writing to the FLASH device there are a number of rules we
             * must enforce:
             *
             *  1. We cannot cross a page boundary which means we have to write
             *     in chunks of no more than a page in length.
             *
             *  2. If we are not currently on a page boundary, we can only write
             *     up to the end of the current page as otherwise we will wrap
             *     around to the beginning of the page.
             *
             * To do this, we ensure we write the first data only up to the end
             * of the current page, write in full page chunks after that and
             * finally write a possibly less than full page last chunk.
             */

            /*
             * If we are not on a page boundary this will set nb_bytes_to_write
             * to the number of bytes required to get us to the end of the
             * current page. Otherwise it will set nb_bytes_to_write to the page
             * length
             */
            nb_bytes_to_write = NB_BYTES_PER_PAGE - (target_addr & (NB_BYTES_PER_PAGE - 1));

            /* Truncate if actually less than nb_bytes_to_write left to go */
            size_left = size_in_bytes - in_buffer_idx;
            if(size_left < nb_bytes_to_write)
            {
                nb_bytes_to_write = size_left;
            }

#if (SF2BL_FLASH_DEVICE == SF2BL_FLASH_DEV_N25Q00AA13GSF40G)
			return_val = spi_flash_4_byte_addr(ENABLE_4_BYTE_ADDR);
			if(SPI_FLASH_SUCCESS == return_val)
			{
				/* Send Write Enable command */
				cmd_buffer[0] = WRITE_ENABLE_CMD;
				IRQS_OFF
				SPI_TRANS_BLOCK( SPI_INSTANCE, cmd_buffer, 1, 0, 0 );
				IRQS_ON

				if(0 != wait_ready(FLASH_TIMEOUT_MISC))
				{
					return_val = SPI_FLASH_TIMEOUT;
				}
			}
            if(SPI_FLASH_SUCCESS == return_val)
            {
             	cmd_len = spi_flash_insert_cmd_addr(cmd_buffer, PROGRAM_PAGE_CMD, target_addr);

                write_cmd_data(SPI_INSTANCE, cmd_buffer, cmd_len,
                               &write_buffer[in_buffer_idx],
                               nb_bytes_to_write);

                if(0 != wait_ready(FLASH_TIMEOUT_WR_PAGE))
                {
                    return_val = SPI_FLASH_TIMEOUT;
                }
                else
                {
        			return_val = spi_flash_4_byte_addr(DISABLE_4_BYTE_ADDR);
                }
#if defined(SF2BL_FLASH_WRITE_VERIFY)
                if(SPI_FLASH_SUCCESS == return_val)
                {
                    /* Read back what we just wrote and see if it is the same */
                    return_val = spi_flash_read(target_addr, verify_buffer, nb_bytes_to_write);
                    if(SPI_FLASH_SUCCESS == return_val)
                    {
                        if(memcmp(verify_buffer, &write_buffer[in_buffer_idx], nb_bytes_to_write))
                        {
                            return_val = SPI_FLASH_VERIFY_FAIL;
                        }

                        /* We need to do this as spi_flash_read() clears it */
#if SF2BL_SPI_PORT == SF2BL_CORE_SPI
                        SPI_set_slave_select(SPI_INSTANCE, SPI_SLAVE);
#else
                        MSS_SPI_set_slave_select(SPI_INSTANCE, SPI_SLAVE);
#endif
                    }
                }
#endif
                target_addr   += nb_bytes_to_write;
                in_buffer_idx += nb_bytes_to_write;
            }
#else
            /* Send Write Enable command */
            cmd_buffer[0] = WRITE_ENABLE_CMD;
            IRQS_OFF
            SPI_TRANS_BLOCK( SPI_INSTANCE, cmd_buffer, 1, 0, 0 );
            IRQS_ON

            if(0 != wait_ready(FLASH_TIMEOUT_MISC))
            {
                return_val = SPI_FLASH_TIMEOUT;
            }

            if(SPI_FLASH_SUCCESS == return_val)
            {
             	cmd_len = spi_flash_insert_cmd_addr(cmd_buffer, PROGRAM_PAGE_CMD, target_addr);

                write_cmd_data(SPI_INSTANCE, cmd_buffer, cmd_len,
                               &write_buffer[in_buffer_idx],
                               nb_bytes_to_write);

                if(0 != wait_ready(FLASH_TIMEOUT_WR_PAGE))
                {
                    return_val = SPI_FLASH_TIMEOUT;
                }
#if defined(SF2BL_FLASH_WRITE_VERIFY)
                else
                {
                    /* Read back what we just wrote and see if it is the same */
                    return_val = spi_flash_read(target_addr, verify_buffer, nb_bytes_to_write);
                    if(SPI_FLASH_SUCCESS == return_val)
                    {
                        if(memcmp(verify_buffer, &write_buffer[in_buffer_idx], nb_bytes_to_write))
                        {
                            return_val = SPI_FLASH_VERIFY_FAIL;
                        }

                    /* We need to do this as spi_flash_read() clears it */
#if SF2BL_SPI_PORT == SF2BL_CORE_SPI
                    SPI_set_slave_select(SPI_INSTANCE, SPI_SLAVE);
#else
                    MSS_SPI_set_slave_select(SPI_INSTANCE, SPI_SLAVE);
#endif
                    }
                }
#endif
                target_addr   += nb_bytes_to_write;
                in_buffer_idx += nb_bytes_to_write;
            }
#endif
        }
    }

#if SF2BL_SPI_PORT == SF2BL_CORE_SPI
    SPI_clear_slave_select(SPI_INSTANCE, SPI_SLAVE);
#else
    MSS_SPI_clear_slave_select(SPI_INSTANCE, SPI_SLAVE);
#endif

    return(return_val);
}


/******************************************************************************
 * This function waits for the FLASH operation to complete.
 *
 * Returns 0 if success or READY_BIT_MASK if we timeout without the ready bit
 * being cleared.
 ******************************************************************************/
static uint8_t wait_ready(uint32_t timeout)
{
    uint8_t ready_bit;
    volatile int error = 0;
#if (SF2BL_FLASH_DEVICE == SF2BL_FLASH_DEV_N25Q00AA13GSF40G)
    uint8_t command = READ_FLAG_STATUS_REG;
#else
    uint8_t command = READ_STATUS;
#endif
    uint32_t start_time;

    start_time = g_10ms_count; /* Record when it all began */

    do {
        IRQS_OFF
        SPI_TRANS_BLOCK(SPI_INSTANCE, &command, 1, &ready_bit, 1);
        IRQS_ON
#if (SF2BL_FLASH_DEVICE == SF2BL_FLASH_DEV_N25Q00AA13GSF40G)
        ready_bit &= FSR_WRITE_BUSY;
    } while((0 == ready_bit) && ((g_10ms_count - start_time) < timeout));
#else
        ready_bit &= READY_BIT_MASK;
        error++;
    } while((0 != ready_bit) && ((g_10ms_count - start_time) < timeout));
#endif

#if (SF2BL_FLASH_DEVICE == SF2BL_FLASH_DEV_N25Q00AA13GSF40G)
    command = CLEAR_FLAG_STATUS_REG;
    IRQS_OFF
    SPI_TRANS_BLOCK(SPI_INSTANCE, &command, 1, 0, 0);
    IRQS_ON

    if(0 == ready_bit)
    	error++;
    return (!ready_bit);
#else
    if(0 != ready_bit)
    	error++;
    return (ready_bit);
#endif
}

#if (SF2BL_FLASH_DEVICE == SF2BL_FLASH_DEV_N25Q00AA13GSF40G)
/******************************************************************************
 * This function implements the preamble/postamble for 4 byte addressing with
 * the N25Q00AA SPI FLASH. If 4 byte addressing is enabled in the non volatile
 * configuration it does nothing.
 *
 * Passing enable == 0 disables 4 byte addressing. Any other value enables 4
 * byte addressing.
 *
 * Returns status of any flash commands executed.
 ******************************************************************************/
static spi_flash_status_t spi_flash_4_byte_addr(int32_t enable)
{
    spi_flash_status_t return_val = SPI_FLASH_SUCCESS;
    uint8_t cmd_buffer;

    if(0 == g_4_byte_addr_enabled)
    {
        /* Check for completion of any prior command */
        if(0 != wait_ready(FLASH_TIMEOUT_MISC + 2000))
        {
            return_val = SPI_FLASH_TIMEOUT;
        }
        else
        {
             /* Send Write Enable command */
            cmd_buffer = WRITE_ENABLE_CMD;
            IRQS_OFF
            SPI_TRANS_BLOCK(SPI_INSTANCE, &cmd_buffer, 1, 0, 0);
            IRQS_ON
            if(0 != wait_ready(FLASH_TIMEOUT_MISC))
            {
                return_val = SPI_FLASH_TIMEOUT;
            }
            else
            {
                /* Safe to send enable/disable 4 byte addressing command now */
                cmd_buffer = enable ? ENTER_4_BYTE_ADR_MODE : EXIT_4_BYTE_ADR_MODE;
                IRQS_OFF
                SPI_TRANS_BLOCK(SPI_INSTANCE, &cmd_buffer, 1, 0, 0);
                IRQS_ON

                if(0 != wait_ready(FLASH_TIMEOUT_MISC))
                {
                    return_val = SPI_FLASH_TIMEOUT;
                }
            }
        }
    }

    return(return_val);
}
#endif

/******************************************************************************
 * This function inserts the command byte and address field for an SPI FLASH
 * command at the location specified by dest. The address  field will be either
 * 3 or 4 bytes long depending on the FLASH device.
 *
 * Returns the number of bytes inserted.
 ******************************************************************************/

static int32_t spi_flash_insert_cmd_addr(uint8_t *dest, uint32_t command, uint32_t target_addr)
{
    *dest++ = (uint8_t)command;
#if (SF2BL_FLASH_DEVICE == SF2BL_FLASH_DEV_N25Q00AA13GSF40G)
    *dest++ = (uint8_t)(target_addr >> 24);
#endif
    *dest++ = (uint8_t)(target_addr >> 16);
    *dest++ = (uint8_t)(target_addr >> 8);
    *dest++ = (uint8_t)target_addr;

#if (SF2BL_FLASH_DEVICE == SF2BL_FLASH_DEV_N25Q00AA13GSF40G)
    return(5);
#else
    return(4);
#endif

}
