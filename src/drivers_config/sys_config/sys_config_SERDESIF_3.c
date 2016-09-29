/*=============================================================*/
/* Created by Microsemi SmartDesign Fri Mar 20 10:11:35 2015   */
/*                                                             */
/* Warning: Do not modify this file, it may lead to unexpected */
/*          simulation failures in your design.                */
/*                                                             */
/*=============================================================*/
/*
 * SVN $Revision: 7337 $
 * SVN $Date: 2015-04-21 12:44:42 +0100 (Tue, 21 Apr 2015) $
 */

/*-------------------------------------------------------------*/
/* SERDESIF_3 Initialization                                   */
/*-------------------------------------------------------------*/

#include <stdint.h>

#include "sf2_bl_options.h"
#include "sf2_bl_defs.h"

#include "../../CMSIS/sys_init_cfg_types.h"
#if defined(SF2_ADV_DEV_KIT)
const cfg_addr_value_pair_t g_m2s_serdes_3_config[] =
{
    { (uint32_t*)( 0x40034000 + 0x2028 ), 0x80F } /* SYSTEM_CONFIG_PHY_MODE_1 */ ,
    { (uint32_t*)( 0x40034000 + 0x1d98 ), 0x30 } /* LANE3_PHY_RESET_OVERRIDE */ ,
    { (uint32_t*)( 0x40034000 + 0x1c00 ), 0x80 } /* LANE3_CR0 */ ,
    { (uint32_t*)( 0x40034000 + 0x1c04 ), 0x20 } /* LANE3_ERRCNT_DEC */ ,
    { (uint32_t*)( 0x40034000 + 0x1c08 ), 0xF8 } /* LANE3_RXIDLE_MAX_ERRCNT_THR */ ,
    { (uint32_t*)( 0x40034000 + 0x1c0c ), 0x80 } /* LANE3_IMPED_RATIO */ ,
    { (uint32_t*)( 0x40034000 + 0x1c14 ), 0x29 } /* LANE3_PLL_M_N */ ,
    { (uint32_t*)( 0x40034000 + 0x1c18 ), 0x20 } /* LANE3_CNT250NS_MAX */ ,
    { (uint32_t*)( 0x40034000 + 0x1c24 ), 0x80 } /* LANE3_TX_AMP_RATIO */ ,
    { (uint32_t*)( 0x40034000 + 0x1c30 ), 0x10 } /* LANE3_ENDCALIB_MAX */ ,
    { (uint32_t*)( 0x40034000 + 0x1c34 ), 0x38 } /* LANE3_CALIB_STABILITY_COUNT */ ,
    { (uint32_t*)( 0x40034000 + 0x1c3c ), 0x70 } /* LANE3_RX_OFFSET_COUNT */ ,
    { (uint32_t*)( 0x40034000 + 0x1dd4 ), 0x2 } /* LANE3_GEN1_TX_PLL_CCP */ ,
    { (uint32_t*)( 0x40034000 + 0x1dd8 ), 0x22 } /* LANE3_GEN1_RX_PLL_CCP */ ,
    { (uint32_t*)( 0x40034000 + 0x1d98 ), 0x0 } /* LANE3_PHY_RESET_OVERRIDE */ ,
    { (uint32_t*)( 0x40034000 + 0x1e00 ), 0x1 } /* LANE3_UPDATE_SETTINGS */ ,
    { (uint32_t*)( 0x40034000 + 0x2028 ), 0xF0F } /* SYSTEM_CONFIG_PHY_MODE_1 */ 
};
#endif
