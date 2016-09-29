
#ifndef RISCV_COREPLEXE31_H
#define RISCV_COREPLEXE31_H

#include "plic.h"

#define PLIC_BASE_ADDR         0x40000000UL


// Setting these correctly makes the initialization scripts
// run faster.
#define PLIC_NUM_SOURCES 31
#define PLIC_NUM_PRIORITIES 0

/*
 * Interrupt numbers:
 */
#define NoInterrupt_IRQn    0
#define External_1_IRQn     1
#define External_2_IRQn     2
#define External_3_IRQn     3 
#define External_4_IRQn     4
#define External_5_IRQn     5
#define External_6_IRQn     6
#define External_7_IRQn     7
#define External_8_IRQn     8
#define External_9_IRQn     9
#define External_10_IRQn    10
#define External_11_IRQn    11
#define External_12_IRQn    12 
#define External_13_IRQn    13 
#define External_14_IRQn    14 
#define External_15_IRQn    15 
#define External_16_IRQn    16 
#define External_17_IRQn    17 
#define External_18_IRQn    18
#define External_19_IRQn    19
#define External_20_IRQn    20
#define External_21_IRQn    21
#define External_22_IRQn    22
#define External_23_IRQn    23
#define External_24_IRQn    24
#define External_25_IRQn    25
#define External_26_IRQn    26
#define External_27_IRQn    27
#define External_28_IRQn    28
#define External_29_IRQn    29
#define External_30_IRQn    30
#define External_31_IRQn    31

#endif  /* RISCV_COREPLEXE31_H */

