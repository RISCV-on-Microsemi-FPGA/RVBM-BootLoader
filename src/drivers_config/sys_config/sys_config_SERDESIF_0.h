/*=============================================================*/
/* Created by Microsemi SmartDesign Thu Dec 18 12:08:17 2014   */
/*                                                             */
/* Warning: Do not modify this file, it may lead to unexpected */
/*          simulation failures in your design.                */
/*                                                             */
/*=============================================================*/
/*
 * SVN $Revision: 8408 $
 * SVN $Date: 2016-06-04 15:53:19 +0100 (Sat, 04 Jun 2016) $
 */

/*-------------------------------------------------------------*/
/* SERDESIF_0 Initialization                                   */
/*-------------------------------------------------------------*/

#if defined(SF2_DEV_KIT)
#define SERDES_0_CFG_NB_OF_PAIRS 18u
#endif

#if defined(SF2_DEV_KIT_1588)
#define SERDES_0_CFG_NB_OF_PAIRS 17u
#endif

#if defined(SF2_EVAL_KIT)
#define SERDES_0_CFG_NB_OF_PAIRS 17u
#endif

#if defined(SF2_SEC_EVAL_KIT)
#define SERDES_0_CFG_NB_OF_PAIRS 20u
#endif
