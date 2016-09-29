/*******************************************************************************
 * (c) Copyright 2016 Microsemi SoC Products Group.  All rights reserved.
 *
 * RISC-V Bootloader application execution routine
 *
 * SVN $Revision: $
 * SVN $Date: $
 */

#include <stdint.h>
#include "sf2_bl_options.h"

void interrupts_deinit(void);

/***************************************************************************//**
 * This is the final  stage of the bootloader before passing control to the
 * application which now resides in DDR. All interrupt generating sources should
 * be shut down at this stage.
 */

void sf2bl_exec(void)
{
    /*
     * Disable all interrupts.
     */
    interrupts_deinit();

    /*
     * Set the return address register to the base of the DDR memory at 0x80000000.
     * The reset handler of the application loaded to DDR memory by the  bootloader
     * is expected to be at that location.
     */
    __asm volatile("lui ra,0x80000");
    
    /*
     * Flush the cache.
     */
    __asm volatile ("fence.i");

    /*
     * We need to explicitly execute a return intruction in case the compiler had
     * done some return addres register manipulation in this function's veneer.
     */
    __asm volatile("ret");
}
