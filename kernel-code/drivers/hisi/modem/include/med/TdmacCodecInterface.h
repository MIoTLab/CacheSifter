/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and 
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may 
 * *    be used to endorse or promote products derived from this software 
 * *    without specific prior written permission.
 * 
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */


#ifndef __TDMACCODECINTERFACE_H__
#define __TDMACCODECINTERFACE_H__

#include "vos.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 ??????????????
*****************************************************************************/
#pragma pack(4)

/*****************************************************************************
  2 ??????
*****************************************************************************/
#define TDMACVOICE_AMR_CODEC_MODE_NUM    (8)             /* AMR-NB????????????CODEC_MODE ????????8??????????SID */

/*****************************************************************************
  3 ????????
*****************************************************************************/
enum TDMACVOICE_AMR_CODECMODE_TYPE_ENUM
{
    TDMACVOICE_AMR_CODECMODE_TYPE_475K          = 0,     /* 4.75K????????: class a 42bit, b 53bit */
    TDMACVOICE_AMR_CODECMODE_TYPE_515K          = 1,     /* 5.15K????????: class a 49bit, b 54bit */
    TDMACVOICE_AMR_CODECMODE_TYPE_590K          = 2,     /* 5.9 K????????: class a 55bit, b 63bit */
    TDMACVOICE_AMR_CODECMODE_TYPE_670K          = 3,     /* 6.7 K????????: class a 58bit, b 76bit */
    TDMACVOICE_AMR_CODECMODE_TYPE_740K          = 4,     /* 7.4 K????????: class a 61bit, b 87bit */
    TDMACVOICE_AMR_CODECMODE_TYPE_795K          = 5,     /* 7.95K????????: class a 75bit, b 84bit */
    TDMACVOICE_AMR_CODECMODE_TYPE_102K          = 6,     /* 10.2K????????: class a 65bit, b 99bit , c 40bit */
    TDMACVOICE_AMR_CODECMODE_TYPE_122K          = 7,     /* 12.2K????????: class a 81bit, b 103bit, c 60bit */
    TDMACVOICE_AMR_CODECMODE_TYPE_BUTT
};
typedef  VOS_UINT16 TDMACVOICE_AMR_CODECMODE_TYPE_ENUM_UINT16;

/*****************************************************************************
 ????????  : TDMACVOICE_AMR_TYPE_ENUM
 ????????  : AMR??????????, ????3GPP TS 26.101
*****************************************************************************/
enum TDMACVOICE_AMR_TYPE_ENUM
{
    TDMACVOICE_AMR_TYPE_AMR2               = 0,
    TDMACVOICE_AMR_TYPE_AMR                = 1,
    TDMACVOICE_AMR_TYPE_BUTT
};
typedef VOS_UINT16 TDMACVOICE_AMR_TYPE_ENUM_UINT16;

/*****************************************************************************
 ????????  : TDMACVOICE_AMR_BANDWIDTH_TYPE_ENUM
 ????????  : AMR????????, ????3GPP TS 26.101
*****************************************************************************/
enum TDMACVOICE_AMR_BANDWIDTH_TYPE_ENUM
{
    TDMACVOICE_AMR_BANDWIDTH_TYPE_NB       = 0,
    TDMACVOICE_AMR_BANDWIDTH_TYPE_BUTT
};
typedef VOS_UINT16 TDMACVOICE_AMR_BANDWIDTH_TYPE_ENUM_UINT16;

/*****************************************************************************
 ????????  : TDMACVOICE_RSLT_ENUM
 ????????  : TDMAC??CODEC??????????????????
*****************************************************************************/
enum TDMACVOICE_RSLT_ENUM
{
    TDMACVOICE_RSLT_SUCC             = 0,                                    /* ??????MAC, ???? */
    TDMACVOICE_RSLT_FAIL             = 1,                                    /* ??????MAC, ???? */
    TDMACVOICE_RSLT_BUTT
};
typedef VOS_UINT16 TDMACVOICE_RSLT_ENUM_UINT16;

/*****************************************************************************
  4 ????????????
*****************************************************************************/


/*****************************************************************************
  5 ??????????
*****************************************************************************/
enum TDMACVOICE_MSG_TYPE_ENUM
{
    ID_TDMAC_VOICE_MODE_SET_IND         = 0x9460,           /* _H2ASN_MsgChoice TDMACVOICE_MAC_VOICE_MODE_SET_IND_STRU */
    ID_VOICE_TDMAC_MODE_SET_RSP         = 0x3060,           /* _H2ASN_MsgChoice TDMACVOICE_VOICE_MAC_MODE_SET_RSP_STRU */

    ID_TDMAC_VOICE_MODE_CHANGE_IND      = 0x9461,           /* _H2ASN_MsgChoice TDMACVOICE_MAC_VOICE_MODE_CHANGE_IND_STRU */
    ID_VOICE_TDMAC_MODE_CHANGE_RSP      = 0x3061,           /* _H2ASN_MsgChoice TDMACVOICE_VOICE_MAC_MODE_CHANGE_RSP_STRU */

    ID_VOICE_TDMAC_AMR_TYPE_REQ         = 0x3062,           /* _H2ASN_MsgChoice TDMACVOICE_VOICE_MAC_AMR_TYPE_REQ_STRU */

    TDMACVOICE_MSG_TYPE_BUTT
} ;
typedef VOS_UINT16 TDMACVOICE_MSG_TYPE_ENUM_UINT16;


/*****************************************************************************
  6 ????????
*****************************************************************************/


/*****************************************************************************
  7 STRUCT????
*****************************************************************************/

/*****************************************************************************
 ????????  : TDMACVOICE_MAC_VOICE_MODE_SET_IND_STRU
 ????????  : WTTF MAC??????????Codec????????CodecMode??
           ????????????????????ID_RRC_MAC_D_CONFIG_REQ ?? ID_RRC_MAC_TFC_CONTROL_REQ??
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                                  /* ?????? */        /*_H2ASN_Skip*/
    TDMACVOICE_MSG_TYPE_ENUM_UINT16           enMsgName;             /*????????*/ /*_H2ASN_Skip*/
    VOS_UINT16                                usOpId;                /*????????*/
    VOS_UINT16                                usSidEnable;           /*????????SID??[0,1] 0:??????, 1:????*/
    TDMACVOICE_AMR_BANDWIDTH_TYPE_ENUM_UINT16 enCodecType;           /*0: NB*/
    VOS_UINT16                                usCodecModeCnt;        /* AMR ????????????????????????????????enCodecType =NB??????[1..8]*/
    VOS_UINT16                                aenCodecModes[TDMACVOICE_AMR_CODEC_MODE_NUM]; /* AMR ????????????, enCodecType =NB??????????????[0..7] 0: 4.75kbps, 7: 12.2kbps??enCodecType =WB??????????????[0..8] 0: 6.60kbps, 8: 23.85kbps */
    VOS_UINT16                                usReserved;
} TDMACVOICE_MAC_VOICE_MODE_SET_IND_STRU;

/******************************************************************************************************
??????    : TCODEC_MAC_MODE_SET_RSP_STRU
????????  : Codec??????????MAC?? TMAC_VOICE_MODE_SET_IND_STRU ????
******************************************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                                  /* ?????? */        /*_H2ASN_Skip*/
    TDMACVOICE_MSG_TYPE_ENUM_UINT16          enMsgName;             /*????????*/ /*_H2ASN_Skip*/
    VOS_UINT16                               usOpId;                /*????????*/
    TDMACVOICE_RSLT_ENUM_UINT16              enResult;              /*????????[0,1] 0:= PS_SUCC , 1:= PS_FAIL */
    VOS_UINT16                               enCurrMode;            /*????AMR ??????????enCodecType=NB??????[0..7] 0: 4.75kbps, 7: 12.2kbps??enCodecType=WB????????????????[0..8] 0: 6.60kbps, 8: 23.85kbps*/
    TDMACVOICE_AMR_TYPE_ENUM_UINT16          enAmrType;             /*enCodecType=NB????AMR ????????[0,1] 0: AMR2, 1: AMR */
    VOS_UINT16                               ausRsv[1];
} TDMACVOICE_VOICE_MAC_MODE_SET_RSP_STRU;

/******************************************************************************************************
??????    : TDMACVOICE_MAC_VOICE_MODE_CHANGE_IND_STRU
????????  : WTTF MAC??????????Codec??????????????????????????
          ????????????????MAC??TFC????????????????????????
******************************************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                                  /* ?????? */        /*_H2ASN_Skip*/
    TDMACVOICE_MSG_TYPE_ENUM_UINT16             enMsgName;          /*????????*/ /*_H2ASN_Skip*/
    VOS_UINT16                                  usOpId;             /*????????*/
    TDMACVOICE_AMR_BANDWIDTH_TYPE_ENUM_UINT16   enCodecType;        /*0: NB*/
    VOS_UINT16                                  enCurrMode;         /*????AMR ??????????enCodecType=NB??????[0..7] 0: 4.75kbps, 7: 12.2kbps??*/
    VOS_UINT16                                  enTargetMode;       /*????AMR ??????????enCodecType=NB??????[0..7] 0: 4.75kbps, 7: 12.2kbps??*/
    VOS_UINT16                                  ausRsv[1];          /*????*/
} TDMACVOICE_MAC_VOICE_MODE_CHANGE_IND_STRU;

/******************************************************************************************************
??????    : TDMACVOICE_VOICE_MAC_MODE_CHANGE_RSP_STRU
????????  : Codec??????????MAC?? TDMACVOICE_MAC_VOICE_MODE_CHANGE_IND_STRU ????
******************************************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                                              /*_H2ASN_Skip*/
    TDMACVOICE_MSG_TYPE_ENUM_UINT16             enMsgName;          /*????????*/ /*_H2ASN_Skip*/
    VOS_UINT16                                  usOpId;             /*????????*/
    TDMACVOICE_RSLT_ENUM_UINT16                 enResult;           /*????????[0,1] 0:= PS_SUCC , 1:= PS_FAIL */
    VOS_UINT16                                  enCurrMode;         /*????AMR ?????????? enCodecType =NB??????[0..7] 0: 4.75kbps, 7: 12.2kbps??*/
    VOS_UINT16                                  enTargetMode;       /*????AMR ?????????? enCodecType =NB??????[0..7] 0: 4.75kbps, 7: 12.2kbps??*/
    VOS_UINT16                                  ausRsv[1];
} TDMACVOICE_VOICE_MAC_MODE_CHANGE_RSP_STRU;

/******************************************************************************************************
??????    : TDMACVOICE_VOICE_MAC_AMR_TYPE_REQ_STRU
????????  : Codec??????????MAC??????????AMR????
******************************************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                                  /* ?????? */        /*_H2ASN_Skip*/
    TDMACVOICE_MSG_TYPE_ENUM_UINT16             enMsgName;          /*????????*/ /*_H2ASN_Skip*/
    TDMACVOICE_AMR_TYPE_ENUM_UINT16             enAmrType;          /*AMR ????????[0,1] 0: AMR2, 1: AMR */
    VOS_UINT16                                  ausRsv[2];
}TDMACVOICE_VOICE_MAC_AMR_TYPE_REQ_STRU;

/******************************************************************************************************
??????    : TDMACVOICE_VOICE_MAC_AMR_DATA_STRU
????????  : Codec??MAC AMR??????????????
******************************************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                                  /* ?????? */        /*_H2ASN_Skip*/
    TDMACVOICE_MSG_TYPE_ENUM_UINT16             enMsgName;          /*????????*/
    VOS_UINT16                                  usMuteFlag;         /*????????????????????*/
    TDMACVOICE_AMR_BANDWIDTH_TYPE_ENUM_UINT16   enCodecType;        /* 0: NB */
    VOS_UINT16                                  enFrameType;        /* AMR?????????? */
}TDMACVOICE_VOICE_MAC_AMR_DATA_STRU;

/*****************************************************************************
  8 UNION????
*****************************************************************************/


/*****************************************************************************
  9 OTHERS????
*****************************************************************************/
#if 0                                                       /* _H2ASN_Skip */
/*****************************************************************************
  H2ASN??????ASN????????????
*****************************************************************************/

/*****************************************************************************
  H2ASN????????????????
*****************************************************************************/
/* ??????????????ASN????,????L????,??????V???? */
typedef struct
{
    TDMACVOICE_MSG_TYPE_ENUM_UINT16     enMsgID;            /* _H2ASN_MsgChoice_Export TDMACVOICE_MSG_TYPE_ENUM_UINT16 */

    VOS_UINT8                           aucMsgBlock[2];
    /***************************************************************************
        _H2ASN_MsgChoice_When_Comment          TDMACVOICE_MSG_TYPE_ENUM_UINT16
    ****************************************************************************/
}VOICE_TDMACVOICE_TRACE_MSG_DATA;

/* ????????????*/
typedef struct
{
    VOS_UINT32 uwSenderCpuId;
    VOS_UINT32 uwSenderPid;
    VOS_UINT32 uwReceiverCpuId;
    VOS_UINT32 uwReceiverPid;
    VOS_UINT32 uwLength;
    VOICE_TDMACVOICE_TRACE_MSG_DATA       stMsgData;
}VoiceTdmacTrace_MSG;

#endif                                                     /* _H2ASN_Skip */

/*****************************************************************************
  10 ????????
*****************************************************************************/







#if ((VOS_OS_VER == VOS_WIN32) || (VOS_OS_VER == VOS_NUCLEUS))
#pragma pack()
#else
#pragma pack(0)
#endif


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of TdmacCodecInterface.h */



