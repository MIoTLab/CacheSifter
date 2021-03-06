/****************************************************************************

                  版权所有 (C), 2001-2011, 华为技术有限公司

 ****************************************************************************
  文 件 名   : test_asp_cfg.c
  版 本 号   : 初稿
  作    者   : w00262566
  生成日期   : 2015年1月27日
  最近修改   :
  功能描述   : 自动化测试公共函数模块
  函数列表   :
                at_util_reg_read_u32
                at_util_reg_write_u32
                at_util_mem_remap_type
                at_util_reg_test
                at_util_log_file_open
                at_util_log_file_close
                at_util_log
  修改历史   :
  1.日    期   : 2015年1月27日
    作    者   : w00262566
    修改内容   : 创建文件
******************************************************************************/

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include <linux/device.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>

#include "util.h"
#include "core.h"
#include "asp_cfg.h"


/*****************************************************************************
  2 全局变量与宏定义定义
*****************************************************************************/
#define DRV_NAME              "tc_cfg_reg"

static int tc_cfg_probe (struct platform_device *pdev);
static int tc_cfg_remove(struct platform_device *pdev);

static struct at_case_type* tc_cfg_reg;

static const struct of_device_id tc_cfg_of_match[] = {
    { .compatible = DRV_NAME, },
    {},
};

static struct platform_driver tc_cfg_driver = {
    .probe      = tc_cfg_probe,
    .remove     = tc_cfg_remove,
    .driver     = {
        .name   = DRV_NAME,
        .owner  = THIS_MODULE,
        .of_match_table = tc_cfg_of_match,
    },
};


struct at_util_reg_test_type asp_cfg_regs[] = {
  //  {0x0  , WO, 0x00000000, 0x0007FFFF, 0x00000000},                    //R_RST_CTRLEN,     澶栬杞浣嶄娇鑳藉瘎瀛樺櫒
    {0x4  , WO, 0x00000000, 0x00000000, 0x00000000},                    //R_RST_CTRLDIS,    澶栬杞浣嶆挙绂诲瘎瀛樺櫒


    {0xC  , WO, 0x00000000, 0x00ffffff, 0x00000000},                    //R_GATE_EN,        鏃堕挓浣胯兘瀵勫瓨鍣?
    {0x10 , WO, 0x00000000, 0x00ffffff, 0x00000000},                    //R_GATE_DIS,       鏃堕挓绂佹瀵勫瓨鍣?
    {0x14 , RO, 0x00000000, 0x00000000, 0x01000000},                    //R_GATE_CLKEN      鏃堕挓浣胯兘鐘舵?佸瘎瀛樺櫒
    {0x18 , RO, 0x00000000, 0x00000000, 0x7fffbfff},                    //R_GATE_CLKSTAT    鏃堕挓鏈?缁堢姸鎬佸瘎瀛樺櫒     ?


    {0x1C , RW, 0x000000FF, 0x0000007f, 0x0000007f},                    //R_GATE_CLKDIV_EN  鏃堕挓鍒嗛鍣ㄩ棬鎺т娇鑳藉瘎瀛樺櫒
    {0x20 , RW, 0x0147AE14, 0x0147AE14, 0x0147AE14},                    //R_CLK1_DIV        鍒嗛姣旀帶鍒跺瘎瀛樺櫒1
    {0x24 , RW, 0x01179EC9, 0x01179EC9, 0x01179EC9},                    //R_CLK2_DIV        鍒嗛姣旀帶鍒跺瘎瀛樺櫒2
    {0x28 , RW, 0x00001707, 0xffff1707, 0x00001707},                    //R_CLK3_DIV        鍒嗛姣旀帶鍒跺瘎瀛樺櫒3
    {0x2C , RW, 0x00001717, 0xffff1717, 0x00001717},                    //R_CLK4_DIV        鍒嗛姣旀帶鍒跺瘎瀛樺櫒4
    {0x30 , RW, 0x00003F3F, 0xffff3f3f, 0x00003f3f},                    //R_CLK5_DIV        鍒嗛姣旀帶鍒跺瘎瀛樺櫒5
    {0x34 , RW, 0x00003F3F, 0xffff3f3f, 0x00003f3f},                    //R_CLK6_DIV        鍒嗛姣旀帶鍒跺瘎瀛樺櫒6

//    {0x38 , RW, 0x0000000F, 0xffffffff, 0x0000ffff},                    //R_CLK_SEL         鏃堕挓閫夋嫨瀵勫瓨鍣?
 //   {0x3C , RW, 0x00000000, 0x0000ffff, 0x0000ffff},                    //R_DSP_NMI         DSP NMI涓柇浜х敓瀵勫瓨鍣?
    {0x40 , RW, 0x00000000, 0x0000ffff, 0x0000ffff},                    //R_DSP_PRID        DSP PRID璁剧疆瀵勫瓨鍣?
    {0x44 , RW, 0x00000001, 0x00000001, 0x00000001},                    //R_DSP_RUNSTALL    DSP RUNSTALL璁剧疆瀵勫瓨鍣?

    {0x48 , RW, 0x00000000, 0x00000001, 0x00000001},                    //R_DSP_STATVECTORSEL   DSP STATVECTORSEL璁剧疆瀵勫瓨鍣?
    {0x4C , RW, 0x00000000, 0x00000001, 0x00000001},                    //R_DSP_OCDHALTONRESET  DSP OCDHALTONRESET璁剧疆瀵勫瓨鍣?   ?

    {0x50 , RO, 0x00000000, 0xffffffff, 0x00000000},                    //R_DSP_STATUS          DSP 鐘舵?佸瘎瀛樺櫒

    {0x54 , RW, 0x00000000, 0xffffffff, 0x0000ffff},                    //R_DMAC_SEL            DMAC閫氶亾閫夋嫨瀵勫瓨鍣?
    {0x58 , RW, 0x853E4000, 0xffffffff, 0xffffffff},                    //R_BUS_PRIORITY        鎬荤嚎浼樺厛绾ч厤缃瘎瀛樺櫒
    {0x5C , RW, 0x00000000, 0xffff0fff, 0x00000fff},                    //R_CG_EN               鑷姩闂ㄦ帶浣胯兘瀵勫瓨鍣? ?
    {0x60 , RW, 0x00000000, 0xffff0fff, 0x00000fff},                    //R_OCRAM_RET           ocram浣庡姛鑰楅厤缃瘎瀛樺櫒
    {0x64 , RO, 0x00000000, 0xffffffff, 0x0000ffff},                    //R_INTR_NS_INI         闈炲畨鍏ㄤ腑鏂師濮嬬姸鎬佸瘎瀛樺櫒
    {0x68 , RW, 0x00000000, 0x0000000d, 0x0000000d},                    //R_INTR_NS_EN          闈炲畨鍏ㄤ腑鏂娇鑳藉瘎瀛樺櫒
    {0x6C , RO, 0x00000000, 0x0000000f, 0x00000000},                    //R_INTR_NS_MASK        闈炲畨鍏ㄤ腑鏂睆钄藉悗鐘舵?佸瘎瀛樺櫒
    {0x70 , RO, 0x01011111, 0x00000000, 0x0000000f},                    //R_DBG_SET_AHB2AXI     浠樺己妗BG淇″彿璁剧疆瀵勫瓨鍣?   ?
    {0x74 , RO, 0x00000000, 0x00ffffff, 0x00000000},                    //R_DBG_STATUS_AHB2AXI  浠樺己妗BG淇″彿鐘舵?佸瘎瀛樺櫒   ?
    {0x78 , RO, 0x00000000, 0x00000000, 0x00000000},                    //R_DLOCK_BP            鎬荤嚎闃叉寕姝籦ypass瀵勫瓨鍣?  ?


    {0x7C, RO,  0x00000000, 0xffffffff, 0x00000001},                    //R_DSP_BINTERRUPT      涓婃姤鍒癶ifi dsp鐨勪腑鏂俊鍙峰彧璇诲瘎瀛樺櫒
    {0x84, WO,  0x00000000, 0xffff0000, 0x00000000},                    //R_DSP_RAM_RET         ?
    {0x84, RW,  0x00000000, 0x0000ffff, 0x00000000},
    {0x100, WO, 0x0000FFFF, 0xffff0000, 0x0000FFFF},                    //R_TZ_SECURE_N         ?
    {0x100, WO, 0x0000FFFF, 0xffffffff, 0xffffffff},
  //  {0x104, RW, 0x000003FF, 0x000003ff, 0x000003ff},                    //R_OCRAM_R0SIZE        ?
    {0x108, RW, 0x00000000, 0x0000000f, 0x00000000},                    //R_SIO_LOOP_SECURE_N   ?

    {0x10C, RO, 0x00000000, 0x00000000, 0x00000000},                    //R_INTR_S_INI          瀹夊叏涓柇鍘熷鐘舵?佸瘎瀛樺櫒
    {0x110, RW, 0x00000000, 0x00000003, 0x00000000},                    //R_INTR_S_EN           瀹夊叏涓柇浣胯兘瀵勫瓨鍣?
    {0x114, RO, 0x00000000, 0x00000000, 0x00000000},                    //R_INTR_S_MASK         瀹夊叏涓柇灞忚斀鍚庣姸鎬佸瘎瀛樺櫒
    {0x118, RW, 0x00000000, 0x000001fd, 0x00000000},                    //R_DSP_REMAPPING_EN    dsp鍦板潃閲嶆槧灏勪娇鑳藉瘎瀛樺櫒

    {0x11C, RW, 0x00000000, 0xFFF00000, 0x00000000},                    //R_DSP_REMAPPING_SRC_BASE_ADDR DSP 鍦板潃閲嶆槧灏勫瘎瀛樺櫒
    {0x120, RW, 0x00000000, 0xFFF00000, 0x00000000},                    //R_DSP_REMAPPING_DES_BASE_ADDR DSP 鍦板潃閲嶆槧灏勫瘎瀛樺櫒

    {0x124, RW, 0x00000000, 0x000003ff, 0x000003ff},                    //R_DDR_HARQ_REMAP_EN               DDR涓嶩arq Memory remap鎺у埗瀵勫瓨鍣?
    {0x128, RW, 0x00000000, 0xFFFFF000, 0xFFFFF000},                    //R_DDR_HARQ_REMAP_SRC_BASE_ADDR    DDR涓嶩arq Memory remap婧愬熀鍦板潃瀵勫瓨鍣?
    {0x130, RW, 0x00000000, 0x000003FF, 0x000003FF},                    //R_DDR_MMBUF_REMAP_EN              DDR涓嶮MBUF remap鎺у埗瀵勫瓨鍣?
    {0x134, RW, 0x00000000, 0xFFFFF000, 0xFFFFF000},                    //R_DDR_MMBUF_REMAP_SRC_BASE_ADDR   DDR涓嶮MBUF remap婧愬熀鍦板潃瀵勫瓨鍣?
    {0x138, RW, 0x00000000, 0xFFFFF000, 0xFFFFF000},                    //R_DDR_MMBUF_REMAP_DES_BASE_ADDR   DDR涓嶮MBUF remap鐩爣鍩哄湴鍧?瀵勫瓨鍣?


    {0x13C, RW, 0x00000000, 0x000003ff, 0x000003ff},                    //R_DDR_OCRAM_REMAP_EN              DDR涓嶰CRAM remap鎺у埗瀵勫瓨鍣?
    {0x140, RW, 0x00000000, 0xFFFFF000, 0xFFFFF000},                    //R_DDR_OCRAM_REMAP_SRC_BASE_ADDR   DDR涓嶰CRAM remap婧愬熀鍦板潃瀵勫瓨鍣?
    {0x144, RW, 0x00000000, 0xFFFFF000, 0xFFFFF000},                    //R_DDR_OCRAM_REMAP_DES_BASE_ADDR   DDR涓嶰CRAM remap鐩爣鍩哄湴鍧?瀵勫瓨鍣?

    {0x148, WO, 0x000000FD, 0xffff0000, 0xffff0000},                    //R_MMBUF_CTRL                      MMBUF CTRL閰嶇疆瀵勫瓨鍣?
    {0x148, WO, 0x000000FD, 0xffffffff, 0xffffffff},

    {0x14C, RW, 0x00000000, 0x000007FF, 0x000007FF},                    //R_HIFIDSP_CLK_DIV_AUTO                HIFIDSP鏃堕挓棰戠巼鑷姩璋冮閰嶇疆瀵勫瓨鍣?
    {0x150, RW, 0x00000110, 0x0000ffff, 0x0000ffff},                    //R_INTR_FREQ_OFFSET_CFG                棰戝亸璁板綍妯″潡閰嶇疆瀵勫瓨鍣?
    {0x160, RW, 0x00000000, 0x0000000f, 0x0000000f},                    //R_ACCESS_ADDR_MONITOR_EN_ADDR         鍐呭瓨鐩戞帶妯″潡浣胯兘閰嶇疆瀵勫瓨鍣?
    {0x164, WO, 0x00000000, 0X00000001, 0x00000000},                    //R_ACCESS_ADDR_MONITOR_INTR_CLR_ADDR   鍐呭瓨鐩戞帶妯″潡涓柇娓呴浂閰嶇疆瀵勫瓨鍣?
    {0x168, RW, 0x00000000, 0x0000000f, 0x0000000f},                    //R_ACCESS_ADDR_MONITOR_INTR_EN_ADDR    鍐呭瓨鐩戞帶妯″潡涓柇浣胯兘閰嶇疆瀵勫瓨鍣?
    {0x16C, RO, 0x00000000, 0x00000000, 0x00000000},                    //R_ACCESS_ADDR_MONITOR_INTR_STAT_ADDR  鍐呭瓨鐩戞帶妯″潡涓柇鐘舵?佸瘎瀛樺櫒
    {0x170, RW, 0x00000000, 0xFFFFF000, 0xFFFFF000},                    //R_ACCESS_ADDR_MONITOR0_BASE_ADDR      鍐呭瓨鐩戞帶妯″潡0鍩哄湴鍧?閰嶇疆瀵勫瓨鍣?
    {0x174, RW, 0x00000000, 0xFFFFF000, 0xFFFFF000},                    //R_ACCESS_ADDR_MONITOR0_LEN_ADDR       鍐呭瓨鐩戞帶妯″潡0鍦板潃闀垮害閰嶇疆瀵勫瓨鍣?
    {0x178, RO, 0x00000000, 0x00000000, 0x00000000},                    //R_ACCESS_ADDR_MONITOR0_HIT_AWADDR     鍐呭瓨鐩戞帶妯″潡0瓒婄晫鐨勫啓鍦板潃璁板綍瀵勫瓨鍣?
    {0x17C, RO, 0x00000000, 0x00000000, 0x00000000},                    //R_ACCESS_ADDR_MONITOR0_HIT_ARADDR     鍐呭瓨鐩戞帶妯″潡0瓒婄晫鐨勮鍦板潃璁板綍瀵勫瓨鍣?
    {0x180, RW, 0x00000000, 0xFFFFF000, 0xFFFFF000},                    //R_ACCESS_ADDR_MONITOR1_BASE_ADDR      鍐呭瓨鐩戞帶妯″潡1鍩哄湴鍧?閰嶇疆瀵勫瓨鍣?
    {0x184, RW, 0x00000000, 0xFFFFF000, 0xFFFFF000},                    //R_ACCESS_ADDR_MONITOR1_LEN_ADDR       鍐呭瓨鐩戞帶妯″潡1鍦板潃闀垮害閰嶇疆瀵勫瓨鍣?
    {0x188, RO, 0x00000000, 0xffffffff, 0x00000000},                    //R_ACCESS_ADDR_MONITOR1_HIT_AWADDR     鍐呭瓨鐩戞帶妯″潡1瓒婄晫鐨勫啓鍦板潃璁板綍瀵勫瓨鍣?
    {0x18C, RO, 0x00000000, 0xffffffff, 0x00000000},                    //R_ACCESS_ADDR_MONITOR1_HIT_ARADDR     鍐呭瓨鐩戞帶妯″潡1瓒婄晫鐨勮鍦板潃璁板綍瀵勫瓨鍣?
    {0x190, RW, 0x00000000, 0xFFFFF000, 0xFFFFF000},                    //R_ACCESS_ADDR_MONITOR2_BASE_ADDR      鍐呭瓨鐩戞帶妯″潡2鍩哄湴鍧?閰嶇疆瀵勫瓨鍣?
    {0x194, RW, 0x00000000, 0xFFFFF000, 0xFFFFF000},                    //R_ACCESS_ADDR_MONITOR2_LEN_ADDR       鍐呭瓨鐩戞帶妯″潡2鍦板潃闀垮害閰嶇疆瀵勫瓨鍣?
    {0x198, RO, 0x00000000, 0xffffffff, 0x00000000},                    //R_ACCESS_ADDR_MONITOR2_HIT_AWADDR     鍐呭瓨鐩戞帶妯″潡2瓒婄晫鐨勫啓鍦板潃璁板綍瀵勫瓨鍣?
    {0x19C, RO, 0x00000000, 0xffffffff, 0x00000000},                    //R_ACCESS_ADDR_MONITOR2_HIT_ARADDR     鍐呭瓨鐩戞帶妯″潡2瓒婄晫鐨勮鍦板潃璁板綍瀵勫瓨鍣?
    {0x1A0, RW, 0x00000000, 0xFFFFF000, 0xFFFFF000},                    //R_ACCESS_ADDR_MONITOR3_BASE_ADDR      鍐呭瓨鐩戞帶妯″潡3鍩哄湴鍧?閰嶇疆瀵勫瓨鍣?
    {0x1A4, RW, 0x00000000, 0xFFFFF000, 0xFFFFF000},                    //R_ACCESS_ADDR_MONITOR3_BASE_ADDR      鍐呭瓨鐩戞帶妯″潡3鍦板潃闀垮害閰嶇疆瀵勫瓨鍣?
    {0x1A8, RO, 0x00000000, 0xffffffff, 0x00000000},                    //R_ACCESS_ADDR_MONITOR3_HIT_AWADDR     鍐呭瓨鐩戞帶妯″潡3瓒婄晫鐨勫啓鍦板潃璁板綍瀵勫瓨鍣?
    {0x1AC, RO, 0x00000000, 0xffffffff, 0x00000000},                    //R_ACCESS_ADDR_MONITOR3_HIT_ARADDR     鍐呭瓨鐩戞帶妯″潡3瓒婄晫鐨勮鍦板潃璁板綍瀵勫瓨鍣?
    {0x1B0, RW, 0x01A80090, 0xffffffff, 0xffffffff},                    //R_MEMORY_CTRL                         memory鎺у埗閰嶇疆瀵勫瓨鍣?


    /*reserved
    {0x1B4, RW, 0x00000001, 0x00000001, 0x00000001},                    //R_SLIMBUS_PRIMARY_ADDR                slimbus primary閰嶇疆瀵勫瓨鍣?
    {0x1B8, RW, 0x00000000, 0x00000000, 0x00000001},                    //R_SLIMBUS_ID_ADDR                     slimbus id閰嶇疆瀵勫瓨鍣?
    {0x200, RW, 0x00000000, 0x00000000, 0x00000001},                    //R_SECURE_AUTHORITY_EN                 瀹夊叏璁块棶灞炴?ф帶鍒跺瘎瀛樺櫒
    */

};

/*****************************************************************************
  3 函数实现
*****************************************************************************/


/*****************************************************************************
 函 数 名  : tc_cfg_read_write_test
 功能描述  : 测试cfg中的寄存器
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年1月27日
    作    者   : w00262566
    修改内容   : 新生成函数

*****************************************************************************/
void tc_cfg_read_write_test(void)
{
    char* tc_cfg_read_write_test_success = "tc_cfg_read_write_test result success\n";
    char* tc_cfg_read_write_test_failed  = "tc_cfg_read_write_test result faied\n";
    pr_info("Audio: test reg beg\n");

    if(AT_SUCCESS == asp_cfg_reg_rw_test(asp_cfg_regs, sizeof(asp_cfg_regs) / sizeof(asp_cfg_regs[0]))){
        at_util_log(tc_cfg_read_write_test_success,strlen(tc_cfg_read_write_test_success));
    }else {
        at_util_log(tc_cfg_read_write_test_failed,strlen(tc_cfg_read_write_test_failed));
    }

    pr_info("Audio: test reg end\n");
}
EXPORT_SYMBOL(tc_cfg_read_write_test);


/*****************************************************************************
 函 数 名  : tc_cfg_reg_prepare
 功能描述  : 测试前处理
 输入参数  : int 测试参数
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年1月27日
    作    者   : w00262566
    修改内容   : 新生成函数

*****************************************************************************/
int tc_cfg_reg_prepare(int test_pram)
{
    pr_info("Audio: tc_cfg_reg_prepare \n");
    asp_cfg_powerup();
    return AT_SUCCESS;
}

/*****************************************************************************
 函 数 名  : tc_cfg_reg_beg
 功能描述  : 测试处理
 输入参数  : int 测试参数
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年1月27日
    作    者   : w00262566
    修改内容   : 新生成函数

*****************************************************************************/
int tc_cfg_reg_beg(int test_pram)
{
    pr_info("Audio: tc_cfg_reg_beg \n");
    tc_cfg_read_write_test();
    return AT_SUCCESS;
}

/*****************************************************************************
 函 数 名  : tc_cfg_reg_stop
 功能描述  : 测试后处理
 输入参数  : int 测试参数
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年1月27日
    作    者   : w00262566
    修改内容   : 新生成函数

*****************************************************************************/
int tc_cfg_reg_stop(int test_pram)
{
    pr_info("Audio: tc_cfg_reg_stop \n");
    return AT_SUCCESS;
}


/*****************************************************************************
 函 数 名  : tc_cfg_read_write_test_individual
 功能描述  : 测试cfg中的寄存器
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年1月27日
    作    者   : w00262566
    修改内容   : 新生成函数

*****************************************************************************/
void tc_cfg_read_write_test_individual(void)
{
    pr_info("Audio: tc_cfg_read_write_test_individual beg\n");
    char* tc_cfg_read_log_beg = "tc_cfg_read_write_test_individual beg\n";
    char* tc_cfg_read_log_end = "tc_cfg_read_write_test_individual end\n";
    at_util_log(tc_cfg_read_log_beg,strlen(tc_cfg_read_log_beg));
    tc_cfg_reg_prepare(0);
    tc_cfg_reg_beg(0);
    tc_cfg_reg_stop(0);
    at_util_log(tc_cfg_read_log_end,strlen(tc_cfg_read_log_end));
    pr_info("Audio: tc_cfg_read_write_test_individual end\n");
}
EXPORT_SYMBOL(tc_cfg_read_write_test_individual);

/*****************************************************************************
 函 数 名  : tc_cfg_probe
 功能描述  : tc cfg 驱动初始化
 输入参数  : struct platform_device* 设备指针
 输出参数  : 无
 返 回 值  : 初始化结果
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年1月27日
    作    者   : w00262566
    修改内容   : 新生成函数

*****************************************************************************/
static int tc_cfg_probe(struct platform_device *pdev)
{
    pr_info("%s.\n",__FUNCTION__);

    tc_cfg_reg = devm_kmalloc(&pdev->dev,sizeof(struct at_case_type), GFP_ATOMIC);
    if(NULL == tc_cfg_reg){
         return -ENOMEM;
    }

    tc_cfg_reg->test_name   = "tc_cfg_reg";
    tc_cfg_reg->test_params = 0;
    tc_cfg_reg->test_pri    = AT_CASE_PRI_HIG;
    tc_cfg_reg->prepare     = tc_cfg_reg_prepare;
    tc_cfg_reg->begin       = tc_cfg_reg_beg;
    tc_cfg_reg->stop        = tc_cfg_reg_stop;

    //at_core_register_test_case(tc_cfg_reg);

    return 0;
}

/*****************************************************************************
 函 数 名  : tc_cfg_remove
 功能描述  : tc cfg 驱动移除
 输入参数  : struct platform_device* 设备指针
 输出参数  : 无
 返 回 值  : 驱动移除结果
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年1月26日
    作    者   : w00262566
    修改内容   : 新生成函数

*****************************************************************************/
static int tc_cfg_remove(struct platform_device *pdev)
{
    pr_info("%s\n",__FUNCTION__);

    at_core_unregister_test_case(tc_cfg_reg);
    kfree(tc_cfg_reg);
    return 0;
}

/*****************************************************************************
 函 数 名  : tc_cfg_init
 功能描述  : tc cfg 模块初始化
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 模块初始化结果
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年1月26日
    作    者   : w00262566
    修改内容   : 新生成函数

*****************************************************************************/
static int __init tc_cfg_init(void)
{
    pr_info("%s \n",__FUNCTION__);

    platform_device_register_simple(DRV_NAME,0,NULL,0);

    return platform_driver_register(&tc_cfg_driver);
}
module_init(tc_cfg_init);


/*****************************************************************************
 函 数 名  : tc_cfg_exit
 功能描述  : tc cfg 模块退出
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 模块退出结果
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年1月26日
    作    者   : w00262566
    修改内容   : 新生成函数

*****************************************************************************/
static void __exit tc_cfg_exit(void)
{
    pr_info("%s\n",__FUNCTION__);
    platform_driver_unregister(&tc_cfg_driver);
}
module_exit(tc_cfg_exit);


MODULE_AUTHOR("wangbingda 00262566");
MODULE_DESCRIPTION("hisilicon driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:" DRV_NAME);
