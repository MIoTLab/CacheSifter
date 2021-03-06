

/*****************************************************************************/
/*                                                                           */
/*                Copyright 1999 - 2003, Huawei Tech. Co., Ltd.              */
/*                           ALL RIGHTS RESERVED                             */
/*                                                                           */
/* FileName: APMInterface.h                                                  */
/*                                                                           */
/*    Author: Jiang KaiBo                                                    */
/*            Xu Cheng                                                       */
/*                                                                           */
/* Version: 1.0                                                              */
/*                                                                           */
/* Date: 2008-02                                                             */
/*                                                                           */
/* Description: process data of APM mailbox                                  */
/*                                                                           */
/* Others:                                                                   */
/*                                                                           */
/* History:                                                                  */
/* 1. Date:                                                                  */
/*    Author:                                                                */
/*    Modification: Create this file                                         */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/


#ifndef  _APM_INTERFACE_H
#define  _APM_INTERFACE_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#include "vos.h"
#include "phyoaminterface.h"

/*---------------------------------------------*
 * ??????????                                  *
 *---------------------------------------------*/

/*????DSP NV????????*/
#define SHPA_TMR_LOAD_PHY_INTERVAL          5000

#define DSP_DATA_WRITE_OK                   0x5555AAAA
#define DSP_DATA_READ_OK                    0x5A5A5A5A

/*????DSP????*/
#define CPHY_DSP_LOAD_SUCCESS               0
#define CPHY_DSP_LOAD_FAILURE               1
#define CPHY_DSP_LOAD_TIMEOUT               2

/* ?????????????????? */
#define SHPA_PROTECT_DELAY_COUNT            (30)

/* ??????DSP???????????? */
#define COMM_EXT_DATA_MEM_PROTECT_BIT_1     0xAAAA
#define COMM_EXT_DATA_MEM_PROTECT_BIT_2     0x5555
#define COMM_EXT_DATA_MEM_PROTECT_BIT_3     0x5A5A
#define COMM_EXT_DATA_MEM_PROTECT_BIT_4     0x5A5A
#define COMM_EXT_DATA_MEM_PROTECT_BIT_5     0xA5A5
#define COMM_EXT_DATA_MEM_PROTECT_BIT_6     0xA5A5
#define COMM_EXT_DATA_MEM_PROTECT_BIT_7     0x5555
#define COMM_EXT_DATA_MEM_PROTECT_BIT_8     0xAAAA

#define SHPA_LDF_DELAY_TIME                 (340)   /* ???????????????????????????? 10ms,????NMI ??????????DTCM??ARM???? */

#define SHPA_LDF_DELAY_1S_TIME              (34)   /* ???????????????????????????? 1ms,????NMI ??????????DTCM??ARM???? */

#define SHPA_LDF_LOAD_PHY_DELAY_TIME        (400)   /* ????DSP??????DSP?????????????? 400ms */

#ifdef  __LDF_FUNCTION__
#define SHPA_Ldf_Check()                    UPHY_MNTN_PhyLdfFileSaving()
#endif

enum DSP_CONFIG_ERROR_EUNM
{
    DSP_NVREAD_NOERROR = 0,
    DSP_BOOTINFO_MEMERR,
    DSP_NVREAD_LENERR,
    DSP_NVREAD_READNVERR,
    DSP_NVREAD_ALLOCMEMERR,
    DSP_NVREAD_TOTALLENERR,
    DSP_LOAD_STATIC_ERROR,
    DSP_GET_COMMON_INFO_ERROR,
    DSP_CONFIG_INFO_TOO_LONG,
    DSP_ACTIVE_TIME_OUT,
    DSP_CONFIG_ERROR_BUTT
};
typedef VOS_UINT32 DSP_CONFIG_ERROR_EUNM_UINT32;

enum APM_CFG_STATE_EUNM
{
    APM_INIT_STATE   = 0,
    APM_SENDED_MSG,
    APM_RECEIVED_MSG,
    APM_CFG_STATE_BUTT
};
typedef VOS_UINT32 APM_CFG_STATE_EUNM_UINT32;

/*****************************************************************************
 ??????    :DUALMODEM_SCENE_TYPE_ENUM_UINT16
 ????????  :
 ????????  :????????CCPU DDR????????????
*****************************************************************************/
enum
{
   DUALMODEM_SCENCE_GSM,
   DUALMODEM_SCENCE_WCDMA,
   DUALMODEM_SCENCE_APS,
   DUALMODEM_SCENCE_TL,
   DUALMODEM_SCENCE_GUDRX,
   DUALMODEM_SCENCE_BUTT,
};
typedef VOS_UINT16 DUALMODEM_SCENE_TYPE_ENUM_UINT16;

typedef struct
{
    VOS_UINT32                      ulDspNVConfigAddr;          /*??DSP??????????DSP NV????????????*/
    VOS_UINT32                      ulDspNvTotalLen;            /*??????????DSP??NV????????*/
    VOS_UINT16                      *pusNVData;                 /*????NV??????????????*/
    DSP_CONFIG_ERROR_EUNM_UINT32    enErrorCode;                /*????????DSP??????????*/
    APM_CFG_STATE_EUNM_UINT32       enActiveState;
    APM_CFG_STATE_EUNM_UINT32       enLoadState;
    VOS_UINT32                      ulActiveResult;
    VOS_UINT32                      ulLoadResult;
    VOS_UINT32                      ulActiveSmPResult;          /*????Active????????????*/
    VOS_UINT32                      ulLoadSmPResult;            /*????Load????????????*/
    VOS_UINT32                      ulGetNvLenErrId;            /*????NV??????????NV ID??*/
    VOS_UINT32                      ulGetNvErrId;               /*????NV??????????NV ID??*/
    /*????????????????????*/
    VOS_UINT32                      ulGetNvLenSlice;            /*????NV????????????*/
    VOS_UINT32                      ulRecNVReqSlice;            /*????DSP NV??????????*/
    VOS_UINT32                      ulConfigNVSlice;            /*????dSP NV??????*/
    VOS_UINT32                      ulDSPActiveSlice;           /*Active DSP????????*/
    VOS_UINT32                      ulDSPActCnfSlice;           /*????Active????????????*/
    VOS_UINT32                      ulDSPLoadSlice;             /*Load DSP????????*/
    VOS_UINT32                      ulDSPLoadCnfSlice;          /*????Load????????????*/
}DSP_CONFIG_CTRL_STRU;

/*????OM??????????*/
typedef struct
{
    VOS_MSG_HEADER                       /* ????????????TOOL_ID??PS??PID,??????????????????????PS???? */
    VOS_UINT16      usTransPrimId;       /* ????????0x5001*/
    VOS_UINT16      usReserve;
    VOS_UINT8       ucFuncType;          /*????????04,????????????:02*/
    VOS_UINT8       ucReserve;
    VOS_UINT16      usAppLength;         /*??????????????????????*/
    VOS_UINT32      ulSn;                /* ???????????????? */
    VOS_UINT32      ulTimeStamp;         /*??????????????????????SOC Tick??*/
    VOS_UINT16      usPrimId;            /* ????ID*/
    VOS_UINT16      usToolsId;           /* ????ID */
    VOS_UINT8       aucData[4];
}PS_APM_OM_TRANS_IND_STRU;

typedef struct
{
    VOS_UINT16 *pusArray;
    VOS_UINT32 ulNum;
}APM_RF_NV_INFO_STRU;

typedef struct
{
    VOS_UINT32 ulDSPPowerDownState;
    VOS_UINT32 ulReserved[7];
}LDF_EXT_INFO_STRU;

/*---------------------------------------------*
 * ????????                                    *
 *---------------------------------------------*/

extern VOS_SEM g_aulShpaLoadPhySemaphor[MODEM_ID_BUTT];

extern VOS_SEM g_aulShpaActivePhySemaphor[MODEM_ID_BUTT];

extern VOS_UINT32 g_aulShpaCurSysId[MODEM_ID_BUTT];

extern DSP_CONFIG_CTRL_STRU g_astDspConfigCtrl[MODEM_ID_BUTT];

extern VOS_UINT16 SHPA_LoadPhy( PHY_OAM_SET_WORK_MODE_REQ_STRU  stSetWorkMode );

extern VOS_UINT32 APM_PIDInit( enum VOS_INIT_PHASE_DEFINE ip );

extern VOS_VOID SHPA_ActivateHw(VOS_VOID);

extern VOS_VOID SHPA_DeactivateHw(VOS_VOID);

extern VOS_UINT32 SHPA_ActiveDSP(VOS_RATMODE_ENUM_UINT32 enRatMode, MODEM_ID_ENUM_UINT16 enModemId);

extern VOS_UINT32 SHPA_ReadWTxPower( VOS_VOID);

extern VOS_UINT32 GHPA_GetRealFN(MODEM_ID_ENUM_UINT16 enModemID);

/*****************************************************************************
 ?? ?? ??  : DualModem_DDRQosUpdate
 ????????  : CCPU????????DDR????????
 ????????  : MODEM_ID_ENUM_UINT16 enModem                       ????Modem Id
             DUALMODEM_SCENE_TYPE_ENUM_UINT16   enSceneType     ????????????
             VOS_UINT32 ulValue                                 DDR????????????
 ????????  : ??
 ?? ?? ??  : VOS_UINT32
 ????????  :
 ????????  :

 ????????      :
  1.??    ??   : 2014??8??11??
    ??    ??   : s00207770
    ????????   : ??????????

*****************************************************************************/
extern VOS_UINT32 DualModem_DDRQosUpdate(MODEM_ID_ENUM_UINT16 enModem, DUALMODEM_SCENE_TYPE_ENUM_UINT16 enSceneType,  VOS_UINT32 ulValue);

#ifdef  __LDF_FUNCTION__
VOS_UINT32 SHPA_Ldf_Hifi_Saving(VOS_VOID);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* _APM_INTERFACE_H */

