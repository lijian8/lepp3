/*!
@sd_LOLA_uEIDN.h

 User SercosIII data base entries for Freescale DSP MC56F8367

 Copyright (C) Institute of Applied Mechanics, TU-Muenchen
 All rights reserved.
 Contact: favot@amm.mw.tum.de

*/

#ifndef SD_LOLA_uEIDN__H_
#define SD_LOLA_uEIDN__H_
#ifdef X86_PC
#include <stdint.h>
//data types (LONG_A etc)
#define LONG_A uint32_t 

#endif//X86_PC
    /*!
      DataBase version number
    */
    enum{SERCOS_USER_DB_SERCOS_VERSION=0x000C};

//--------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------
// Local redefine the Element number for SercosIII EIDNs
#define S_ELEMENT_SHIFT_lola (16)
#define S_ELEM_MASK_lola     (0xffUL << S_ELEMENT_SHIFT_lola )
#define	S_ELEM_0_lola		 (0)
#define S_ELEM_1_lola        (1UL    << S_ELEMENT_SHIFT_lola )
#define S_ELEM_1_lola        (1UL    << S_ELEMENT_SHIFT_lola )
#define S_ELEM_2_lola        (2UL    << S_ELEMENT_SHIFT_lola )
#define S_ELEM_3_lola        (3UL    << S_ELEMENT_SHIFT_lola )
#define S_ELEM_4_lola        (4UL    << S_ELEMENT_SHIFT_lola )
#define S_ELEM_5_lola        (5UL    << S_ELEMENT_SHIFT_lola )
#define S_ELEM_6_lola        (6UL    << S_ELEMENT_SHIFT_lola )
#define S_ELEM_7_lola        (7UL    << S_ELEMENT_SHIFT_lola )
#define S_ELEM_8_lola        (8UL    << S_ELEMENT_SHIFT_lola )
#define S_ELEM_9_lola        (9UL    << S_ELEMENT_SHIFT_lola )
#define S_ELEM_10_lola       (10UL   << S_ELEMENT_SHIFT_lola )
#define S_ELEM_11_lola       (11UL   << S_ELEMENT_SHIFT_lola )
#define S_ELEM_12_lola       (12UL   << S_ELEMENT_SHIFT_lola )
#define S_ELEM_13_lola       (13UL   << S_ELEMENT_SHIFT_lola )
#define S_ELEM_14_lola       (14UL   << S_ELEMENT_SHIFT_lola )
#define S_ELEM_15_lola       (15UL   << S_ELEMENT_SHIFT_lola )
#define S_ELEM_16_lola       (16UL   << S_ELEMENT_SHIFT_lola )
#define S_ELEM_17_lola       (17UL   << S_ELEMENT_SHIFT_lola )
#define S_ELEM_18_lola       (18UL   << S_ELEMENT_SHIFT_lola )
#define S_ELEM_19_lola       (19UL   << S_ELEMENT_SHIFT_lola )
#define S_ELEM_20_lola       (20UL   << S_ELEMENT_SHIFT_lola )

// favot@amm.mw.tum.de --> Added macro to address the P/S part of the DataBase
// __PS=0 --> "S"
// __PS=1 --> "P"
#define EIDN_S 0
#define EIDN_P 1
#define EIDNPS(__PS,__id,__si,__se) ((((LONG_A)__si) << 24) | (((LONG_A)__se)) | ((LONG_A)__PS << 15) | ((LONG_A)__id))
//--------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------
//++++++++++++++++++++++++++++++++++++++++++++++++++++
//SerocsDB Offset for S parameters
//++++++++++++++++++++++++++++++++++++++++++++++++++++
//Communication cycle time (tScyc)
#define S_tScyc_IDN_OFFSET	1002
	// Paramters Elements
	#define S_tScyc_IDN	EIDNPS(EIDN_S,S_tScyc_IDN_OFFSET,0,S_ELEM_0_lola)

//++++++++++++++++++++++++++++++++++++++++++++++++++++
//Added SerocsDB Offset for P parameters
//++++++++++++++++++++++++++++++++++++++++++++++++++++
//Joint Position command database Offset (values scaled to 24 bit)
#define P_Joint_Position_cmn_IDN_OFFSET	101
	// Paramters Elements
	#define P_Position_cmn_Joint0_IDN	EIDNPS(EIDN_P,P_Joint_Position_cmn_IDN_OFFSET,0,S_ELEM_0_lola)
	#define P_Position_cmn_Joint1_IDN	EIDNPS(EIDN_P,P_Joint_Position_cmn_IDN_OFFSET,0,S_ELEM_1_lola)
	#define P_Position_cmn_Joint2_IDN	EIDNPS(EIDN_P,P_Joint_Position_cmn_IDN_OFFSET,0,S_ELEM_2_lola)

//Motor Position command database Offset (values scaled to 24 bit)
#define P_Motor_Position_cmn_IDN_OFFSET	102
	// Paramters Elements
	#define P_Position_cmn_Motor0_IDN	EIDNPS(EIDN_P,P_Motor_Position_cmn_IDN_OFFSET,0,S_ELEM_0_lola)
	#define P_Position_cmn_Motor1_IDN	EIDNPS(EIDN_P,P_Motor_Position_cmn_IDN_OFFSET,0,S_ELEM_1_lola)
	#define P_Position_cmn_Motor2_IDN	EIDNPS(EIDN_P,P_Motor_Position_cmn_IDN_OFFSET,0,S_ELEM_2_lola)
	
//Joint Position feedback database Offset (values scaled to 24 bit)
#define P_Joint_Position_fb_IDN_OFFSET	111
	// Paramters Elements
	#define P_Position_fb_Joint0_IDN	EIDNPS(EIDN_P,P_Joint_Position_fb_IDN_OFFSET,0,S_ELEM_0_lola)
	#define P_Position_fb_Joint1_IDN	EIDNPS(EIDN_P,P_Joint_Position_fb_IDN_OFFSET,0,S_ELEM_1_lola)
	#define P_Position_fb_Joint2_IDN	EIDNPS(EIDN_P,P_Joint_Position_fb_IDN_OFFSET,0,S_ELEM_2_lola)

//Motor Position feedback database Offset (values scaled to 24 bit)
#define P_Motor_Position_fb_IDN_OFFSET	112
	// Paramters Elements
	#define P_Position_fb_Motor0_IDN	EIDNPS(EIDN_P,P_Motor_Position_fb_IDN_OFFSET,0,S_ELEM_0_lola)
	#define P_Position_fb_Motor1_IDN	EIDNPS(EIDN_P,P_Motor_Position_fb_IDN_OFFSET,0,S_ELEM_1_lola)
	#define P_Position_fb_Motor2_IDN	EIDNPS(EIDN_P,P_Motor_Position_fb_IDN_OFFSET,0,S_ELEM_2_lola)

//Motor Velocity feedback database Offset (values scaled to 24 bit) -- Computed on FPGA
#define P_IncEncVel_fb_IDN_OFFSET	115
	// Paramters Elements
	#define P_IncEncVel_fb_Joint0_IDN		EIDNPS(EIDN_P,P_IncEncVel_fb_IDN_OFFSET,0,S_ELEM_0_lola)
	#define P_IncEncVel_fb_Joint1_IDN		EIDNPS(EIDN_P,P_IncEncVel_fb_IDN_OFFSET,0,S_ELEM_1_lola)
	#define P_IncEncVel_fb_Joint2_IDN		EIDNPS(EIDN_P,P_IncEncVel_fb_IDN_OFFSET,0,S_ELEM_2_lola)

//Motor Velocity feedback database Offset (values scaled to 24 bit) -- Computed on ELMO
#define P_IncEncVelElmo_fb_IDN_OFFSET	116
	// Paramters Elements
	#define P_IncEncVelElmo_fb_Joint0_IDN		EIDNPS(EIDN_P,P_IncEncVelElmo_fb_IDN_OFFSET,0,S_ELEM_0_lola)
	#define P_IncEncVelElmo_fb_Joint1_IDN		EIDNPS(EIDN_P,P_IncEncVelElmo_fb_IDN_OFFSET,0,S_ELEM_1_lola)
	#define P_IncEncVelElmo_fb_Joint2_IDN		EIDNPS(EIDN_P,P_IncEncVelElmo_fb_IDN_OFFSET,0,S_ELEM_2_lola)

//++++++++++++++++++++++++++++++++++++++++++++++++++++
//Incremental Encoder database Offset
#define P_IncEnc_cmn_IDN_OFFSET	100
	// Paramters Elements
	#define P_IncEnc_cmn_Joint0_IDN	EIDNPS(EIDN_P,P_IncEnc_cmn_IDN_OFFSET,0,S_ELEM_0_lola)
	#define P_IncEnc_cmn_Joint1_IDN	EIDNPS(EIDN_P,P_IncEnc_cmn_IDN_OFFSET,0,S_ELEM_1_lola)
	#define P_IncEnc_cmn_Joint2_IDN	EIDNPS(EIDN_P,P_IncEnc_cmn_IDN_OFFSET,0,S_ELEM_2_lola)

#define P_IncEnc_fb_IDN_OFFSET	110
	// Paramters Elements
	#define P_IncEnc_fb_Joint0_IDN		EIDNPS(EIDN_P,P_IncEnc_fb_IDN_OFFSET,0,S_ELEM_0_lola)
	#define P_IncEnc_fb_Joint1_IDN		EIDNPS(EIDN_P,P_IncEnc_fb_IDN_OFFSET,0,S_ELEM_1_lola)
	#define P_IncEnc_fb_Joint2_IDN		EIDNPS(EIDN_P,P_IncEnc_fb_IDN_OFFSET,0,S_ELEM_2_lola)
	#define P_IncEnc_offset_Joint0_IDN	EIDNPS(EIDN_P,P_IncEnc_fb_IDN_OFFSET,0,S_ELEM_3_lola)
	#define P_IncEnc_offset_Joint1_IDN	EIDNPS(EIDN_P,P_IncEnc_fb_IDN_OFFSET,0,S_ELEM_4_lola)
	#define P_IncEnc_offset_Joint2_IDN	EIDNPS(EIDN_P,P_IncEnc_fb_IDN_OFFSET,0,S_ELEM_5_lola)

#define P_IncEncVel_fb_IDN_OFFSET	115
	// Paramters Elements
	#define P_IncEncVel_fb_Joint0_IDN		EIDNPS(EIDN_P,P_IncEncVel_fb_IDN_OFFSET,0,S_ELEM_0_lola)
	#define P_IncEncVel_fb_Joint1_IDN		EIDNPS(EIDN_P,P_IncEncVel_fb_IDN_OFFSET,0,S_ELEM_1_lola)
	#define P_IncEncVel_fb_Joint2_IDN		EIDNPS(EIDN_P,P_IncEncVel_fb_IDN_OFFSET,0,S_ELEM_2_lola)

#define P_IncEncVelElmo_fb_IDN_OFFSET	116
	// Paramters Elements
	#define P_IncEncVelElmo_fb_Joint0_IDN		EIDNPS(EIDN_P,P_IncEncVelElmo_fb_IDN_OFFSET,0,S_ELEM_0_lola)
	#define P_IncEncVelElmo_fb_Joint1_IDN		EIDNPS(EIDN_P,P_IncEncVelElmo_fb_IDN_OFFSET,0,S_ELEM_1_lola)
	#define P_IncEncVelElmo_fb_Joint2_IDN		EIDNPS(EIDN_P,P_IncEncVelElmo_fb_IDN_OFFSET,0,S_ELEM_2_lola)
//++++++++++++++++++++++++++++++++++++++++++++++++++++
//EnDat Encoder database Offset
#define P_EnDat_cmn_IDN_OFFSET	120
	// Paramters Elements
	#define P_EnDat_cmn_Joint0_IDN	EIDNPS(EIDN_P,P_EnDat_cmn_IDN_OFFSET,0,S_ELEM_0_lola)
	#define P_EnDat_cmn_Joint1_IDN	EIDNPS(EIDN_P,P_EnDat_cmn_IDN_OFFSET,0,S_ELEM_1_lola)
	#define P_EnDat_cmn_Joint2_IDN	EIDNPS(EIDN_P,P_EnDat_cmn_IDN_OFFSET,0,S_ELEM_2_lola)
	
	
#define P_EnDat_fb_IDN_OFFSET	130
	// Paramters Elements
	#define P_EnDat_fb_Joint0_IDN		EIDNPS(EIDN_P,P_EnDat_fb_IDN_OFFSET,0,S_ELEM_0_lola)
	#define P_EnDat_fb_Joint1_IDN		EIDNPS(EIDN_P,P_EnDat_fb_IDN_OFFSET,0,S_ELEM_1_lola)
	#define P_EnDat_fb_Joint2_IDN		EIDNPS(EIDN_P,P_EnDat_fb_IDN_OFFSET,0,S_ELEM_2_lola)
	#define P_EnDat_offset_Joint0_IDN	EIDNPS(EIDN_P,P_EnDat_fb_IDN_OFFSET,0,S_ELEM_3_lola)
	#define P_EnDat_offset_Joint1_IDN	EIDNPS(EIDN_P,P_EnDat_fb_IDN_OFFSET,0,S_ELEM_4_lola)
	#define P_EnDat_offset_Joint2_IDN	EIDNPS(EIDN_P,P_EnDat_fb_IDN_OFFSET,0,S_ELEM_5_lola)

//++++++++++++++++++++++++++++++++++++++++++++++++++++
//Force Torque Sensor database Offset	:: DSP_4 & DSP_7
#define P_FTS_IDN_OFFSET		140
	// Paramters Elements
	#define P_FTS_fx_IDN		EIDNPS(EIDN_P,P_FTS_IDN_OFFSET,0,S_ELEM_0_lola)
	#define P_FTS_fy_IDN		EIDNPS(EIDN_P,P_FTS_IDN_OFFSET,0,S_ELEM_1_lola)
	#define P_FTS_fz_IDN		EIDNPS(EIDN_P,P_FTS_IDN_OFFSET,0,S_ELEM_2_lola)
	#define P_FTS_Mx_IDN		EIDNPS(EIDN_P,P_FTS_IDN_OFFSET,0,S_ELEM_3_lola)
	#define P_FTS_My_IDN		EIDNPS(EIDN_P,P_FTS_IDN_OFFSET,0,S_ELEM_4_lola)
	#define P_FTS_Mz_IDN		EIDNPS(EIDN_P,P_FTS_IDN_OFFSET,0,S_ELEM_5_lola)

	
	#define P_FTS_CS_IDN		EIDNPS(EIDN_P,P_FTS_IDN_OFFSET,0,S_ELEM_6_lola)

//++++++++++++++++++++++++++++++++++++++++++++++++++++
//Force Torque Sensor database Offset	:: DSP_4 & DSP_7
#define P_SpindelDef_adc_IDN_OFFSET		145
	// Paramters Elements
	#define P_SpindelDef_adc_0_IDN		EIDNPS(EIDN_P,P_SpindelDef_adc_IDN_OFFSET,0,S_ELEM_0_lola)
	#define P_SpindelDef_adc_1_IDN		EIDNPS(EIDN_P,P_SpindelDef_adc_IDN_OFFSET,0,S_ELEM_1_lola)


//++++++++++++++++++++++++++++++++++++++++++++++++++++
//IMU database Offset					:: DSP_6
#define P_IMU_IDN_OFFSET		150
	// Paramters Elements
	#define P_IMU_nrpy_x_IDN		EIDNPS(EIDN_P,P_IMU_IDN_OFFSET,0,S_ELEM_0_lola)
	#define P_IMU_nrpy_y_IDN		EIDNPS(EIDN_P,P_IMU_IDN_OFFSET,0,S_ELEM_1_lola)
	#define P_IMU_nrpy_z_IDN		EIDNPS(EIDN_P,P_IMU_IDN_OFFSET,0,S_ELEM_2_lola)
	#define P_IMU_omgs_x_IDN		EIDNPS(EIDN_P,P_IMU_IDN_OFFSET,0,S_ELEM_3_lola)
	#define P_IMU_omgs_y_IDN		EIDNPS(EIDN_P,P_IMU_IDN_OFFSET,0,S_ELEM_4_lola)
	#define P_IMU_omgs_z_IDN		EIDNPS(EIDN_P,P_IMU_IDN_OFFSET,0,S_ELEM_5_lola)
	#define P_IMU_accs_x_IDN		EIDNPS(EIDN_P,P_IMU_IDN_OFFSET,0,S_ELEM_6_lola)
	#define P_IMU_accs_y_IDN		EIDNPS(EIDN_P,P_IMU_IDN_OFFSET,0,S_ELEM_7_lola)
	#define P_IMU_accs_z_IDN		EIDNPS(EIDN_P,P_IMU_IDN_OFFSET,0,S_ELEM_8_lola)
	#define P_IMU_status_IDN		EIDNPS(EIDN_P,P_IMU_IDN_OFFSET,0,S_ELEM_9_lola)


//++++++++++++++++++++++++++++++++++++++++++++++++++++
//Controller Parameters database Offset
#define P_Controller_Param_IDN_OFFSET	160
	// Paramters Elements
	#define P_Controller_Param_GGAIN_IDN					EIDNPS(EIDN_P,P_Controller_Param_IDN_OFFSET,0,S_ELEM_0_lola)

#define P_Controller_Param_Position_P_IDN_OFFSET	(P_Controller_Param_IDN_OFFSET+0x01)
	// Paramters Elements
	#define P_Controller_Param_Position_KP_Joint_0_IDN		EIDNPS(EIDN_P,P_Controller_Param_Position_P_IDN_OFFSET,0,S_ELEM_0_lola)
	#define P_Controller_Param_Position_KP_Joint_1_IDN		EIDNPS(EIDN_P,P_Controller_Param_Position_P_IDN_OFFSET,0,S_ELEM_1_lola)
	#define P_Controller_Param_Position_KP_Joint_2_IDN		EIDNPS(EIDN_P,P_Controller_Param_Position_P_IDN_OFFSET,0,S_ELEM_2_lola)

#define P_Controller_Param_Position_D_IDN_OFFSET	(P_Controller_Param_IDN_OFFSET+0x02)
	// Paramters Elements
	#define P_Controller_Param_Position_KD_Joint_0_IDN		EIDNPS(EIDN_P,P_Controller_Param_Position_D_IDN_OFFSET,0,S_ELEM_0_lola)
	#define P_Controller_Param_Position_KD_Joint_1_IDN		EIDNPS(EIDN_P,P_Controller_Param_Position_D_IDN_OFFSET,0,S_ELEM_1_lola)
	#define P_Controller_Param_Position_KD_Joint_2_IDN		EIDNPS(EIDN_P,P_Controller_Param_Position_D_IDN_OFFSET,0,S_ELEM_2_lola)
	#define P_Controller_Param_Position_LPF_Joint_0_IDN		EIDNPS(EIDN_P,P_Controller_Param_Position_D_IDN_OFFSET,0,S_ELEM_3_lola)
	#define P_Controller_Param_Position_LPF_Joint_1_IDN		EIDNPS(EIDN_P,P_Controller_Param_Position_D_IDN_OFFSET,0,S_ELEM_4_lola)
	#define P_Controller_Param_Position_LPF_Joint_2_IDN		EIDNPS(EIDN_P,P_Controller_Param_Position_D_IDN_OFFSET,0,S_ELEM_5_lola)
	
#define P_Controller_Param_Position_FF_IDN_OFFSET	(P_Controller_Param_IDN_OFFSET+0x03)
	// Paramters Elements
	#define P_Controller_Param_Position_FF_Joint_0_IDN		EIDNPS(EIDN_P,P_Controller_Param_Position_FF_IDN_OFFSET,0,S_ELEM_0_lola)
	#define P_Controller_Param_Position_FF_Joint_1_IDN		EIDNPS(EIDN_P,P_Controller_Param_Position_FF_IDN_OFFSET,0,S_ELEM_1_lola)
	#define P_Controller_Param_Position_FF_Joint_2_IDN		EIDNPS(EIDN_P,P_Controller_Param_Position_FF_IDN_OFFSET,0,S_ELEM_2_lola)
	#define P_Controller_Param_Position_FFGain_Joint_0_IDN	EIDNPS(EIDN_P,P_Controller_Param_Position_FF_IDN_OFFSET,0,S_ELEM_3_lola)
	#define P_Controller_Param_Position_FFGain_Joint_1_IDN	EIDNPS(EIDN_P,P_Controller_Param_Position_FF_IDN_OFFSET,0,S_ELEM_4_lola)
	#define P_Controller_Param_Position_FFGain_Joint_2_IDN	EIDNPS(EIDN_P,P_Controller_Param_Position_FF_IDN_OFFSET,0,S_ELEM_5_lola)
	#define P_Controller_Param_Position_FFMult_IDN	        EIDNPS(EIDN_P,P_Controller_Param_Position_FF_IDN_OFFSET,0,S_ELEM_6_lola)
	
#define P_Controller_Param_Position_Error_IncEnc_IDN_OFFSET	(P_Controller_Param_IDN_OFFSET+0x04)
	// Paramters Elements
	#define P_Controller_Param_Position_err_IncEnd_Joint_0_IDN		EIDNPS(EIDN_P,P_Controller_Param_Position_Error_IncEnc_IDN_OFFSET,0,S_ELEM_0_lola)
	#define P_Controller_Param_Position_err_IncEnd_Joint_1_IDN		EIDNPS(EIDN_P,P_Controller_Param_Position_Error_IncEnc_IDN_OFFSET,0,S_ELEM_1_lola)
	#define P_Controller_Param_Position_err_IncEnd_Joint_2_IDN 		EIDNPS(EIDN_P,P_Controller_Param_Position_Error_IncEnc_IDN_OFFSET,0,S_ELEM_2_lola)
	
#define P_Controller_Param_Position_Error_EnDat_IDN_OFFSET	(P_Controller_Param_IDN_OFFSET+0x05)
	// Paramters Elements
	#define P_Controller_Param_Position_err_EnDat_Joint_0_IDN		EIDNPS(EIDN_P,P_Controller_Param_Position_Error_EnDat_IDN_OFFSET,0,S_ELEM_0_lola)
	#define P_Controller_Param_Position_err_EnDat_Joint_1_IDN		EIDNPS(EIDN_P,P_Controller_Param_Position_Error_EnDat_IDN_OFFSET,0,S_ELEM_1_lola)
	#define P_Controller_Param_Position_err_EnDat_Joint_2_IDN		EIDNPS(EIDN_P,P_Controller_Param_Position_Error_EnDat_IDN_OFFSET,0,S_ELEM_2_lola)


//++++++++++++++++++++++++++++++++++++++++++++++++++++
//Current Command database Offset
#define P_Current_cmn_IDN_OFFSET		180
	// Paramters Elements
	#define P_Current_cmn_Joint_0_IDN		EIDNPS(EIDN_P,P_Current_cmn_IDN_OFFSET,0,S_ELEM_0_lola)
	#define P_Current_cmn_Joint_1_IDN		EIDNPS(EIDN_P,P_Current_cmn_IDN_OFFSET,0,S_ELEM_1_lola)
	#define P_Current_cmn_Joint_2_IDN		EIDNPS(EIDN_P,P_Current_cmn_IDN_OFFSET,0,S_ELEM_2_lola)

//Current Feedback database Offset
#define P_Current_fb_IDN_OFFSET			181
	// Paramters Elements
	#define P_Current_fb_Joint_0_IDN		EIDNPS(EIDN_P,P_Current_fb_IDN_OFFSET,0,S_ELEM_0_lola)
	#define P_Current_fb_Joint_1_IDN		EIDNPS(EIDN_P,P_Current_fb_IDN_OFFSET,0,S_ELEM_1_lola)
	#define P_Current_fb_Joint_2_IDN		EIDNPS(EIDN_P,P_Current_fb_IDN_OFFSET,0,S_ELEM_2_lola)

//Current Feedback database Offset
#define P_Current_RatedCurrent_IDN_OFFSET   182
	// Paramters Elements
	#define P_Current_RatedCurrent_Joint_0_IDN		EIDNPS(EIDN_P,P_Current_RatedCurrent_IDN_OFFSET,0,S_ELEM_0_lola)
	#define P_Current_RatedCurrent_Joint_1_IDN		EIDNPS(EIDN_P,P_Current_RatedCurrent_IDN_OFFSET,0,S_ELEM_1_lola)
	#define P_Current_RatedCurrent_Joint_2_IDN		EIDNPS(EIDN_P,P_Current_RatedCurrent_IDN_OFFSET,0,S_ELEM_2_lola)

//++++++++++++++++++++++++++++++++++++++++++++++++++++
//DSP state over SVC database Offset
#define P_DSP_STATE_SERCOS_IDN_OFFSET			500
	// Paramters Elements
	#define P_DSP_STATE_SERCOS_IDN					EIDNPS(EIDN_P,P_DSP_STATE_SERCOS_IDN_OFFSET,0,S_ELEM_0_lola)
	

//DSP command over SVC database Offset
#define P_DSP_COMMAND_SVC_SERCOS_IDN_OFFSET		600
	// Paramters Elements
	#define P_DSP_COMMAND_SVC_SERCOS_IDN			EIDNPS(EIDN_P,P_DSP_COMMAND_SVC_SERCOS_IDN_OFFSET,0,S_ELEM_0_lola)


//++++++++++++++++++++++++++++++++++++++++++++++++++++
// Error class 1 Shutdown
#define S_DSP_ManC1D_IDN_OFFSET			        129
	// Paramters Elements
	#define S_DSP_ManC1D_IDN			        EIDNPS(EIDN_S,S_DSP_ManC1D_IDN_OFFSET,0,S_ELEM_0_lola)
	
//DSP error extra data over SVC database Offset
#define P_DSP_Error_ExtraDATA_IDN_OFFSET		1290
	// Paramters Elements
	#define P_DSP_Error_ExtraDATA_1_IDN		EIDNPS(EIDN_P,P_DSP_Error_ExtraDATA_IDN_OFFSET,0,S_ELEM_0_lola)
	#define P_DSP_Error_Buffer__IDN			EIDNPS(EIDN_P,P_DSP_Error_ExtraDATA_IDN_OFFSET,0,S_ELEM_1_lola)         // "__FILE__ ; __LINE__"

//++++++++++++++++++++++++++++++++++++++++++++++++++++
// Warning class 2 diagnostic
#define S_DSP_ManC2D_IDN_OFFSET			        181
	// Paramters Elements
	#define S_DSP_ManC2D_IDN			        EIDNPS(EIDN_S,S_DSP_ManC2D_IDN_OFFSET,0,S_ELEM_0_lola)
	
//++++++++++++++++++++++++++++++++++++++++++++++++++++
//DSP iFaceSercos version (HWord:IfaceSercosVersion :: LWord:DSP_SWVersion)
#define P_DSP_SW_iFaceSercos_IDN_OFFSET			4000
	// Paramters Elements
	#define P_DSP_SW_iFaceSercos_IDN		EIDNPS(EIDN_P,P_DSP_SW_iFaceSercos_IDN_OFFSET,0,S_ELEM_0_lola)
//++++++++++++++++++++++++++++++++++++++++++++++++++++
//DSP Cycle Timer
#define P_DSP_CYCLE_TIMER_IDN_OFFSET			2050
	// Paramters Elements
    //==> DSPTimerData
	#define P_DSP_Cycle_Timer_IDN			EIDNPS(EIDN_P,P_DSP_CYCLE_TIMER_IDN_OFFSET ,0,S_ELEM_0_lola)
	#define P_DSP_IRQ_Timer_IDN				EIDNPS(EIDN_P,P_DSP_CYCLE_TIMER_IDN_OFFSET ,0,S_ELEM_1_lola)
	#define P_DSP_Controller_Timer_IDN		EIDNPS(EIDN_P,P_DSP_CYCLE_TIMER_IDN_OFFSET ,0,S_ELEM_2_lola)

//--------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------
/*------------------------------------------------------------------------------------------------*/
// Operation Modes
/*------------------------------------------------------------------------------------------------*/
///	1st Operation mode 	--> DSP_IDLE							:0x0000
/// 2nd Operation mode 1--> DSP_MFREE							:0x8000
/// 2nd Operation mode 2--> DSP_TCONT							:0x0001
/// 2nd Operation mode 3--> DSP_VCONT							:0x0002
/// 2nd Operation mode 4--> DSP_PCONT Motor Encoder				:0x0003
/// 2nd Operation mode 5--> DSP_PCONT EnDat						:0x0004
/// 2nd Operation mode 6--> DSP_PCONT Motor Encoder & EnDat		:0x0005
/// 2nd Operation mode 7--> Fault Mode							:0x8001

//SerocsDB Offset for S parameters
#define S_PrimaryOperationMode			EIDNPS(EIDN_S,32,0,0)
#define S_SecondaryOperationMode1		EIDNPS(EIDN_S,33,0,0)
#define S_SecondaryOperationMode2		EIDNPS(EIDN_S,34,0,0)
#define S_SecondaryOperationMode3		EIDNPS(EIDN_S,35,0,0)
#define S_SecondaryOperationMode4		EIDNPS(EIDN_S,284,0,0)
#define S_SecondaryOperationMode5		EIDNPS(EIDN_S,285,0,0)
#define S_SecondaryOperationMode6		EIDNPS(EIDN_S,286,0,0)
#define S_SecondaryOperationMode7		EIDNPS(EIDN_S,287,0,0)

#define S_OPMODE_DSP_IDLE             S_PrimaryOperationMode
#define S_OPMODE_DSP_MFREE            S_SecondaryOperationMode1
#define S_OPMODE_DSP_TCONT            S_SecondaryOperationMode2
#define S_OPMODE_DSP_VCONT            S_SecondaryOperationMode3
#define S_OPMODE_DSP_PCONT_MENC       S_SecondaryOperationMode4
#define S_OPMODE_DSP_PCONT_JENC       S_SecondaryOperationMode5
#define S_OPMODE_DSP_PCONT_MENC_JENC  S_SecondaryOperationMode6
#define S_OPMODE_DSP_FAULT            S_SecondaryOperationMode7

// Sercos Control Word
#define S_DriveControlWord				EIDNPS(EIDN_S,134,0,0)

// Sercos Status Word
#define S_DriveStatusWord				EIDNPS(EIDN_S,135,0,0)

// Sercos Error
#define S_ManufactorClass1Diagnostic	EIDNPS(EIDN_S,129,0,0)


#endif//SD_LOLA_uEIDN__H_
