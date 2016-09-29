/*=============================================================*/
/* Created by Microsemi SmartDesign Fri Jan 02 10:11:54 2015   */
/*                                                             */
/* Warning: Do not modify this file, it may lead to unexpected */
/*          functional failures in your design.                */
/*                                                             */
/*=============================================================*/
/*
 * SVN $Revision: 8408 $
 * SVN $Date: 2016-06-04 15:53:19 +0100 (Sat, 04 Jun 2016) $
 */

#ifndef SYS_CONFIG_MSS_CLOCKS
#define SYS_CONFIG_MSS_CLOCKS

#if defined(SF2_EVAL_KIT)
#define MSS_SYS_M3_CLK_FREQ             166000000u
#define MSS_SYS_MDDR_CLK_FREQ           166000000u
#define MSS_SYS_APB_0_CLK_FREQ          83000000u
#define MSS_SYS_APB_1_CLK_FREQ          83000000u
#define MSS_SYS_APB_2_CLK_FREQ          41500000u
#define MSS_SYS_FIC_0_CLK_FREQ          166000000u
#define MSS_SYS_FIC_1_CLK_FREQ          166000000u
#define MSS_SYS_FIC64_CLK_FREQ          166000000u
#endif

#if defined(SF2_SEC_EVAL_KIT)
#define MSS_SYS_M3_CLK_FREQ             142000000u
#define MSS_SYS_MDDR_CLK_FREQ           142000000u
#define MSS_SYS_APB_0_CLK_FREQ          71000000u
#define MSS_SYS_APB_1_CLK_FREQ          71000000u
#define MSS_SYS_APB_2_CLK_FREQ          35500000u
#define MSS_SYS_FIC_0_CLK_FREQ          142000000u
#define MSS_SYS_FIC_1_CLK_FREQ          142000000u
#define MSS_SYS_FIC64_CLK_FREQ          142000000u
#endif

#if defined(SF2_ADV_DEV_KIT)
#define MSS_SYS_M3_CLK_FREQ             166000000u
#define MSS_SYS_MDDR_CLK_FREQ           332000000u
#define MSS_SYS_APB_0_CLK_FREQ          83000000u
#define MSS_SYS_APB_1_CLK_FREQ          83000000u
#define MSS_SYS_APB_2_CLK_FREQ          41500000u
#define MSS_SYS_FIC_0_CLK_FREQ          166000000u
#define MSS_SYS_FIC_1_CLK_FREQ          166000000u
#define MSS_SYS_FIC64_CLK_FREQ          332000000u
#endif

#if defined(SF2_DEV_KIT)
#define MSS_SYS_M3_CLK_FREQ             166000000u
#define MSS_SYS_MDDR_CLK_FREQ           332000000u
#define MSS_SYS_APB_0_CLK_FREQ          83000000u
#define MSS_SYS_APB_1_CLK_FREQ          83000000u
#define MSS_SYS_APB_2_CLK_FREQ          41500000u
#define MSS_SYS_FIC_0_CLK_FREQ          166000000u
#define MSS_SYS_FIC_1_CLK_FREQ          166000000u
#define MSS_SYS_FIC64_CLK_FREQ          332000000u
#endif

#if defined(SF2_DEV_KIT_1588)
#define MSS_SYS_M3_CLK_FREQ             150000000u
#define MSS_SYS_MDDR_CLK_FREQ           300000000u
#define MSS_SYS_APB_0_CLK_FREQ          75000000u
#define MSS_SYS_APB_1_CLK_FREQ          75000000u
#define MSS_SYS_APB_2_CLK_FREQ          37500000u
#define MSS_SYS_FIC_0_CLK_FREQ          150000000u
#define MSS_SYS_FIC_1_CLK_FREQ          150000000u
#define MSS_SYS_FIC64_CLK_FREQ          300000000u
#endif

#endif /* SYS_CONFIG_MSS_CLOCKS */
