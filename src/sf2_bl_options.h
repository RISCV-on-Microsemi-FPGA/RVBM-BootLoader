/*******************************************************************************
 * (c) Copyright 2015 Microsemi SoC Products Group.  All rights reserved.
 *
 * SmartFusion2 Bootloader central configuration file.
 *
 * SVN $Revision: 8449 $
 * SVN $Date: 2016-06-26 15:15:12 +0100 (Sun, 26 Jun 2016) $
 */

/*
 * First pull in the users selection of options before processing the
 * configuration. This avoids having to edit project properties every time you
 * need to change the bootloader configuration.
 */
#include "sf2_bl_user_opts.h"

#ifndef SF2_BL_OPTIONS_H_
#define SF2_BL_OPTIONS_H_

/* SF2BL_GOLDEN
 *
 * Define this macro to enable storage of a non-modifiable fall back firmware
 * image. When this feature is enabled, the first firmware image written to a
 * system with a blank or empty SPI FLASH device will be written to the area
 * reserved for the golden image. This image will not be alterable by the
 *  Bootloader once it is created.
 *
 *  #define SF2BL_GOLDEN
 */

/* SF2BL_2ND_IMAGE
 *
 * Define this macro to enable storage of 2 separate firmware images in a
 * “ping pong” fashion in the SPI FLASH device. This reduces the chance of an
 * update failure leaving the system in a non-operational state as the current
 * firmware image is not modified when an update is performed. Depending on
 * this macro and the SF2BL_GOLDEN macro, there will be space for 1, 2 or 3
 * firmware images reserved in the SPI FLASH device.
 *
 * #define SF2BL_2ND_IMAGE
 */

/* SF2BL_USE_CRC
 *
 * Define this macro to use a 32bit CRC for image integrity checking.
 */

#define SF2BL_USE_CRC

/* SF2BL_USE_MD5
 *
 * Define this macro to use an MD5 hash for image integrity checking.
 *
 * #define SF2BL_USE_MD5
 */

#if defined(SF2BL_USE_CRC) && defined(SF2BL_USE_MD5)
#error "Only one of SF2BL_USE_CRC or SF2BL_USE_MD5 can be defined."
#endif

/* SF2BL_SPI_PORT
 *
 * This macro takes the following values to indicate the SPI peripheral to use
 * (default is 0):
 *
 * 0 - MSS SPI 0
 * 1 - MSS SPI 1
 * 2 - CoreSPI
 *
 * Macro definitions for the port numbers are provided below.
 */

#define SF2BL_MSS_SPI0 0
#define SF2BL_MSS_SPI1 1
#define SF2BL_CORE_SPI 2

#if !defined(SF2BL_SPI_PORT)
#define SF2BL_SPI_PORT SF2BL_MSS_SPI0
#endif

#if (SF2BL_SPI_PORT != SF2BL_MSS_SPI0) && (SF2BL_SPI_PORT != SF2BL_MSS_SPI1) && (SF2BL_SPI_PORT != SF2BL_CORE_SPI)
#error "Invalid SF2BL_SPI_PORT value"
#endif

/* SF2BL_SPI_WP
 *
 * GPIO port to use for SPI FLASH write protect pin. -1 indicates no write
 * protect pin used.
 */

#define SF2BL_NO_PIN -1

#if !defined(SF2BL_SPI_WP)
#define SF2BL_SPI_WP SF2BL_NO_PIN
#endif

/* SF2BL_SPI_RESET
 *
 * GPIO port to use for SPI FLASH reset pin. -1 indicates no write
 * protect pin used.
 */

#if !defined(SF2BL_SPI_RESET)
#define SF2BL_SPI_RESET SF2BL_NO_PIN
#endif

/*
 * SF2BL_FLASH_WRITE_VERIFY
 *
 * Define this macro to have the Bootloader verify every FLASH write operation
 * by reading back the data and comparing with the original.
 *
 * #define SF2BL_FLASH_WRITE_VERIFY
 */

/* SF2BL_FLASH_USE_DMA
 *
 * Define this macro to enable PDMA operation for SPI FLASH access.
 *
 * #define SF2BL_FLASH_USE_DMA
 */

/* SF2BL_ FLASH_RX_DMA_CHAN
 *
 * Sets the PDMA channel to use for SPI receive data. Default is 0.
 */

#if !defined(SF2BL_FLASH_RX_DMA_CHAN)
#define SF2BL_FLASH_RX_DMA_CHAN 0
#endif

/* SF2BL_ FLASH_TX_DMA_CHAN
 *
 * Sets the PDMA channel to use for SPI transmit data. Default is 1.
 */

#if !defined(SF2BL_FLASH_TX_DMA_CHAN)
#define SF2BL_FLASH_TX_DMA_CHAN 1
#endif

/* SF2BL_FLASH_IMAGE_GRANUALARITY
 *
 * Sets the minimum block size for the firmware image to accommodate the FLASH
 * device sector size. Default is 4096.
 */

#if !defined(SF2BL_FLASH_IMAGE_GRANUALARITY)
#define SF2BL_FLASH_IMAGE_GRANUALARITY 4096
#endif

/* SF2BL_TARGET_SYSTEM
 *
 * This macro can be set to one of the following to select pre-configured SPI
 * FLASH setups based on the standard development boards:
 *
 * SF2BL_TARGET_ADV_DEV_KIT
 * SF2BL_TARGET_EVAL_KIT
 * SF2BL_TARGET_STARTER_KIT
 *
 * The default is SF2BL_TARGET_CUSTOM to indicate non-potted configuration.
 */

#define SF2BL_TARGET_CUSTOM          0
#define SF2BL_TARGET_ADV_DEV_KIT     1
#define SF2BL_TARGET_EVAL_KIT        2
#define SF2BL_TARGET_STARTER_KIT     3
#define SF2BL_TARGET_SEC_EVAL_KIT    4
#define SF2BL_TARGET_DEV_KIT         5

#if !defined(SF2BL_TARGET_SYSTEM)
#define SF2BL_TARGET_SYSTEM SF2BL_TARGET_CUSTOM
#endif

#if SF2BL_TARGET_SYSTEM != SF2BL_TARGET_CUSTOM

/* Erase any existing definitions first */
#undef SF2BL_FLASH_SIZE
#undef SF2BL_FLASH_BASE
#undef SF2BL_LOG_SIZE
#undef SF2BL_CONFIG_SIZE
#undef SF2BL_IMAGE_SIZE
#undef SF2BL_FLASH_IMAGE_GRANUALARITY
#undef SF2BL_DDR_SIZE
#undef SF2BL_FLASH_DEVICE

#if SF2BL_TARGET_SYSTEM == SF2BL_TARGET_ADV_DEV_KIT

#define SF2BL_2ND_IMAGE
#define SF2BL_GOLDEN
#define SF2BL_FLASH_SIZE               134217728
#define SF2BL_FLASH_BASE               0
#define SF2BL_LOG_SIZE                 9
#define SF2BL_CONFIG_SIZE              8
#define SF2BL_FLASH_IMAGE_GRANUALARITY 4096
#define SF2BL_DDR_SIZE                 0x40000000
#define SF2BL_FLASH_DEVICE             SF2BL_FLASH_DEV_N25Q00AA13GSF40G

#elif (SF2BL_TARGET_SYSTEM == SF2BL_TARGET_EVAL_KIT) || (SF2BL_TARGET_SYSTEM == SF2BL_TARGET_SEC_EVAL_KIT)

#define SF2BL_2ND_IMAGE
#undef  SF2BL_GOLDEN
#define SF2BL_FLASH_SIZE               8388608
#define SF2BL_FLASH_BASE               0
#define SF2BL_LOG_SIZE                 8
#define SF2BL_CONFIG_SIZE              8
#define SF2BL_FLASH_IMAGE_GRANUALARITY 4096
#define SF2BL_DDR_SIZE                 0x4000000
#define SF2BL_FLASH_DEVICE             SF2BL_FLASH_DEV_W25Q64FVSSIG

#elif SF2BL_TARGET_SYSTEM == SF2BL_TARGET_STARTER_KIT

#define SF2BL_2ND_IMAGE
#undef  SF2BL_GOLDEN
#define SF2BL_FLASH_SIZE               16777216
#define SF2BL_FLASH_BASE               0
#define SF2BL_LOG_SIZE                 16
#define SF2BL_CONFIG_SIZE              16
#define SF2BL_FLASH_IMAGE_GRANUALARITY 65536
#define SF2BL_DDR_SIZE                 0x4000000
#define SF2BL_FLASH_DEVICE             SF2BL_FLASH_DEV_S25FL128SDPBHICO

#elif SF2BL_TARGET_SYSTEM == SF2BL_TARGET_DEV_KIT

#define SF2BL_2ND_IMAGE
#undef  SF2BL_GOLDEN
#define SF2BL_FLASH_SIZE               8388608
#define SF2BL_FLASH_BASE               0
#define SF2BL_LOG_SIZE                 8
#define SF2BL_CONFIG_SIZE              8
#define SF2BL_FLASH_IMAGE_GRANUALARITY 4096
#define SF2BL_DDR_SIZE                 0x20000000
#define SF2BL_FLASH_DEVICE             SF2BL_FLASH_DEV_AT25DF641

#else
#error "Unrecognised target system"
#endif
#endif

/*
 * SF2BL_FLASH_DEVICE
 *
 * This macro must be set to one of the following to select the type of FLASH
 * device being used:
 *
 * SF2BL_FLASH_DEV_AT25DF641        - SmartFusion2 Dev Kit
 * SF2BL_FLASH_DEV_W25Q64FVSSIG     - SmartFusion2 Eval Kit
 * SF2BL_FLASH_DEV_N25Q00AA13GSF40G - Smartfusion2 Advanced Dev kit
 * SF2BL_FLASH_DEV_S25FL128SDPBHICO - SmartFusion2 Starter Kit
 *
 * If the SmartFusion2 Bootloader is being used with a custom design not using
 * one of the above devices, the user should add a definition for the device in
 * question and modify the SPI FLASH driver to provide support for the new
 * device.
 */

#if !defined(SF2BL_FLASH_DEVICE)
#error "SF2BL_FLASH_DEVICE must be set to an appropriate value"
#endif

/* SF2BL_FLASH_SIZE
 *
 * The number of bytes in total available in the FLASH device. Depending on the
 * SF2BL_GOLDEN and SF2BL_2ND_IMAGE macros, SF2BL_FLASH_SIZE should equal one
 * of the following:
 *
 * SF2BL_FLASH_BASE + SF2BL_FLASH_IMAGE_SIZE + (SF2BL_LOG_SIZE x 4096) + (SF2BL_CONFIG_SIZE x 4096)
 *
 * SF2BL_FLASH_BASE + (SF2BL_FLASH_IMAGE_SIZE x 2) + (SF2BL_LOG_SIZE x 4096) + (SF2BL_CONFIG_SIZE x 4096)
 *
 * SF2BL_FLASH_BASE + (SF2BL_FLASH_IMAGE_SIZE x 3) + (SF2BL_LOG_SIZE x 4096) + (SF2BL_CONFIG_SIZE x 4096)
 *
 * Space can be reserved at the end of the SPI FLASH device by setting this
 * value to less than the available space in the device. The default is 8MB as
 * this is the smallest size on the supported demo platforms.
 */

#if !defined(SF2BL_FLASH_SIZE)
#define SF2BL_FLASH_SIZE 8388608
#endif

/* SF2BL_FLASH_BASE
 *
 * The starting address in the SPI FLASH device of our data. This should take
 * into account any image data required for the fabric image(s) used by the
 * System Controller for IAP, programming recovery etc. It is the users’
 * responsibility to get this figure right as there is no way get the lengths
 * from the directory structure that the System Controller uses at the start of
 * the SPI FLASH and for a blank device there would be nothing to check.
 * Default value is 0.
 *
 * Note for non uniform devices like the Spansion S25FL128SDPBHICO parts used
 * on the EmCraft based SmartFusion2 Starter Kit, the values of
 * SF2BL_FLASH_BASE, SF2BL_LOG_SIZE, SF2BL_CONFIG_SIZE and SF2BL_FLASH_SIZE
 * should be chosen to ensure the image(s) align on sector boundaries.
 */

#if !defined(SF2BL_FLASH_BASE)
#define SF2BL_FLASH_BASE 0
#endif

/* SF2BL_LOG_SIZE
 *
 * The number of 4K blocks allocated for logging purposes. 0 to disable
 * logging, otherwise must be at least 2. Default is 8.
 */

#if !defined(SF2BL_LOG_SIZE)
#define SF2BL_LOG_SIZE 8
#endif

#if (SF2BL_LOG_SIZE == 1)
#error "SF2BL_LOG_SIZE cannot be 1"
#endif

/* SF2BL_CONFIG_SIZE
 *
 * The number of 4K blocks allocated for configuration information. Must be a
 * multiple of 2 and has a minimum value of 2. The Bootloader reserves the
 * first 2,048 bytes of this for its own use and maintains 2 copies of the
 * configuration information in a ping pong fashion. Default is 8.
 */

#if !defined(SF2BL_CONFIG_SIZE)
#define SF2BL_CONFIG_SIZE 8
#endif

#if (SF2BL_CONFIG_SIZE == 0) || ((SF2BL_CONFIG_SIZE % 2) != 0)
#error "Invalid SF2BL_CONFIG_SIZE value, must be non 0 multiple of 2"
#endif

/* SF2BL_IMAGE_SIZE
 *
 * The amount of memory reserved in the SPI FLASH for each image. The default
 * value depends on a number of factors and is calculated at compile time.
 */

#if !defined(SF2BL_IMAGE_SIZE)
 #if defined(SF2BL_GOLDEN)
  #if defined(SF2BL_2ND_IMAGE)
   #define SF2BL_IMAGE_SIZE ((SF2BL_FLASH_SIZE - (SF2BL_FLASH_BASE + (SF2BL_LOG_SIZE * 4096) + (SF2BL_CONFIG_SIZE * 4096))) / 3)
  #else
   #define SF2BL_IMAGE_SIZE ((SF2BL_FLASH_SIZE - (SF2BL_FLASH_BASE + (SF2BL_LOG_SIZE * 4096) + (SF2BL_CONFIG_SIZE * 4096))) / 2)
  #endif
 #else
  #if defined(SF2BL_2ND_IMAGE)
   #define SF2BL_IMAGE_SIZE ((SF2BL_FLASH_SIZE - (SF2BL_FLASH_BASE + (SF2BL_LOG_SIZE * 4096) + (SF2BL_CONFIG_SIZE * 4096))) / 2)
  #else
   #define SF2BL_IMAGE_SIZE (SF2BL_FLASH_SIZE - (SF2BL_FLASH_BASE + (SF2BL_LOG_SIZE * 4096) + (SF2BL_CONFIG_SIZE * 4096)))
  #endif
 #endif
#endif

#if (SF2BL_IMAGE_SIZE % SF2BL_FLASH_IMAGE_GRANUALARITY) != 0
#error "Image size is not a multiple of SF2BL_FLASH_IMAGE_GRANUALARITY"
#endif

/* SF2BL_COMMS_OPTION
 *
 * Set to one of the following to select the firmware update option:
 *
 *   SF2BL_COMMS_SFTP_ETHER_CLIENT
 *   SF2BL_COMMS_SFTP_ETHER_SERVER
 *   SF2BL_COMMS_SFTP_RNDIS_CLIENT
 *   SF2BL_COMMS_SFTP_RNDIS_SERVER
 *   SF2BL_COMMS_YMODEM
 *   SF2BL_COMMS_USB_MSC_HOST
 *   SF2BL_COMMS_USB_MSC_DEVICE
 *
 * The default is SF2BL_COMMS_YMODEM.
 */

#define SF2BL_COMMS_YMODEM            0
#define SF2BL_COMMS_SFTP_ETHER_CLIENT 1
#define SF2BL_COMMS_SFTP_ETHER_SERVER 2
#define SF2BL_COMMS_SFTP_RNDIS_CLIENT 3
#define SF2BL_COMMS_SFTP_RNDIS_SERVER 4
#define SF2BL_COMMS_USB_MSC_HOST      5
#define SF2BL_COMMS_USB_MSC_DEVICE    6

#if !defined(SF2BL_COMMS_OPTION)
#define SF2BL_COMMS_OPTION SF2BL_COMMS_YMODEM
#endif

#if (SF2BL_COMMS_OPTION < SF2BL_COMMS_YMODEM) || (SF2BL_COMMS_OPTION > SF2BL_USB_MSC_DEVICE)
#error "Unrecognised SF2BL_COMMS_OPTION"
#endif

/* SF2BL_USER_USE_GOLDEN_PIN
 *
 * This macro can be set to the number of an MSS GPIO pin to check to determine
 * if the system should boot as normal or only execute the GOLDEN image. This
 * allows a temporary return to a factory default operation or perhaps operation
 * of a special firmware for calibration or configuration etc. The MSS GPIO will
 * be configured as an input and should read 0 for normal operation and 1 for
 * execute GOLDEN mode.
 *
 * #define SF2BL_USER_USE_GOLDEN_PIN 1
 */

/* SF2BL_USER_HOOK_USE_GOLDEN
 *
 * This macro can be set to the name of a user supplied function to be called to
 * determine if the system should boot as normal or execute the GOLDEN image.
 * This allows a temporary return to a factory default operation or perhaps
 * operation of a special firmware for calibration or configuration etc. The
 * function takes no parameters and returns 0 for normal operation and 1 for
 * execute GOLDEN mode.
 *
 * #define SF2BL_USER_HOOK_USE_GOLDEN my_use_golden_check
 */

/* SF2BL_USER_COPY_GOLDEN_PIN
 *
 * This macro can be set to the number of an MSS GPIO pin to check to determine
 * if the system should boot as normal or copy the GOLDEN image to the current
 * image and execute it. This provides support for a restore factory default
 * operation. The MSS GPIO will be configured as an input and should read 0 for
 * normal operation and 1 for copy GOLDEN mode.
 *
 * #define SF2BL_USER_COPY_GOLDEN_PIN 1
 */

/* SF2BL_USER_HOOK_COPY_GOLDEN
 *
 * This macro can be set to the name of a user supplied function to be called to
 * determine if the system should boot as normal or execute the GOLDEN image.
 * The function takes no parameters and returns 0 for normal operation and 1 for
 * copy GOLDEN mode.
 *
 * #define SF2BL_USER_HOOK_COPY_GOLDEN my_copy_golden_check
 */

/*
 * If golden firmware image not supported then make sure any dependent features
 * are also disabled.
 */
#if !defined(SF2BL_GOLDEN)
#undef SF2BL_USER_USE_GOLDEN_PIN
#undef SF2BL_USER_HOOK_USE_GOLDEN
#undef SF2BL_USER_COPY_GOLDEN_PIN
#undef SF2BL_USER_HOOK_COPY_GOLDEN
#endif


/* SF2BL_USER_UPDATE_PIN
 *
 * This macro can be set to the number of an MSS GPIO pin to check to determine
 * if the system should boot as normal or go into update mode. The MSS GPIO will
 * be configured as an input and should read 0 for normal operation and 1 for
 * update mode.
 *
 * #define SF2BL_USER_UPDATE_PIN 1
 */

/* SF2BL_USER_HOOK_UPDATE
 *
 * This macro can be set to the name of a user supplied function to be called to
 * determine if the system should boot as normal or go into update mode. The
 * function takes no parameters and returns 0 for normal operation and 1 for
 * update mode.
 *
 * #define SF2BL_USER_HOOK_UPDATE my_update_check
 */

#if !defined(SF2BL_USER_HOOK_UPDATE) && !defined(SF2BL_USER_UPDATE_PIN)
#error "No update detection method defined"
#endif

#if !defined(SF2BL_USER_UPDATE_PIN)
#define SF2BL_USER_UPDATE_PIN SF2BL_NO_PIN
#endif


/* SF2BL_USER_HOOK_INIT1
 *
 * This macro can be set to the name of a user supplied function to be called
 * early on in the boot process which performs essential system initialisation
 * of fabric based and/or external systems for safe, consistent start up.
 *
 * #define SF2BL_USER_HOOK_INIT1 my_early_init_1
 */

/* SF2BL_USER_HOOK_INIT2
 *
 * This macro can be set to the name of a user supplied function to be called
 * when starting update operations. This separate hook is provided as there
 * may be different user initialization required when a potentially longer
 * period is expected to pass before the user application will start.
 *
 * #define SF2BL_USER_HOOK_INIT2 my_early_init_2
 */

/* SF2BL_USER_HOOK_DEINIT
 *
 * This macro can be set to the name of a user supplied function to be called
 * just prior to the startup of the user application. This hook is provided to
 * allow safe shut down of any user hardware which may have been setup and used
 * by the user hooks in the boot or update process.
 *
 * #define SF2BL_USER_HOOK_DEINIT my_deinit
 */

/* SF2BL_USER_HOOK_PERIODIC1
 *
 * This macro can be set to the name of a user supplied function to be called
 * periodically during the boot process. This may be used to perform servicing
 * of for example, user hardware for tamper detection or other essential
 * operations which need to be carried out while the system is booting.
 *
 * #define SF2BL_USER_HOOK_PERIODIC1 my_periodic_fn_1
 */

/* SF2BL_USER_HOOK_PERIODIC2
 *
 * This macro can be set to the name of a user supplied function to be called
 * periodically during the update process. This may be used to perform servicing
 * of for example, user hardware for tamper detection or other essential
 * operations which need to be carried out while the system is booting.
 *
 * #define SF2BL_USER_HOOK_PERIODIC2 my_periodic_fn_2
 */

/* SF2BL_USER_HOOK_INTERVAL1
 *
 * Time required between successive calls to SF2BL_USER_HOOK_PERIODIC1 in mS.
 * The Bootloader will attempt to call SF2BL_USER_HOOK_PERIODIC1 with no more
 * than the specified interval between calls but the actual time between calls
 * may vary depending on other tasks being performed by the Bootloader.
 *
 * #define SF2BL_USER_HOOK_INTERVAL1 1000
 */

/* SF2BL_USER_HOOK_INTERVAL2
 *
 * Time required between successive calls to SF2BL_USER_HOOK_PERIODIC2 in mS.
 * The Bootloader will attempt to call SF2BL_USER_HOOK_PERIODIC2 with no more
 * than the specified interval between calls but the actual time between calls
 * may vary depending on other tasks being performed by the Bootloader.
 *
 * #define SF2BL_USER_HOOK_INTERVAL2 1000
 */

/* SF2BL_USER_HOOK_STATUS
 *
 * This macro can be set to the name of a user supplied function to be called at
 * various stages during the boot or update process. The function has two
 * parameters, the first is a state value which can be one of the following:
 *
 *   SF2BL_STATE_INIT1
 *   SF2BL_STATE_INIT2
 *   SF2BL_STATE_DEINIT
 *   SF2BL_STATE_READ_FLASH
 *   SF2BL_STATE_VERIFY
 *   SF2BL_STATE_BOOT_FAIL
 *   SF2BL_STATE_UPDATE_START
 *   SF2BL_STATE_UPDATE_TRANSFER
 *   SF2BL_STATE_UPDATE_PROCESS
 *   SF2BL_STATE_UPDATE_WRITE
 *   SF2BL_STATE_UPDATE_OK
 *   SF2BL_STATE_UPDATE_FAIL
 *
 * The second parameter is an integer value between 0 and 100 which represents
 * the percentage of the operation which is currently completed.
 * This hook can be used to provide user indications (visual or otherwise) of
 * the progress of the boot/update operation.
 *
 * #define SF2BL_USER_HOOK_STATUS my_status_fn
 */

#define SF2BL_STATE_INIT1           0
#define SF2BL_STATE_INIT2           1
#define SF2BL_STATE_DEINIT          2
#define SF2BL_STATE_READ_FLASH      3
#define SF2BL_STATE_VERIFY          4
#define SF2BL_STATE_BOOT_FAIL       5
#define SF2BL_STATE_UPDATE_START    6
#define SF2BL_STATE_UPDATE_TRANSFER 7
#define SF2BL_STATE_UPDATE_PROCESS  8
#define SF2BL_STATE_UPDATE_WRITE    9
#define SF2BL_STATE_UPDATE_OK       10
#define SF2BL_STATE_UPDATE_FAIL     11

/* SF2BL_YMODEM_PORT
 *
 * This macro takes the following values to indicate the UART peripheral to use
 * for YMODEM support (default is 0):
 *
 *   0 - MSS UART 0
 *   1 - MSS UART 1
 *   2 – Core16550
 *
 * Macro definitions for the port numbers are provided below.
 *
 * If the Core16550 option is selected, the user is responsible for configuring
 * the interrupt setup for the driver and will need to modify the code in
 * SF2BL_init() and SF2BL_deinit() to accommodate the design specific elements
 * of the system. The details of the changes required is beyond the scope of
 * this User’s Guide.
 */

#define SF2BL_MSS_UART0 0
#define SF2BL_MSS_UART1 1
#define SF2BL_CORE16550 2
#define SF2BL_COREUART  3

#if !defined(SF2BL_YMODEM_PORT)
#define SF2BL_YMODEM_PORT SF2BL_MSS_UART0
#endif

#if (SF2BL_YMODEM_PORT != SF2BL_MSS_UART0) && (SF2BL_YMODEM_PORT != SF2BL_MSS_UART1) \
    && (SF2BL_YMODEM_PORT != SF2BL_CORE16550) && (SF2BL_YMODEM_PORT != SF2BL_COREUART)
#error "Invalid SF2BL_YMODEM_PORT value"
#endif

/* SF2BL_YMODEM_BAUD
 *
 * This is the baud rate to use for the YMODEM connection.
 * For MSS UART, this is the actual baud rate to use and the MSS UART driver
 * will calculate the divisors required based on the baud rate and the clock
 * values in the sys_config_mss_clocks.h file.
 *
 * For Core16550 the divisor is again the actual baud rate to use but the
 * SF2BL_CORE16550_FIC macro also needs to be defined to allow the Bootloader
 * calculate the correct divisor based on the clock for the associated FIC.
 *
 * The default is 57,600bps.
 */

#if !defined(SF2BL_YMODEM_BAUD)
#define SF2BL_MODEM_BAUD 57600
#endif

/* SF2BL_YMODEM_VERBOSE
 *
 * If this macro is defined, the Bootloader will print out additional prompts
 * and status messages on the YMODEM serial port to assist the user with the
 * update process.
 *
 * #define SF2BL_YMODEM_VERBOSE
 */

/* SF2BL_PLATFORM_STRING
 *
 * String printed to identify the target platform when SF2BL_YMODEM_VERBOSE is
 * defined.
 *
 * The default is "Unidentified platform".
 */

#if !defined(SF2BL_PLATFORM_STRING)
#define SF2BL_PLATFORM_STRING "Unidentified platform"
#endif

/* SF2BL_VERBOSE
 *
 * If this macro is defined, the Bootloader will print out additional prompts
 * and status messages on the YMODEM serial port to indicate the progress of the
 * boot process.
 *
 * #define SF2BL_VERBOSE
 */

#if defined(SF2BL_VERBOSE)
#define SF2BL_MESSAGE(x)  _putstring((uint8_t *)x);
#else
#define SF2BL_MESSAGE(x)
#endif

/* SF2BL_CORE16550_FIC
 *
 * This indicates which FIC the Core16550 is connected, 0 or 1 depending on the
 * device capabilities. This is required for baud rate calculations to allow the
 * Bootloader determine which clock information to use for the divisor
 * calculations.
 *
 * The default is 0.
 */

#if !defined(SF2BL_CORE16550_FIC)
#define SF2BL_CORE16550_FIC 0
#endif

/* SF2BL_CORE16550_HW_BASE
 *
 * This should be set to the base address of the registers for the Core16550
 * instance we are using.
 */

#if (SF2BL_YMODEM_PORT == SF2BL_CORE16550) && !defined(SF2BL_CORE16550_HW_BASE)
#error "No hardware base address for Core16550 defined"
#endif

/* SF2BL_EMBEDDED_VER
 *
 * Define this macro to enable support for the embedded version block in the
 * downloaded image.
 *
 * #define SF2BL_EMBEDDED_VER
 */

/* SF2BL_DDR_BASE
 *
 * Base address for non-mapped DDR memory. Amongst other things, this is where
 * the Intel Hex image is loaded into when the firmware update is being
 * performed. The binary image for programming into the SPI FLASH device is
 * assembled at SF2BL_DDR_BASE + (SF2BL_DDR_SIZE / 3) * 2. This is done on the
 * basis that the binary image will be less than half the size of the Intel Hex
 * image and so we allow 2/3 for the hex file and 1/3 for the binary file.
 *
 * This implies that for Intel Hex files we can have a binary image size of at
 * most 1/3 of the DDR capacity. Depending on system configurations this may not
 * be as restrictive as it at first seems. This is because the maximum capacity
 * available for DDR in the 0xA0000000 window is 1,024MB but the maximum that
 * can be mapped into the cacheable code space at 0x00000000 is 512MB which
 * means that for performance reasons the upper limit on code size is 512MB and
 * in this instance we could support an image size of 341MB.
 *
 * The default is 0xA0000000.
 */

#if !defined(SF2BL_DDR_BASE)
#define SF2BL_DDR_BASE 0xA0000000
#endif

/* SF2BL_DDR_SIZE
 *
 * Length of non-mapped DDR memory.
 *
 * Default is 64MB - 67108864 - 0x4000000
 */

#if ! defined(SF2BL_DDR_SIZE)
#define SF2BL_DDR_SIZE 0x4000000
#endif

/* SF2BL_USBD_TIMEOUT
 *
 * This is the number of seconds to wait for a file being downloaded in USB MSC
 * device mode. If there is no file with the appropriate type written to the RAM
 * disk within this time, the system will reboot.
 *
 * The default is 0 which indicates no timeout.
 *
 */

#if ! defined(SF2BL_USBD_TIMEOUT)
#define SF2BL_USBD_TIMEOUT 0
#endif

/* SF2BL_USBD_IDLE
 *
 * This is the number of seconds to wait after a file has been downloaded before
 * assuming the download is complete and the file can be examined.
 *
 * The default is 5 seconds.
 */

#if !defined(SF2BL_USBD_IDLE)
#define SF2BL_USBD_IDLE 5
#endif

#endif /* SF2_BL_OPTIONS_H_ */
