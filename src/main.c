/*******************************************************************************
 * (c) Copyright 2015 Microsemi SoC Products Group.  All rights reserved.
 *
 * SmartFusion2 Bootloader main application logic.
 *
 * SVN $Revision: 8451 $
 * SVN $Date: 2016-06-27 06:30:14 +0100 (Mon, 27 Jun 2016) $
 */
#include <stdint.h>
#include <ctype.h>
#include <string.h>

#include "sf2_bl_options.h"
#if !defined(RISCV_PLATFORM)
#include "m2sxxx.h"
#endif
#include "drivers_config/sys_config/sys_config.h"
#include "ymodem.h"

#include "spi_flash.h"
#include "exec.h"
#include "intel_hex.h"
#include "sf2_bl_defs.h"
#include "image_tools.h"

//#include "mss_watchdog.h"
//#include "mss_gpio.h"
#if defined(SF2BL_USE_COREGPIO)
#include "core_gpio.h"
#endif

#include "hw_platform.h"

void interrupts_init(void);

volatile uint32_t g_10ms_count;

uint8_t *g_rx_base;
uint8_t *g_bin_base;
uint32_t g_rx_size;
uint32_t g_n_images = 0;

sf2bl_boot_mode_t g_boot_mode = SF2BL_BOOT_EXEC;

/* Start off with all drivers uninitialised */
uint32_t g_driver_init = 0;

/*
 * CoreGPIO instance data.
 */
#if defined(SF2BL_USE_COREGPIO)
gpio_instance_t g_gpio;
#endif

/***************************************************************************//**
 * Examine the images and the boot mode to determine exactly what to do next.
 */

void sf2bl_select_image(void)
{
    uint16_t temp;
    uint32_t winner = 0; /* This is the one we decide is active if there are problems */
    uint32_t valid;
    spi_flash_status_t flash_result;

    sf2bl_check_flash(); /* Examine current state of FLASH images */
#if defined(SF2BL_GOLDEN)
    if(SF2BL_HDR_OK != g_golden_img_status) /* Something wrong with golden image */
    {
        /*
         * Force update of golden image. This will also trigger an update of the
         * primary image as we normally boot from the image(s) and not the
         * golden image.
         */
        g_boot_mode = SF2BL_BOOT_DOWNLOAD_GOLDEN;
        g_current_sequence = 0;
    }
    else if((SF2BL_BOOT_EXEC_GOLDEN != g_boot_mode) && (SF2BL_BOOT_COPY_GOLDEN != g_boot_mode))
    {
#endif
#if defined(SF2BL_2ND_IMAGE)
        /* both normal images out of action so force download of first */
        if((SF2BL_HDR_OK != g_img1_status) && (SF2BL_HDR_OK != g_img2_status))
        {
            /* Force update of image */
            g_boot_mode = SF2BL_BOOT_DOWNLOAD_1;
            g_current_sequence = 0;
        }
        else if((SF2BL_HDR_OK == g_img1_status) && (SF2BL_HDR_OK == g_img2_status))
        {
            /*
             * Both images good? Shouldn't happen but if it does we want the one
             * with the newest sequence number to be the victor.
             *
             * Note: the sequence numbers should always have Image 1 odd, Image
             * 2 even and the only case where image 1 should be more than image
             * 2 is for 65535, 0.
             *
             * If the current image 2 sequence is not equal to the current image
             * 1 sequence plus 1 (with 16 bit roll over) then we check the
             * opposite case and finally make decision the hard way if needs be.
             */
            temp = g_img1_header.sequence;
            temp++;
            if(temp != g_img2_header.sequence)
            {
                temp = g_img2_header.sequence;
                temp++;
                if(temp == g_img1_header.sequence) /* Image 1 is good to go */
                {
                    winner = 1;
                }
                else
                {
                    /*
                     * Have to pick one or the other...
                     * Since the sequence numbers are no longer in synch, all we
                     * can do is pick the highest numbered, non zero one or if
                     * somehow they are the same we pick image 1.
                     */
                    if((g_img2_header.sequence == g_img1_header.sequence) ||
                       ((g_img2_header.sequence < g_img1_header.sequence) && (0 != g_img2_header.sequence)))
                    {
                        winner = 1; /* Image 1 is good to go */
                    }
                    else /* Image 2 is good to go */
                    {
                        winner = 2;
                    }
                }
            }
            else /* Image 2 is good to go */
            {
                winner = 2;
            }
            /*
             * Invalidate the loser before deciding what to do next so we don't
             * have to do this all over again...
             */
            valid = SF2BL_IMG_HDR_INVALID;
            if(1 == winner)
            {
                g_current_sequence = g_img1_header.sequence;
                /* We ignore spi_flash_write() return here as it hopefully will
                 * not impede our progress if there was a fail. */
                flash_result = spi_flash_write(g_img2_offset, (uint8_t *)&valid, sizeof(valid), 0);
                if((SF2BL_BOOT_DOWNLOAD == g_boot_mode))
                {
                    g_boot_mode = SF2BL_BOOT_DOWNLOAD_2;
                }
                else
                {
                    g_boot_mode = SF2BL_BOOT_EXEC_1;
                }
            }
            else
            {
                g_current_sequence = g_img2_header.sequence;
                /* We ignore spi_flash_write() return here as it hopefully will
                 * not impede our progress if there was a fail. */
                flash_result = spi_flash_write(g_img1_offset, (uint8_t *)&valid, sizeof(valid), 0);
                if((SF2BL_BOOT_DOWNLOAD == g_boot_mode))
                {
                    g_boot_mode = SF2BL_BOOT_DOWNLOAD_1;
                }
                else
                {
                    g_boot_mode = SF2BL_BOOT_EXEC_2;
                }
            }
        }
        else if((SF2BL_HDR_OK == g_img1_status))
        {
            /* Image 1 is ok so either execute it or update image 2 */
            g_current_sequence = g_img1_header.sequence;
            if((SF2BL_BOOT_DOWNLOAD == g_boot_mode))
            {
                g_boot_mode = SF2BL_BOOT_DOWNLOAD_2;
            }
            else
            {
                g_boot_mode = SF2BL_BOOT_EXEC_1;
            }
        }
        else
        {
            /* Image 2 is ok so either execute it or update image 1 */
            g_current_sequence = g_img2_header.sequence;
            if((SF2BL_BOOT_DOWNLOAD == g_boot_mode))
            {
                g_boot_mode = SF2BL_BOOT_DOWNLOAD_1;
            }
            else
            {
                g_boot_mode = SF2BL_BOOT_EXEC_2;
            }

        }
#else /* not SF2BL_SECOND_IMAGE */
        /* Something wrong with only normal image or download selected and there is only one choice... */
        if(SF2BL_HDR_OK != g_img1_status)
        {
            /* Force update of image */
            g_current_sequence = 0;
            g_boot_mode = SF2BL_BOOT_DOWNLOAD_1;
        }
        else if(SF2BL_BOOT_DOWNLOAD == g_boot_mode)
        {
            /* Normal update of image */
            g_current_sequence = g_img1_header.sequence;
            g_boot_mode = SF2BL_BOOT_DOWNLOAD_1;
        }
        else
        {
            /* Lets try and execute normal image */
            g_boot_mode = SF2BL_BOOT_EXEC_1;
        }
#endif
#if defined(SF2BL_GOLDEN)
    }
#endif
}

/***************************************************************************//**
 *
 */

void sf2bl_check_for_update(void)
{
    int32_t  update_flag      = 0;
#if defined(SF2BL_GOLDEN)
    int32_t  use_golden_flag  = 0;
    int32_t  copy_golden_flag = 0;
#endif

    /*
     * The order of precedence in the following in decreasing priority is:
     *
     *   1 Copy Golden
     *   2 Use Golden
     *   3 Update
     *   4 Execute
     */

#if SF2BL_NO_PIN != SF2BL_USER_UPDATE_PIN
#if defined(SF2BL_USE_COREGPIO)
    /* We assume that the CoreGPIO I/Os are configured as part of hardware
     * design flow. No need to configuration through CoreGPIO driver.*/
    update_flag = (GPIO_get_inputs(&g_gpio) >> SF2BL_USER_UPDATE_PIN) & 1;
#else
    MSS_GPIO_config(SF2BL_USER_UPDATE_PIN, MSS_GPIO_INPUT_MODE);
    update_flag = (MSS_GPIO_get_inputs() >> SF2BL_USER_UPDATE_PIN) & 1;
#endif
#endif

#if defined(SF2BL_USER_HOOK_UPDATE)
    /* OR the flag with function return in case already set by pin check */
       update_flag |= SF2BL_USER_HOOK_UPDATE();
#endif

#if (defined(SF2BL_GOLDEN)) && (SF2BL_NO_PIN != SF2BL_USER_USE_GOLDEN_PIN)
#if defined(SF2BL_USE_COREGPIO)
    use_golden_flag = (GPIO_get_inputs(&g_gpio) >> SF2BL_USER_USE_GOLDEN_PIN) & 1;
#else
    MSS_GPIO_config(SF2BL_USER_USE_GOLDEN_PIN, MSS_GPIO_INPUT_MODE);
    use_golden_flag = (MSS_GPIO_get_inputs() >> SF2BL_USER_USE_GOLDEN_PIN) & 1;
#endif
#endif

#if defined(SF2BL_USER_HOOK_USE_GOLDEN)
    /* OR the flag with function return in case already set by pin check */
       use_golden_flag |== SF2BL_USER_HOOK_USE_GOLDEN();
#endif


#if defined(SF2BL_GOLDEN) && (SF2BL_NO_PIN != SF2BL_USER_COPY_GOLDEN_PIN)
#if defined(SF2BL_USE_COREGPIO)
    copy_golden_flag = (GPIO_get_inputs(&g_gpio) >> SF2BL_USER_COPY_GOLDEN_PIN) & 1;
#else
    MSS_GPIO_config(SF2BL_USER_COPY_GOLDEN_PIN, MSS_GPIO_INPUT_MODE);
    copy_golden_flag = (MSS_GPIO_get_inputs() >> SF2BL_USER_COPY_GOLDEN_PIN) & 1;
#endif
#endif

#if defined(SF2BL_USER_HOOK_COPY_GOLDEN)
    /* OR the flag with function return in case already set by pin check */
    copy_golden_flag |= SF2BL_USER_HOOK_UPDATE();
#endif

/* Enforce priority and decide what to do next */
#if defined(SF2BL_GOLDEN)
if(copy_golden_flag)
    {

    g_boot_mode     = SF2BL_BOOT_EXEC_GOLDEN;
    }
else if(use_golden_flag)
    {
        g_boot_mode = SF2BL_BOOT_COPY_GOLDEN;
    }
else if(update_flag)
    {
        g_boot_mode = SF2BL_BOOT_DOWNLOAD;
    }
else
    {
        g_boot_mode = SF2BL_BOOT_EXEC;
    }

#else
if(update_flag)
    {
        g_boot_mode = SF2BL_BOOT_DOWNLOAD;
    }
else
    {
        g_boot_mode = SF2BL_BOOT_EXEC;
    }
#endif
}

/***************************************************************************//**
 *
 */

int main()
{
    uint32_t received;
    uint32_t processed;
    int32_t  temp;
    uint32_t valid;
    img_hdr_block_t *pheader;
    spi_flash_status_t flash_result;
    int32_t image_status; /* 0 - ok, -1 not ok */

#if !defined(RISCV_PLATFORM)
#if defined(NDEBUG)
    SYSREG->FLUSH_CR = 1u;
    SYSREG->CC_CR = 1u;
#else
    SYSREG->CC_CR = 0u;
    SYSREG->FLUSH_CR = 1u;
#endif
#endif

    image_status = -1; /* assume the worst for now */
    g_10ms_count = 0;
#if defined(RISCV_PLATFORM)
    /*<CJ>TODO */
    interrupts_init();
#else
    /*-------------------------------------------------------------------------
     * Initialize the system tick for 10mS operation or 1 tick every 100th of
     * a second and also make sure it is lowest priority interrupt.
    */
    NVIC_SetPriority(SysTick_IRQn, 0xFFu); /* Lowest possible priority */
    /*
     * If the systick 10mS calibration factor is set correctly in your design
     * you can use the following:
     * SysTick_Config((SysTick->CALIB));
     */
    SysTick_Config(MSS_SYS_M3_CLK_FREQ / 100);

    g_driver_init |= SF2BL_DRIVER_SYSTICK;

    MSS_WD_init();
    g_driver_init |= SF2BL_DRIVER_MSS_WD;
#endif
#if -5 != (SF2BL_SPI_WP + SF2BL_SPI_RESET + SF2BL_USER_UPDATE_PIN + SF2BL_USER_USE_GOLDEN_PIN + SF2BL_USER_COPY_GOLDEN_PIN)
#if defined(SF2BL_USE_COREGPIO)
    GPIO_init(&g_gpio, COREGPIO_IN_BASE_ADDR, GPIO_APB_32_BITS_BUS);
#else
    MSS_GPIO_init();
#endif
    g_driver_init |= SF2BL_DRIVER_MSS_GPIO;
#endif

    spi_flash_init();
    g_rx_base = (uint8_t *)SF2BL_DDR_BASE;
    g_rx_size = (SF2BL_DDR_SIZE / 3) & 0xFFFFFFFC;
    g_rx_size *= 2;
    g_bin_base = (uint8_t *)(SF2BL_DDR_BASE + g_rx_size);

    sf2bl_check_for_update(); /* See what we are expected to do */
    sf2bl_select_image();     /* Select image to work with */

#if defined(SF2BL_YMODEM_VERBOSE) || defined(SF2BL_VERBOSE)
    sf2bl_ymodem_init();
    _putstring((uint8_t *)SF2BL_VERSION_STRING);
    _putstring((uint8_t *)"\r\nRunning on ");
    _putstring((uint8_t *)SF2BL_PLATFORM_STRING);
    _putstring((uint8_t *)"\r\n");
#endif
    if((SF2BL_BOOT_DOWNLOAD_1      == g_boot_mode) ||
       (SF2BL_BOOT_DOWNLOAD_2      == g_boot_mode) ||
       (SF2BL_BOOT_DOWNLOAD_GOLDEN == g_boot_mode))
    {
#if !defined(SF2BL_YMODEM_VERBOSE) /* Only do this here if not already */
        sf2bl_ymodem_init();
#endif
        processed = 0;
#if defined(SF2BL_YMODEM_VERBOSE)
        _putstring((uint8_t *)"Commencing YMODEM download of ");
        if(SF2BL_BOOT_DOWNLOAD_1 == g_boot_mode)
        {
            _putstring((uint8_t *)"1st image.\r\n");
        }

        else if(SF2BL_BOOT_DOWNLOAD_2 == g_boot_mode)
        {
            _putstring((uint8_t *)"2nd image.\r\n");
        }
        else
        {
            _putstring((uint8_t *)"golden image.\r\n");
        }
        _putstring((uint8_t *)"Please start file transfer now.\r\n");

#endif
        received = ymodem_receive(g_rx_base, g_rx_size);

#if defined(SF2BL_VERBOSE)
        /*
         * ExtraPuTTY seems to have an issue if we start sending messages
         * straight away when the transfer finishes so we delay half a second to
         * let it catch its breath.
         */
        {
            uint32_t delay_time;
            delay_time = g_10ms_count;
            while((g_10ms_count - delay_time) < 500)
                ;
        }
#endif
        if(received > 0)
        {
            SF2BL_MESSAGE("\r\nDownload complete, processing Hex file\r\n")
            processed = sf2bl_process_hex_file(received);
        }
#if defined(SF2BL_VERBOSE)
        else
        {
            SF2BL_MESSAGE("\r\nFile download failed!\r\n")
        }
#endif
        if(0 != processed)
        {
#if defined(SF2BL_GOLDEN)
            if(SF2BL_BOOT_DOWNLOAD_GOLDEN == g_boot_mode)
            {
                /* Write golden image first */
                SF2BL_MESSAGE("Writing golden image to SPI FLASH.\r\n")
                temp = sf2bl_wr_flash_image(g_golden_img_offset, processed);

                /* Then we update image 1 with same image but sequence = 1 */
                pheader = (img_hdr_block_t *)g_bin_base;
                pheader->sequence = 1u;
                /* Set the valid flag temporarily so CRC is correct */
                pheader->valid = SF2BL_IMG_HDR_VALID;
                pheader->crc16 = sf2bl_crc16((uint8_t *)pheader, sizeof(img_hdr_block_t) - 2u);
                /* Reset valid flag until we have finished writing to FLASH */
                pheader->valid = SF2BL_IMG_HDR_BLANK;
                SF2BL_MESSAGE("Writing copy of golden image to image 1 in SPI FLASH.\r\n")
                temp = sf2bl_wr_flash_image(g_img1_offset, processed);
#if defined(SF2BL_2ND_IMAGE)
                /* Finally invalidate the 2nd image just in case it was used */
                valid = SF2BL_IMG_HDR_INVALID;
                SF2BL_MESSAGE("Invalidating image 2.\r\n")
                flash_result = spi_flash_write(g_img2_offset, (uint8_t *)&valid, sizeof(valid), 0);
                /* Load image 1 into RAM */
                image_status = sf2bl_rd_flash_image(g_img1_offset);
            }
            else if (SF2BL_BOOT_DOWNLOAD_1 == g_boot_mode)
#else
                /* Load image 1 into RAM */
                SF2BL_MESSAGE("loading image 1 into RAM.\r\n")
                image_status = sf2bl_rd_flash_image(g_img1_offset);
            }
            else
#endif

#else
#if defined(SF2BL_2ND_IMAGE)
            if (SF2BL_BOOT_DOWNLOAD_1 == g_boot_mode)
#endif
#endif
            {
                /* Write to first image */
                SF2BL_MESSAGE("Writing image 1 to SPI FLASH.\r\n")
                temp = sf2bl_wr_flash_image(g_img1_offset, processed);
#if defined(SF2BL_2ND_IMAGE)
                /* Invalidate the 2nd image if it exists */
                valid = SF2BL_IMG_HDR_INVALID;
                SF2BL_MESSAGE("Invalidating image 2.\r\n")
                flash_result = spi_flash_write(g_img2_offset, (uint8_t *)&valid, sizeof(valid), 0);
                /* Load image 1 into RAM */
                SF2BL_MESSAGE("loading image 1 into RAM.\r\n")
                image_status = sf2bl_rd_flash_image(g_img1_offset);
#endif
            }
#if defined(SF2BL_2ND_IMAGE)
            else
            {
                /* Write to second image */
                SF2BL_MESSAGE("Writing image 2 to SPI FLASH.\r\n")
                temp = sf2bl_wr_flash_image(g_img2_offset, processed);
                /* Invalidate the 1st image */
                valid = SF2BL_IMG_HDR_INVALID;
                SF2BL_MESSAGE("Invalidating image 1.\r\n")
                flash_result = spi_flash_write(g_img1_offset, (uint8_t *)&valid, sizeof(valid), 0);
                /* Load image 2 into RAM */
                SF2BL_MESSAGE("loading image 2 into RAM.\r\n")
                image_status = sf2bl_rd_flash_image(g_img2_offset);
            }
#endif
        }
#if defined(SF2BL_VERBOSE)
        else if(received > 0) /* Got file but decode failed... */
        {
            SF2BL_MESSAGE("Error processing Hex file!\r\n")
        }
#endif
    }
#if defined(SF2BL_GOLDEN)
    else if(SF2BL_BOOT_EXEC_GOLDEN == g_boot_mode)
    {
        SF2BL_MESSAGE("Loading golden image into RAM.\r\n")
        image_status = sf2bl_rd_flash_image(g_golden_img_offset);
    }
    else if(SF2BL_BOOT_COPY_GOLDEN == g_boot_mode)
    {
        /*
         * Read golden image file and header directly into DDR at binary image
         * build location. Fix up header sequence number, write to image 1 and
         * invalidate image 2. Finally load image 1 into memory ready for
         * execution.
         */

        SF2BL_MESSAGE("Copying golden image to image 1.\r\n")
        processed = sf2bl_raw_rd_flash_image(g_golden_img_offset);
        /* Then we update image 1 with same image but sequence = 1 */
        pheader = (img_hdr_block_t *)g_bin_base;
        pheader->sequence = 1u;
        /* Set the valid flag temporarily so CRC is correct */
        pheader->valid = SF2BL_IMG_HDR_VALID;
        pheader->crc16 = sf2bl_crc16((uint8_t *)pheader, sizeof(img_hdr_block_t) - 2u);
        /* Reset valid flag until we have finished writing to FLASH */
        pheader->valid = SF2BL_IMG_HDR_BLANK;
        temp = sf2bl_wr_flash_image(g_img1_offset, processed);
#if defined(SF2BL_2ND_IMAGE)
        /* Finally invalidate the 2nd image just in case it was used */
        SF2BL_MESSAGE("Invalidating image 2.\r\n")
        valid = SF2BL_IMG_HDR_INVALID;
        flash_result = spi_flash_write(g_img2_offset, (uint8_t *)&valid, sizeof(valid), 0);
        /* Load image 1 into RAM */
        SF2BL_MESSAGE("Loading image 1 into RAM.\r\n")
        image_status = sf2bl_rd_flash_image(g_img1_offset);
#endif
    }
#endif /* defined(SF2BL_GOLDEN)*/
    else /* Must be exec of normal image */
    {
#if defined(SF2BL_2ND_IMAGE)
        if(SF2BL_BOOT_EXEC_1 == g_boot_mode)
        {
            SF2BL_MESSAGE("Loading image 1 into RAM.\r\n")
            image_status = sf2bl_rd_flash_image(g_img1_offset);
        }
        else
        {
            SF2BL_MESSAGE("Loading image 2 into RAM.\r\n")
            image_status = sf2bl_rd_flash_image(g_img2_offset);
        }
#else
        image_status = sf2bl_rd_flash_image(g_img1_offset);
#endif
    }

    if(0 == image_status) /* Get Ready to execute application */
    {
        spi_flash_deinit();
#if defined(SF2BL_VERBOSE)
        SF2BL_MESSAGE("Commencing application execution.\r\n")
//<CJ>TODO:        _put_wait(); /* Wait for completion before shutting down UART */
#endif
        sf2bl_ymodem_deinit();

#if (SF2BL_NO_PIN != SF2BL_SPI_WP) || (SF2BL_NO_PIN != SF2BL_SPI_RESET) || (SF2BL_NO_PIN != SF2BL_USER_UPDATE_PIN)
        /*
         * MSS_GPIO_init() resets the MSS GPIO hardware and disables all interrupts.
         * This means a separate deinit is not needed.
         */
#if defined(SF2BL_USE_COREGPIO)
    GPIO_init(&g_gpio, COREUARTAPB0_BASE_ADDR, GPIO_APB_32_BITS_BUS);
#else
        MSS_GPIO_init();
#endif
        g_driver_init &= ~SF2BL_DRIVER_MSS_GPIO;
#endif
//<CJ>TODO        SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk; /* Disable SysTick */
        /*
         * Finally, we get to execute the users application, this is the point of no
         * return.
         */
        {
            sf2bl_exec();
        }
    }

    /* We should never make it this far... */
    SF2BL_MESSAGE("No valid application image available.\r\n")
    SF2BL_MESSAGE("Bootloader halting.\r\n")
    for(;;)
    {
    }
}


/*------------------------------------------------------------------------------
 * Count the number of elapsed milliseconds (SysTick_Handler is called every
 * 10mS so the resolution will be 10ms). Rolls over every 49 days or so...
 *
 * Should be safe to read g_10ms_count from elsewhere.
 */
void SysTick_Handler(void)
{
    g_10ms_count += 10;

     /*
      * For neatness, if we roll over, reset cleanly back to 0 so the count
      * always goes up in proper 10s.
      */
    if(g_10ms_count < 10)
        g_10ms_count = 0;
}

