/*******************************************************************************
 * (c) Copyright 2016 Microsemi SoC Products Group.  All rights reserved.
 *
 * SmartFusion2 Bootloader user configurations file.
 *
 * SVN $Revision: 8449 $
 * SVN $Date: 2016-06-26 15:15:12 +0100 (Sun, 26 Jun 2016) $
 */

#ifndef SF2_BL_USER_OPTS_H_
#define SF2_BL_USER_OPTS_H_

/* #define SF2_ADV_DEV_KIT */  /* Advanced Dev Kit with M2S150 */
/* #define SF2_DEV_KIT */      /* Dev Kit with M2S050T */
/* #define SF2_DEV_KIT_1588 */ /* Dev Kit with M2S050T and 1588 test design */
/* #define SF2_EVAL_KIT */     /* Eval Kit with M2S025T */
/* #define SF2_SEC_EVAL_KIT */ /* Eval Kit with M2s090TS */
/* #define SF2_DEV_KIT_RISCV *//* RISC-V on Advanced Dev Kit with M2S150 */
#define SF2_DEV_KIT_RISCV

/*
 *  <CJ>TODO Roll the following into a new platform configuration opion
 */
#define SF2BL_USE_COREGPIO
#define SF2BL_USE_CORESPI

#if defined(SF2_DEV_KIT_RISCV)
/*
 *  SF2 Dev Kit sample application configuration for RISC-V sample design:
 *
 * The following MSS GPIOs are used in the example:
 *
 *   MSS_GPIO_0  - W6  - Dev Kit SW1
 *   MSS_GPIO_1  - AA1 - Dev Kit SW2
 *   MSS_GPIO_2  - AB1 - Dev Kit SW4
 *   MSS_GPIO_3  - AB2 - Dev Kit SW5
 *   MSS_GPIO_4  - R3  - Dev Kit DIP1
 *   MSS_GPIO_5  - R4  - Dev Kit DIP2
 *   MSS_GPIO_6  - AE2 - Dev Kit DIP3
 *   MSS_GPIO_7  - AD1 - Dev Kit DIP4
 *   MSS_GPIO_8  - A18 - LED 1, flashes once a second to show link status task is active
 *   MSS_GPIO_9  - B18 - LED 2, toggles each time an RNDIS control message is received
 *   MSS_GPIO_10 - D18 - LED 3, toggles each time a network packet is received
 *   MSS_GPIO_11 - E18 - LED 4, toggles each time a network packet is sent
 *   MSS_GPIO_12 - A20 - LED 5
 *   MSS_GPIO_13 - D20 - LED 6
 *   MSS_GPIO_14 - E20 - LED 7
 *   MSS_GPIO_15 - B20 - LED 8
 *   MSS_GPIO_16 - P24 - USB PHY Reset
 *
 *   MSS_GPIO_17 - J29 - Spare - ADS8568_DB0, J184 pin 2
 *   MSS_GPIO_18 - J28 - Spare - ADS8568_SDO_A, J187 pin 2
 *
 *   MSS_GPIO_24 - N23  - ADS8568_RESET - J214 pin 2, for debug output purposes
 *   MSS_GPIO_25 - L26  - ADS8568_DB8   - J175 pin 2, for debug output purposes
 *   MSS_GPIO_26 - M25  - ADS8568_CSN   - J201 pin 2, for debug output purposes
 *   MSS_GPIO_27 - Not used due to clash with MSS UART 0 fabric requirements
 *   MSS_GPIO_28 - J30  - ADS8568_DB11  - J183 pin 2, for debug output purposes
 *   MSS_GPIO_29 - H29  - ADS8568_SDO_B - J194 pin 2, for debug output purposes
 *   MSS_GPIO_30 - G30  - ADS8568_SDO_C - J196 pin 2, for debug output purposes
 *   MSS_GPIO_31 - F30  - ADS8568_SDO_D - J200 pin 2, for debug output purposes
 *
 *   MMUART_1_RXD - G29  - MSIO072PB7 J1 pin 55, for debug output purposes
 *   MMUART_1_TXD - H30  - MSIO072NB7 J1 pin 57, for debug output purposes
 *
 */
#define SF2BL_TARGET_SYSTEM     SF2BL_TARGET_CUSTOM
#define RISCV_PLATFORM
#define SF2BL_DDR_BASE          0x80000000
#define SF2BL_DDR_SIZE          0x10000000

#define SF2BL_YMODEM_PORT       SF2BL_COREUART
#define SF2BL_YMODEM_VERBOSE
#define SF2BL_VERBOSE
#define SF2BL_YMODEM_BAUD       115200
#define SF2BL_MODEM_BAUD        SF2BL_YMODEM_BAUD


#define SF2BL_SPI_PORT              SF2BL_CORE_SPI
//<CJ>#define SF2BL_FLASH_USE_DMA
#define SF2BL_FLASH_WRITE_VERIFY
#define SF2BL_SPI_WP               SF2BL_NO_PIN
#define SF2BL_SPI_RESET            SF2BL_NO_PIN
#define SF2BL_USER_UPDATE_PIN      4
#define SF2BL_USER_USE_GOLDEN_PIN  SF2BL_NO_PIN
#define SF2BL_USER_COPY_GOLDEN_PIN SF2BL_NO_PIN

//<CJ>#define SF2BL_2ND_IMAGE
//<CJ>#define SF2BL_GOLDEN
#define SF2BL_FLASH_DEVICE             SF2BL_FLASH_DEV_N25Q00AA13GSF40G
#define SF2BL_FLASH_SIZE               134217728
#define SF2BL_FLASH_BASE               0

#if defined(SF2BL_GOLDEN) && defined(SF2BL_2ND_IMAGE)

/* Adjust log size to ensure images are on 4K boundary in all cases */
#define SF2BL_LOG_SIZE                 6
#else
#define SF2BL_LOG_SIZE                 8
#endif
#define SF2BL_CONFIG_SIZE              8
#define SF2BL_FLASH_IMAGE_GRANUALARITY 4096
//<CJ>#define SF2BL_DDR_SIZE                 0x20000000

#define SF2BL_PLATFORM_STRING "SmartFusion2 Dev Kit"
#endif
 
#if defined(SF2_DEV_KIT)
/*
 *  SF2 Dev Kit sample application configuration:
 *
 * The following MSS GPIOs are used in the example:
 *
 *   MSS_GPIO_0  - W6  - Dev Kit SW1
 *   MSS_GPIO_1  - AA1 - Dev Kit SW2
 *   MSS_GPIO_2  - AB1 - Dev Kit SW4
 *   MSS_GPIO_3  - AB2 - Dev Kit SW5
 *   MSS_GPIO_4  - R3  - Dev Kit DIP1
 *   MSS_GPIO_5  - R4  - Dev Kit DIP2
 *   MSS_GPIO_6  - AE2 - Dev Kit DIP3
 *   MSS_GPIO_7  - AD1 - Dev Kit DIP4
 *   MSS_GPIO_8  - A18 - LED 1, flashes once a second to show link status task is active
 *   MSS_GPIO_9  - B18 - LED 2, toggles each time an RNDIS control message is received
 *   MSS_GPIO_10 - D18 - LED 3, toggles each time a network packet is received
 *   MSS_GPIO_11 - E18 - LED 4, toggles each time a network packet is sent
 *   MSS_GPIO_12 - A20 - LED 5
 *   MSS_GPIO_13 - D20 - LED 6
 *   MSS_GPIO_14 - E20 - LED 7
 *   MSS_GPIO_15 - B20 - LED 8
 *   MSS_GPIO_16 - P24 - USB PHY Reset
 *
 *   MSS_GPIO_17 - J29 - Spare - ADS8568_DB0, J184 pin 2
 *   MSS_GPIO_18 - J28 - Spare - ADS8568_SDO_A, J187 pin 2
 *
 *   MSS_GPIO_24 - N23  - ADS8568_RESET - J214 pin 2, for debug output purposes
 *   MSS_GPIO_25 - L26  - ADS8568_DB8   - J175 pin 2, for debug output purposes
 *   MSS_GPIO_26 - M25  - ADS8568_CSN   - J201 pin 2, for debug output purposes
 *   MSS_GPIO_27 - Not used due to clash with MSS UART 0 fabric requirements
 *   MSS_GPIO_28 - J30  - ADS8568_DB11  - J183 pin 2, for debug output purposes
 *   MSS_GPIO_29 - H29  - ADS8568_SDO_B - J194 pin 2, for debug output purposes
 *   MSS_GPIO_30 - G30  - ADS8568_SDO_C - J196 pin 2, for debug output purposes
 *   MSS_GPIO_31 - F30  - ADS8568_SDO_D - J200 pin 2, for debug output purposes
 *
 *   MMUART_1_RXD - G29  - MSIO072PB7 J1 pin 55, for debug output purposes
 *   MMUART_1_TXD - H30  - MSIO072NB7 J1 pin 57, for debug output purposes
 *
 */
#define SF2BL_YMODEM_PORT   0
#define SF2BL_YMODEM_VERBOSE
#define SF2BL_VERBOSE

#define SF2BL_SPI_PORT      0
#define SF2BL_FLASH_USE_DMA
#define SF2BL_FLASH_WRITE_VERIFY
#define SF2BL_SPI_WP               SF2BL_NO_PIN
#define SF2BL_SPI_RESET            SF2BL_NO_PIN
#define SF2BL_USER_UPDATE_PIN      4
#define SF2BL_USER_USE_GOLDEN_PIN  5
#define SF2BL_USER_COPY_GOLDEN_PIN 6

#define SF2BL_2ND_IMAGE
#define SF2BL_GOLDEN
#define SF2BL_FLASH_DEVICE             SF2BL_FLASH_DEV_AT25DF641
#define SF2BL_FLASH_SIZE               8388608
#define SF2BL_FLASH_BASE               0
#if defined(SF2BL_GOLDEN) && defined(SF2BL_2ND_IMAGE)

/* Adjust log size to ensure images are on 4K boundary in all cases */
#define SF2BL_LOG_SIZE                 6
#else
#define SF2BL_LOG_SIZE                 8
#endif
#define SF2BL_CONFIG_SIZE              8
#define SF2BL_FLASH_IMAGE_GRANUALARITY 4096
#define SF2BL_DDR_SIZE                 0x20000000

#define SF2BL_PLATFORM_STRING "SmartFusion2 Dev Kit"
#endif

#if defined(SF2_DEV_KIT_1588)
/*
 *  SF2 Eval Kit sample application configuration:
 *
 * The following MSS GPIOs are used in the example:
 *
 *   MSS_GPIO_0  - V28/L26 - ZL_CS/DEBUG/ZL_SPI_CS
 *   MSS_GPIO_1  - R3      - Dev Kit DIP1
 *   MSS_GPIO_2  - R4      - Dev Kit DIP2
 *   MSS_GPIO_3  - AE2     - Dev Kit DIP3
 *   MSS_GPIO_4  - AD1     - Dev Kit DIP4
 *   MSS_GPIO_8  - N3      - PHY_INTN
 *   MSS_GPIO_9  - T28     - PPS_CLK
 *   MSS_GPIO_16 - L23     - GPIO_DELAY_DEBUG
 *
 */
#define SF2BL_YMODEM_PORT   0
#define SF2BL_YMODEM_VERBOSE
#define SF2BL_VERBOSE

#define SF2BL_SPI_PORT      0
#define SF2BL_FLASH_USE_DMA
#define SF2BL_FLASH_WRITE_VERIFY
#define SF2BL_SPI_WP               SF2BL_NO_PIN
#define SF2BL_SPI_RESET            SF2BL_NO_PIN
#define SF2BL_USER_UPDATE_PIN      1
#define SF2BL_USER_USE_GOLDEN_PIN  2
#define SF2BL_USER_COPY_GOLDEN_PIN 3

#define SF2BL_2ND_IMAGE
#define SF2BL_GOLDEN
#define SF2BL_FLASH_DEVICE             SF2BL_FLASH_DEV_AT25DF641
#define SF2BL_FLASH_SIZE               8388608
#define SF2BL_FLASH_BASE               0
#if defined(SF2BL_GOLDEN) && defined(SF2BL_2ND_IMAGE)

/* Adjust log size to ensure images are on 4K boundary in all cases */
#define SF2BL_LOG_SIZE                 6
#else
#define SF2BL_LOG_SIZE                 8
#endif
#define SF2BL_CONFIG_SIZE              8
#define SF2BL_FLASH_IMAGE_GRANUALARITY 4096
#define SF2BL_DDR_SIZE                 0x20000000

#define SF2BL_PLATFORM_STRING "SmartFusion2 Dev Kit 1588"
#endif

#if defined(SF2_EVAL_KIT)  || defined(SF2_SEC_EVAL_KIT)
/*
 *  SF2 Eval Kit sample application configuration:
 *
 * The following MSS GPIOs are used in the example:
 *
 *   MSS_GPIO_0  - L20 - Eval Kit SW1
 *   MSS_GPIO_1  - K16 - Eval Kit SW2
 *   MSS_GPIO_2  - K18 - Eval Kit SW3
 *   MSS_GPIO_3  - J18 - Eval Kit SW4
 *   MSS_GPIO_4  - L19 - Eval Kit DIP1
 *   MSS_GPIO_5  - L18 - Eval Kit DIP2
 *   MSS_GPIO_6  - K21 - Eval Kit DIP3
 *   MSS_GPIO_7  - K20 - Eval Kit DIP4
 *   MSS_GPIO_8  - E1  - LED 1, flashes once a second to show link status task is active
 *   MSS_GPIO_9  - F4  - LED 2, toggles each time an RNDIS control message is received
 *   MSS_GPIO_10 - F3  - LED 3, toggles each time a network packet is received
 *   MSS_GPIO_11 - G7  - LED 4, toggles each time a network packet is sent
 *   MSS_GPIO_12 - H7  - LED 5
 *   MSS_GPIO_13 - J6  - LED 6
 *   MSS_GPIO_14 - H6  - LED 7
 *   MSS_GPIO_15 - H5  - LED 8
 *   MSS_GPIO_16 - R16 - USB PHY Reset
 *   MSS_GPIO_17 - K15 - SPI FLASH Reset
 *   MSS_GPIO_18 - L16 - SPI FLASH WP
 *
 *   MSS_GPIO_24 - AB15 - MSIO110PB4 J1 pin 1,  for debug output purposes
 *   MSS_GPIO_25 - AA15 - MSIO110NB4 J1 pin 3,  for debug output purposes
 *   MSS_GPIO_26 - AB18 - MSIO118PB4 J1 pin 7,  for debug output purposes
 *   MSS_GPIO_27 - Not used due to clash with MSS UART 0 fabric requirements
 *   MSS_GPIO_28 - AB19 - MSIO118NB4 J1 pin 9,  for debug output purposes
 *   MSS_GPIO_29 - Y18  - MSIO117PB4 J1 pin 13, for debug output purposes
 *   MSS_GPIO_30 - Y19  - MSIO117NB4 J1 pin 15, for debug output purposes
 *   MSS_GPIO_31 - W16  - MSIO115PB4 J1 pin 19, for debug output purposes
 *
 *   MMUART_0_RXD - D1  - MSIO072PB7 J1 pin 55, for debug output purposes
 *   MMUART_0_TXD - D2  - MSIO072NB7 J1 pin 57, for debug output purposes
 *
 */
#define SF2BL_YMODEM_PORT   1
#define SF2BL_YMODEM_VERBOSE
#define SF2BL_VERBOSE

#define SF2BL_SPI_PORT      0
#define SF2BL_FLASH_USE_DMA
#define SF2BL_FLASH_WRITE_VERIFY
#define SF2BL_SPI_WP               18
#define SF2BL_SPI_RESET            17
#define SF2BL_USER_UPDATE_PIN      4
#define SF2BL_USER_USE_GOLDEN_PIN  5
#define SF2BL_USER_COPY_GOLDEN_PIN 6

#define SF2BL_2ND_IMAGE
#define SF2BL_GOLDEN
#define SF2BL_FLASH_DEVICE             SF2BL_FLASH_DEV_W25Q64FVSSIG
#define SF2BL_FLASH_SIZE               8388608
#define SF2BL_FLASH_BASE               0
#if defined(SF2BL_GOLDEN) && defined(SF2BL_2ND_IMAGE)

/* Adjust log size to ensure images are on 4K boundary in all cases */
#define SF2BL_LOG_SIZE                 6
#else
#define SF2BL_LOG_SIZE                 8
#endif
#define SF2BL_CONFIG_SIZE              8
#define SF2BL_FLASH_IMAGE_GRANUALARITY 4096
#define SF2BL_DDR_SIZE                 0x4000000

#if defined(SF2_EVAL_KIT)
#define SF2BL_PLATFORM_STRING "SmartFusion2 Eval Kit"
#else
#define SF2BL_PLATFORM_STRING "SmartFusion2 Security Eval Kit"
#endif
#endif

#if defined(SF2_ADV_DEV_KIT)
/*
 * The following MSS GPIOs are used in the example for the SmartFusion2 Advanced
 * Dev Kit:
 *
 *   MSS_GPIO_0  - J25 - Adv Dev Kit SW1
 *   MSS_GPIO_1  - H25 - Adv Dev Kit SW2
 *   MSS_GPIO_2  - J24 - Adv Dev Kit SW3
 *   MSS_GPIO_3  - H23 - Adv Dev Kit SW4
 *   MSS_GPIO_4  - F25 - Adv Dev Kit DIP0
 *   MSS_GPIO_5  - G25 - Adv Dev Kit DIP1
 *   MSS_GPIO_6  - J23 - Adv Dev Kit DIP2
 *   MSS_GPIO_7  - J22 - Adv Dev Kit DIP3
 *   MSS_GPIO_8  - D26 - LED 0, flashes to show link status task is active
 *   MSS_GPIO_9  - F26 - LED 1, toggles when RNDIS control message is received
 *   MSS_GPIO_10 - A27 - LED 2, toggles each time a network packet is received
 *   MSS_GPIO_11 - C26 - LED 3, toggles each time a network packet is sent
 *   MSS_GPIO_12 - Not used due to clash with MSS SPI 1 fabric requirements
 *   MSS_GPIO_13 - Not used due to clash with MSS SPI 1 fabric requirements
 *   MSS_GPIO_14 - C27 - LED 6
 *   MSS_GPIO_15 - E26 - LED 7
 *   MSS_GPIO_16 - N9  - USB PHY Reset
 *   MSS_GPIO_17 - N3  - SPI FLASH 0 WP
 *   MSS_GPIO_18 - J2  - SPI FLASH 1 WP
 *   MSS_GPIO_12 - C28 - LED 4
 *   MSS_GPIO_13 - B27 - LED 5
 *
 *   MSS_GPIO_24 - R7  - BB_GPIO 5,  for debug output purposes
 *   MSS_GPIO_25 - P7  - BB_GPIO 6,  for debug output purposes
 *   MSS_GPIO_26 - M2  - BB_GPIO 7,  for debug output purposes
 *   MSS_GPIO_27 - M3  - BB_GPIO 8,  for debug output purposes
 *   MSS_GPIO_28 - R9  - BB_GPIO 9,  for debug output purposes
 *   MSS_GPIO_29 - R10 - BB_GPIO 10, for debug output purposes
 *   MSS_GPIO_30 - P8  - BB_GPIO 11, for debug output purposes
 *   MSS_GPIO_31 - P9  - BB_GPIO 12, for debug output purposes
 *
 *   MMUART_1_RXD - L10 - BB_UART_RX, for debug output purposes
 *   MMUART_1_TXD - E3  - BB_UART_TX, for debug output purposes
 */
#define SF2BL_YMODEM_PORT   0
#define SF2BL_YMODEM_VERBOSE
#define SF2BL_VERBOSE

#define SF2BL_SPI_PORT      0
#define SF2BL_FLASH_USE_DMA
#define SF2BL_FLASH_WRITE_VERIFY
#define SF2BL_SPI_WP               17
#define SF2BL_USER_UPDATE_PIN      4
#define SF2BL_USER_USE_GOLDEN_PIN  5
#define SF2BL_USER_COPY_GOLDEN_PIN 6

#define SF2BL_2ND_IMAGE
#define SF2BL_GOLDEN
#define SF2BL_FLASH_DEVICE             SF2BL_FLASH_DEV_N25Q00AA13GSF40G
#define SF2BL_FLASH_SIZE               134217728
#define SF2BL_FLASH_BASE               0
#if defined(SF2BL_GOLDEN) && defined(SF2BL_2ND_IMAGE)

/* Adjust log size to ensure images are on 4K boundary in all cases */
#define SF2BL_LOG_SIZE                 6
#else
#define SF2BL_LOG_SIZE                 8
#endif
#define SF2BL_CONFIG_SIZE              8
#define SF2BL_FLASH_IMAGE_GRANUALARITY 4096
#define SF2BL_DDR_SIZE                 0x20000000

#define SF2BL_PLATFORM_STRING "SmartFusion2 Advanced Dev Kit"
#endif
#endif /* SF2_BL_USER_OPTS_H_ */
