/*
* Copyright (C) 2011-2014 MediaTek Inc.
*
* This program is free software: you can redistribute it and/or modify it under the terms of the
* GNU General Public License version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with this program.
* If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef BUILD_LK
#else
#include <linux/string.h>
#endif

#ifdef BUILD_LK
    #include <platform/mt_gpio.h>
    #include <string.h>
#elif defined(BUILD_UBOOT)
    #include <asm/arch/mt_gpio.h>
#else
    #include <mach/mt_gpio.h>
#endif

#include "lcm_drv.h"

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1280)

#define REGFLAG_DELAY             							0XFE
#define REGFLAG_END_OF_TABLE      							0xFF   // END OF REGISTERS MARKER

#define LCM_DSI_CMD_MODE                                    0
#define LCM_ID                      (0x1308)

static unsigned int lcm_driver_id;
static unsigned int lcm_module_id;

static struct LCM_setting_table *para_init_table;
static unsigned int para_init_size;
static LCM_PARAMS *para_params ;

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util;

#define SET_RESET_PIN(v)                                    (lcm_util.set_reset_pin((v)))

#define UDELAY(n)                                             (lcm_util.udelay(n))
#define MDELAY(n)                                             (lcm_util.mdelay(n))

//static kal_bool IsFirstBoot = KAL_TRUE;

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)    lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)        lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)                                        lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)                    lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg                                            lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)                   lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size, 1)
#define dsi_set_cmdq_V3(para_tbl,size,force_update)        	lcm_util.dsi_set_cmdq_V3(para_tbl,size,force_update)

static  LCM_setting_table_V3 lcm_initialization_setting[] = {
	/*
	Note :
	Data ID will depends on the following rule.
	
		count of parameters > 1	=> Data ID = 0x39
		count of parameters = 1	=> Data ID = 0x15
		count of parameters = 0	=> Data ID = 0x05
	Structure Format :
	{DCS command, count of parameters, {parameter list}}
	{REGFLAG_DELAY, milliseconds of time, {}},
	...
	Setting ending by predefined flag
	
	{REGFLAG_END_OF_TABLE, 0x00, {}}
	*/


	//must use 0x39 for init setting for all register.

/*	{0XB0, 1, {0X04}},
	{0XB3, 1, {0X02}},
	{0XB6, 4, {0X52,0X63,0X85,0X00}},
	{0XB7, 3, {0X00,0X00,0X01}},
	
	{0xC0, 7, {0x43,0xFF,0x04,0x0B,0x02,0x07,0x07}},
	{0xC1, 7, {0x50,0x02,0x22,0x00,0x00,0x61,0x11}},
	{0xC3, 16, {0x04,0x00,0x05,0x14,0x80,0x00,0x00,
						0x00,0x00,0x90,0x00,0x00,0x0C,0x00,0x00,0x5C}},
	{0xC8, 24, {0x1C,0x23,0x27,
						0x2B,0x30,0x34,0x16,
						0x11,0x0F,0x0D,0x0A,
						0x00,0x1C,0x22,0x28,
						0x2E,0x33,0x38,0x13,
						0x11,0x0F,0x0D,0x0A,0x00}}, 
	{0xC9, 11, {0x00,0x80,0x80,
						0x80,0x80,0x80,0x80,
						0x80,0x08,0x20,0x80}},
	{0xD0, 11, {0x74,0x29,0xDD,0x15,
						0x09,0x2B,0x00,0xC0,0xCC}},
	{0xD1, 13, {0x4D,0x24,0x34,0x55,
						0x55,0x77,0x77,0x66,
						0x31,0x75,0x42,0x86,0x06}},
	{0xD5, 2, {0x18, 0x18}},
	{0xD6, 1, {0xA8}},
	{0xDE, 3, {0X03, 0X2D, 0X2D}},*/

	{0x11,0,{0x00}},
    {REGFLAG_ESCAPE_ID,REGFLAG_DELAY_MS_V3, 150, {}},

	{0x36, 1, {0x00}},
	{0x3A, 1, {0x07}},

	{0x29, 0, {0x00}},
    {REGFLAG_ESCAPE_ID,REGFLAG_DELAY_MS_V3, 100, {}},
};

static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
    {0x11, 0, {0x00}},
    {REGFLAG_DELAY, 100, {}},

    // Display ON
    {0x29, 0, {0x00}},
    {REGFLAG_DELAY, 40, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
    // Display off sequence
	{0x28, 0, {0x00}},
	{REGFLAG_DELAY, 50, {}},
    // Sleep Mode On
	{0x10, 0, {0x00}},
	{REGFLAG_DELAY, 100, {}},
		{0XB0, 1, {0X04}},
	{REGFLAG_DELAY, 20, {}},
	{0XB1, 1, {0X01}},
	{REGFLAG_DELAY, 50, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
    unsigned int i;

    for(i = 0; i < count; i++) {

        unsigned cmd;
        cmd = table[i].cmd;

        switch (cmd) {

            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;

            case REGFLAG_END_OF_TABLE :
                break;

            default:
                dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
                //MDELAY(10);//soso add or it will fail to send register
           }
    }

}


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}

static void lcm_set_params(struct LCM_setting_table *init_table, unsigned int init_size, LCM_PARAMS *params)
{
    para_init_table = init_table;
    para_init_size = init_size;
    para_params = params;
}

static void lcm_get_params(LCM_PARAMS *params)
{
    memset(params, 0, sizeof(LCM_PARAMS));
    if (para_params != NULL)
    {
        memcpy(params, para_params, sizeof(LCM_PARAMS));
    }
    else
    {
        params->type   = LCM_TYPE_DSI;
        params->width  = FRAME_WIDTH;
        params->height = FRAME_HEIGHT;
#if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
#else
		params->dsi.mode   = BURST_VDO_MODE;
#endif
        // DSI
        /* Command mode setting */
        //params->dsi.LANE_NUM                = LCM_TWO_LANE;
        //The following defined the fomat for data coming from LCD engine.
        params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
        params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
        params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
        params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

        // Highly depends on LCD driver capability.
        // Not support in MT6573
        params->dsi.packet_size=256;
        // Video mode setting
        params->dsi.intermediat_buffer_num = 0;
        params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
        params->dsi.vertical_sync_active				= 1;// 3    2
        params->dsi.vertical_backporch					= 11;// 20   1
        params->dsi.vertical_frontporch					= 4; // 1  12
        params->dsi.vertical_active_line				= FRAME_HEIGHT;
        params->dsi.horizontal_sync_active				= 2;// 50  2
        params->dsi.horizontal_backporch				= 50;
        params->dsi.horizontal_frontporch				= 148;
        params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
        // Bit rate calculation
        //params->dsi.PLL_CLOCK=208;
        params->dsi.pll_div1=0;//32        // fref=26MHz, fvco=fref*(div1+1)    (div1=0~63, fvco=500MHZ~1GHz)
        params->dsi.pll_div2=1;         // div2=0~15: fout=fvo/(2*div2)
#if (LCM_DSI_CMD_MODE)
        params->dsi.fbk_div =7;
#else
        params->dsi.fbk_div =19;    // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)	
#endif  // div2=0~15: fout=fvo/(2*div2)
    }
}

static void lcm_get_id(unsigned int* driver_id, unsigned int* module_id)
{
    *driver_id = lcm_driver_id;
    *module_id = lcm_module_id;
}
static void lcm_init(void)
{

    SET_RESET_PIN(1);
    MDELAY(20);
    SET_RESET_PIN(0);
    MDELAY(20);//Must > 10ms
    SET_RESET_PIN(1);
    MDELAY(50);//Must > 120ms

    if (para_init_table != NULL)
    {
        push_table(para_init_table, para_init_size, 1);
    }
    else
    {
        dsi_set_cmdq_V3(lcm_initialization_setting,ARRAY_SIZE(lcm_initialization_setting),1);
    }
    //push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_suspend(void)
{

    SET_RESET_PIN(0);
    MDELAY(20);//Must > 10ms
    SET_RESET_PIN(1);
    MDELAY(50);//Must > 120ms

    push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_resume(void)
{
    //lcm_compare_id();

    lcm_init();

	SET_RESET_PIN(1);
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(20);//50
	SET_RESET_PIN(1);
	MDELAY(30);//100

    push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
}
static unsigned int lcm_compare_id(void)
{
    unsigned int id = 0;
    unsigned char buffer[5];
    unsigned int array[16];

    SET_RESET_PIN(1);  //NOTE:should reset LCM firstly
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(20);

    array[0] = 0x00053700;// read id return 5 byte
    dsi_set_cmdq(array, 1, 1);

    array[0] = 0x04B02300;// unlock for reading ID
    dsi_set_cmdq(array, 1, 1);
    MDELAY(50);

    read_reg_v2(0xBF, buffer, 5);
    id = (buffer[2] << 8) | buffer[3]; //we only need ID
    lcm_driver_id = id;
    lcm_module_id = 0x0;
#if defined(BUILD_LK)
        printf("%s, id1 = 0x%x\n", __func__, id);
#endif
        return (LCM_ID == id)?1:0;
}

static unsigned int lcm_esd_check(void)
{
#ifndef BUILD_LK
    char  buffer[3];
    int   array[4];
    array[0] = 0x00013700;
    dsi_set_cmdq(array, 1, 1);
    read_reg_v2(0x0a, buffer, 1);
	printk("R61308 lcm_esd_check 0x0A=%x\n",buffer[0]);
	if(buffer[0]==0x1c)
	{
		return 0;
	}
	else
	{			 
		return 1;
	}
#endif
}

static unsigned int lcm_esd_recover(void)
{
	lcm_init();
	return 1;
}

LCM_DRIVER r61308_dsi_vdo_lcm_drv =
{
    .name           = "r61308_dsi_vdo",
    .set_util_funcs = lcm_set_util_funcs,
    .set_params     = lcm_set_params,
    .get_id         = lcm_get_id,
    .get_params     = lcm_get_params,
    .init           = lcm_init,
    .suspend        = lcm_suspend,
    .resume         = lcm_resume,
    .compare_id     = lcm_compare_id,
    .esd_check      = lcm_esd_check,
    .esd_recover    = lcm_esd_recover,
};

