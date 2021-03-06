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

#define FRAME_WIDTH                                       (720)
#define FRAME_HEIGHT                                      (1280)

#define REGFLAG_DELAY                                       0XFFE
#define REGFLAG_END_OF_TABLE                         0xFFF   // END OF REGISTERS MARKER

#define LCM_DSI_CMD_MODE                                    0
#define LCM_ID_NT35521 					(0x21)

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
	{0xF0,	5,	{0x55,0xAA,0x52,0x08,0x00}},
		{0xB1,	2,	{0x68,0x21}},
		{0xB5,	1,	{0xC8}},
		{0xB6,	1,	{0x0F}},
		{0xB8,	4,	{0x00,0x00,0x0A,0x00}},
		{0xB9,	1,	{0x00}},
		{0xBA,	1,	{0x02}},
		{0xBB,	2,	{0x63,0x63}},		 
		{0xBC,	2,	{0x02,0x02}},//00	 
		//	{0xBD,	5,	{0x02,0x7F,0x0D,0x0B,0x00}},
		{0xCC, 16,	{0x41,0x36,0x87,0x54,0x46,0x65,0x10,0x12,0x14,0x10,0x12,0x14,0x40,0x08,0x15,0x05}}, 	   
		{0xD0,	1,	{0x00}},
		{0xD1, 16,	{0x00,0x04,0x08,0x0C,0x10,0x14,0x18,0x1C,0x20,0x24,0x28,0x2C,0x30,0x34,0x38,0x3C}},  
		{0xD3,	1,	{0x00}},
		{0xD6,	2,	{0x44,0x44}},
		{0xD7, 12,	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}, 	   
		{0xD8, 13,	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},		 
		{0xD9,	2,	{0x00,0x28}},
		{0xE5,	2,	{0x00,0xFF}},
		{0xE6,	4,	{0xF3,0xEC,0xE7,0xDF}},
		{0xE7, 10,	{0xF3,0xD9,0xCC,0xCD,0xB3,0xA6,0x99,0x99,0x99,0x95}},		  
		{0xE8, 10,	{0xF3,0xD9,0xCC,0xCD,0xB3,0xA6,0x99,0x99,0x99,0x95}},		  
		{0xE9,	2,	{0x00,0x04}},
		{0xEA,	1,	{0x00}},
		{0xEE,	4,	{0x87,0x78,0x00,0x00}},
		{0xEF,	2,	{0x07,0xFF}},		  
		{0xF0,	5,	{0x55,0xAA,0x52,0x08,0x01}},
		// {0xB0,  2,  {0x0D,0x0D}},
		// {0xB1,  2,  {0x0D,0x0D}},
		{0xB3,	2,	{0x28,0x28}},
		{0xB4,	2,	{0x0F,0x0F}},
		{0xB5,	2,	{0x06,0x06}},
		{0xB6,	2,	{0x05,0x05}},
		{0xB7,	2,	{0x05,0x05}},
		{0xB8,	2,	{0x05,0x05}},
		{0xB9,	2,	{0x44,0x44}},
		{0xBA,	2,	{0x36,0x36}},
		{0xBC,	2,	{0x50,0x00}},
		{0xBD,	2,	{0x50,0x00}},
		{0xBE,	1,	{0x2A}},//39 //25  22
		{0xBF,	1,	{0x3C}},//39
		{0xC0,	1,	{0x0C}},
		{0xC1,	1,	{0x00}},
		{0xC2,	2,	{0x19,0x19}},
		{0xC3,	2,	{0x0A,0x0A}},
		{0xC4,	2,	{0x23,0x23}},
		{0xC7,	3,	{0x00,0x80,0x00}},
		{0xC9,	6,	{0x00,0x00,0x00,0x00,0x00,0x00}},		 
		{0xCA,	1,	{0x01}},		 
		{0xCB,	2,	{0x0B,0x53}},
		{0xCC,	1,	{0x00}},
		{0xCD,	3,	{0x0B,0x52,0x53}},
		{0xCE,	1,	{0x44}},
		{0xCF,	3,	{0x00,0x50,0x50}},
		{0xD0,	2,	{0x50,0x50}},
		{0xD1,	2,	{0x50,0x50}},
		{0xD2,	1,	{0x39}},
		{0xD3,	1,	{0x39}},		 
		{0xF0,	5,	{0x55,0xAA,0x52,0x08,0x02}},
		{0xB0, 16,	{0x00,0xAC,0x00,0xBA,0x00,0xD9,0x00,0xED,0x01,0x01,0x01,0x1E,0x01,0x3A,0x01,0x62}}, 		
		{0xB1, 16,	{0x01,0x85,0x01,0xB8,0x01,0xE4,0x02,0x27,0x02,0x5B,0x02,0x5D,0x02,0x8C,0x02,0xBE}}, 		
		{0xB2, 16,	{0x02,0xDF,0x03,0x0C,0x03,0x2A,0x03,0x51,0x03,0x6D,0x03,0x8D,0x03,0xA4,0x03,0xBE}}, 		
		{0xB3,	4,	{0x03,0xCC,0x03,0xCC}}, 		
		{0xB4, 16,	{0x00,0xAC,0x00,0xBA,0x00,0xD9,0x00,0xED,0x01,0x01,0x01,0x1E,0x01,0x3A,0x01,0x62}}, 		
		{0xB5, 16,	{0x01,0x85,0x01,0xB8,0x01,0xE4,0x02,0x27,0x02,0x5B,0x02,0x5D,0x02,0x8C,0x02,0xBE}}, 		 
		{0xB6, 16,	{0x02,0xDF,0x03,0x0C,0x03,0x2A,0x03,0x51,0x03,0x6D,0x03,0x8D,0x03,0xA4,0x03,0xBE}}, 		 
		{0xB7,	4,	{0x03,0xCC,0x03,0xCC}}, 		 
		{0xB8, 16,	{0x00,0xAC,0x00,0xBA,0x00,0xD9,0x00,0xED,0x01,0x01,0x01,0x1E,0x01,0x3A,0x01,0x62}}, 		 
		{0xB9, 16,	{0x01,0x85,0x01,0xB8,0x01,0xE4,0x02,0x27,0x02,0x5B,0x02,0x5D,0x02,0x8C,0x02,0xBE}}, 		
		{0xBA, 16,	{0x02,0xDF,0x03,0x0C,0x03,0x2A,0x03,0x51,0x03,0x6D,0x03,0x8D,0x03,0xA4,0x03,0xBE}}, 		 
		{0xBB,	4,	{0x03,0xCC,0x03,0xCC}}, 		 
		{0xBC, 16,	{0x00,0xAC,0x00,0xBA,0x00,0xD9,0x00,0xED,0x01,0x01,0x01,0x1E,0x01,0x3A,0x01,0x62}}, 		
		{0xBD, 16,	{0x01,0x85,0x01,0xB8,0x01,0xE4,0x02,0x27,0x02,0x5B,0x02,0x5D,0x02,0x8C,0x02,0xBE}}, 		
		{0xBE, 16,	{0x02,0xDF,0x03,0x0C,0x03,0x2A,0x03,0x51,0x03,0x6D,0x03,0x8D,0x03,0xA4,0x03,0xBE}}, 		
		{0xBF,	4,	{0x03,0xCC,0x03,0xCC}}, 		
		{0xC0, 16,	{0x00,0xAC,0x00,0xBA,0x00,0xD9,0x00,0xED,0x01,0x01,0x01,0x1E,0x01,0x3A,0x01,0x62}}, 		
		{0xC1, 16,	{0x01,0x85,0x01,0xB8,0x01,0xE4,0x02,0x27,0x02,0x5B,0x02,0x5D,0x02,0x8C,0x02,0xBE}}, 		 
		{0xC2, 16,	{0x02,0xDF,0x03,0x0C,0x03,0x2A,0x03,0x51,0x03,0x6D,0x03,0x8D,0x03,0xA4,0x03,0xBE}}, 		 
		{0xC3,	4,	{0x03,0xCC,0x03,0xCC}}, 		 
		{0xC4, 16,	{0x00,0xAC,0x00,0xBA,0x00,0xD9,0x00,0xED,0x01,0x01,0x01,0x1E,0x01,0x3A,0x01,0x62}}, 		 
		{0xC5, 16,	{0x01,0x85,0x01,0xB8,0x01,0xE4,0x02,0x27,0x02,0x5B,0x02,0x5D,0x02,0x8C,0x02,0xBE}}, 		
		{0xC6, 16,	{0x02,0xDF,0x03,0x0C,0x03,0x2A,0x03,0x51,0x03,0x6D,0x03,0x8D,0x03,0xA4,0x03,0xBE}}, 		 
		{0xC7,	4,	{0x03,0xCC,0x03,0xCC}}, 
		{0xEE,	1,	{0x00}},		 
		{0xF0,	5,	{0x55,0xAA,0x52,0x08,0x03}},			
		{0xB0,	2,	{0x00,0x00}},			
		{0xB1,	2,	{0x00,0x00}},	
		{0xB2,	5,	{0x03,0x00,0x00,0x00,0x00}},
		{0xB3,	5,	{0x03,0x00,0x00,0x00,0x00}},	
		{0xB4,	5,	{0x03,0x00,0x00,0x00,0x00}},					
		{0xB5,	5,	{0x03,0x00,0x00,0x00,0x00}},			
		{0xB6,	5,	{0x03,0x00,0x00,0x00,0x00}},						
		{0xB7,	5,	{0x03,0x00,0x00,0x00,0x00}},		
		{0xB8,	5,	{0x03,0x00,0x00,0x00,0x00}},	
		{0xB9,	5,	{0x03,0x00,0x00,0x00,0x00}},				
		{0xBA,	5,	{0x35,0x10,0x00,0x00,0x00}},				
		{0xBB,	5,	{0x35,0x10,0x00,0x00,0x00}},				
		{0xBC,	5,	{0x35,0x10,0x00,0x00,0x00}},	
		{0xBD,	5,	{0x35,0x10,0x00,0x00,0x00}},		
		{0xC0,	4,	{0x00,0x34,0x00,0x00}}, 					
		{0xC1,	4,	{0x00,0x34,0x00,0x00}},
		{0xC2,	4,	{0x00,0x34,0x00,0x00}}, 			
		{0xC3,	4,	{0x00,0x34,0x00,0x00}}, 			
		{0xC4,	1,	{0x40}},			
		{0xC5,	1,	{0x40}},			
		{0xC6,	1,	{0x40}},			
		{0xC7,	1,	{0x40}},			
		{0xEF,	1,	{0x00}},		 
		{0xF0,	5,	{0x55,0xAA,0x52,0x08,0x05}},						
		{0xB0,	2,	{0x1B,0x10}},						
		{0xB1,	2,	{0x1B,0x10}},						
		{0xB2,	2,	{0x1B,0x10}},							
		{0xB3,	2,	{0x1B,0x10}},							
		{0xB4,	2,	{0x1B,0x10}},							
		{0xB5,	2,	{0x1B,0x10}},							
		{0xB6,	2,	{0x1B,0x10}},							
		{0xB7,	2,	{0x1B,0x10}},							
		{0xB8,	1,	{0x00}},								
		{0xB9,	1,	{0x00}},								
		{0xBA,	1,	{0x00}},								
		{0xBB,	1,	{0x00}},
		{0xBC,	1,	{0x00}},								
		{0xBD,	5,	{0x03,0x03,0x03,0x00,0x01}},				
		{0xC0,	1,	{0x03}},							
		{0xC1,	1,	{0x05}},							
		{0xC2,	1,	{0x03}},							
		{0xC3,	1,	{0x05}},								
		{0xC4,	1,	{0x80}},								
		{0xC5,	1,	{0xA2}},							
		{0xC6,	1,	{0x80}},							
		{0xC7,	1,	{0xA2}},							
		{0xC8,	2,	{0x01,0x20}},							
		{0xC9,	2,	{0x00,0x20}},						
		{0xCA,	2,	{0x01,0x00}},						
		{0xCB,	2,	{0x00,0x00}},						
		{0xCC,	3,	{0x00,0x00,0x01}},				
		{0xCD,	3,	{0x00,0x00,0x01}},				
		{0xCE,	3,	{0x00,0x00,0x01}},				
		{0xCF,	3,	{0x00,0x00,0x01}},				
		{0xD0,	1,	{0x00}},						
		{0xD1,	5,	{0x03,0x00,0x00,0x07,0x10}},	
		{0xD2,	5,	{0x13,0x00,0x00,0x07,0x11}},	
		{0xD3,	5,	{0x23,0x00,0x00,0x07,0x10}},		
		{0xD4,	5,	{0x33,0x00,0x00,0x07,0x11}},		
		{0xE5,	1,	{0x06}},						
		{0xE6,	1,	{0x06}},						
		{0xE7,	1,	{0x06}},					
		{0xE8,	1,	{0x06}},						
		{0xE9,	1,	{0x06}},					
		{0xEA,	1,	{0x06}},					
		{0xEB,	1,	{0x06}},							
		{0xEC,	1,	{0x06}},							
		{0xED,	1,	{0x31}},		
		{0xF0,	5,	{0x55,0xAA,0x52,0x08,0x06}},			
		{0xB0,	2,	{0x10,0x11}},						
		{0xB1,	2,	{0x12,0x13}},						
		{0xB2,	2,	{0x08,0x00}},						
		{0xB3,	2,	{0x2D,0x2D}},						
		{0xB4,	2,	{0x2D,0x34}},						
		{0xB5,	2,	{0x34,0x2D}},						
		{0xB6,	2,	{0x2D,0x34}},						
		{0xB7,	2,	{0x34,0x34}},					
		{0xB8,	2,	{0x02,0x0A}},					
		{0xB9,	2,	{0x00,0x08}},					
		{0xBA,	2,	{0x09,0x01}},					
		{0xBB,	2,	{0x0B,0x03}},					
		{0xBC,	2,	{0x34,0x34}},						
		{0xBD,	2,	{0x34,0x2D}},						
		{0xBE,	2,	{0x2D,0x34}},					
		{0xBF,	2,	{0x34,0x2D}},					
		{0xC0,	2,	{0x2D,0x2D}},					
		{0xC1,	2,	{0x01,0x09}},					
		{0xC2,	2,	{0x19,0x18}},
		{0xC3,	2,	{0x17,0x16}},
		{0xC4,	2,	{0x19,0x18}},
		{0xC5,	2,	{0x17,0x16}},
		{0xC6,	2,	{0x01,0x09}},
		{0xC7,	2,	{0x2D,0x2D}},
		{0xC8,	2,	{0x2D,0x34}},
		{0xC9,	2,	{0x34,0x2D}},
		{0xCA,	2,	{0x2D,0x34}},
		{0xCB,	2,	{0x34,0x34}},
		{0xCC,	2,	{0x0B,0x03}},
		{0xCD,	2,	{0x09,0x01}},
		{0xCE,	2,	{0x00,0x08}},
		{0xCF,	2,	{0x02,0x0A}},
		{0xD0,	2,	{0x34,0x34}},
		{0xD1,	2,	{0x34,0x2D}},
		{0xD2,	2,	{0x2D,0x34}},
		{0xD3,	2,	{0x34,0x2D}},
		{0xD4,	2,	{0x2D,0x2D}},
		{0xD5,	2,	{0x08,0x00}},
		{0xD6,	2,	{0x10,0x11}},
		{0xD7,	2,	{0x12,0x13}},
		{0xD8,	5,	{0x55,0x55,0x55,0x55,0x55}},
		{0xD9,	5,	{0x55,0x55,0x55,0x55,0x55}},
		{0xE5,	2,	{0x34,0x34}},
		{0xE6,	2,	{0x34,0x34}},
		{0xE7,	1,	{0x05}},		
		{0xF0,	5,	{0x55,0xAA,0x52,0x00,0x00}},
	
    {REGFLAG_ESCAPE_ID,REGFLAG_DELAY_MS_V3, 120, {}},
    {0x11,0x29,1,{0x00}},
    {REGFLAG_ESCAPE_ID,REGFLAG_DELAY_MS_V3, 20, {}},
};

static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
    {0x11, 0, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
    {0x29, 0, {0x00}},
    {REGFLAG_DELAY, 40, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
    // Display off sequence
    {0x28, 0, {0x00}},

    // Sleep Mode On
    {0x10, 0, {0x00}},

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
        params->dsi.mode   = SYNC_PULSE_VDO_MODE;
        // DSI
        /* Command mode setting */
        params->dsi.LANE_NUM                = LCM_FOUR_LANE;
        //The following defined the fomat for data coming from LCD engine.
        params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
        params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
        params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
        params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

        // Highly depends on LCD driver capability.
        // Not support in MT6573
        params->dsi.packet_size=128;
        // Video mode setting
        params->dsi.intermediat_buffer_num = 0;
        params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
        params->dsi.vertical_sync_active				= 5;// 3    2
        params->dsi.vertical_backporch					= 30;// 20   1
        params->dsi.vertical_frontporch					= 30; // 1  12
        params->dsi.vertical_active_line				= FRAME_HEIGHT;
        params->dsi.horizontal_sync_active				= 10;// 50  2
        params->dsi.horizontal_backporch				= 178;
        params->dsi.horizontal_frontporch				= 178;
        params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
        // Bit rate calculation
        params->dsi.PLL_CLOCK=208;
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
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(10);//Must > 10ms
    SET_RESET_PIN(1);
    MDELAY(120);//Must > 120ms

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
    SET_RESET_PIN(1);
    MDELAY(20);
    SET_RESET_PIN(0);
    MDELAY(120);//Must > 10ms
    SET_RESET_PIN(1);
    MDELAY(20);//Must > 120ms

    push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_resume(void)
{
    //lcm_compare_id();

    lcm_init();

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
    MDELAY(20);
    SET_RESET_PIN(1);
    MDELAY(80);
    array[0] = 0x00033700;// read id return two byte,version and id
    dsi_set_cmdq(array, 1, 1);
    read_reg_v2(0xc5, buffer, 3);
    id = buffer[1]; //we only need ID
    lcm_driver_id = id;
    lcm_module_id = 0x0;
#ifdef BUILD_LK
	printf("leanda lcm_compare_id nt35521 buffer[0] = 0x%x,buffer[1] = 0x%x,buffer[2] = 0x%x\n",buffer[0],buffer[1],buffer[2]);
#else
	printk("leanda2 lcm_compare_id nt35521 buffer[0] = 0x%x,buffer[1] = 0x%x,buffer[2] = 0x%x\n",buffer[0],buffer[1],buffer[2]);

#endif


	if((buffer[0]==0x55)&&(buffer[1]==0x21))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

static unsigned int lcm_esd_check(void)
{
#ifndef BUILD_LK
    char  buffer[3];
    int   array[4];
    array[0] = 0x00013700;
    dsi_set_cmdq(array, 1, 1);
    read_reg_v2(0x0a, buffer, 1);
	if(buffer[0]==0x9c)
	{
		return FALSE;
	}
	else
	{	
		return TRUE;
	}
#else
	return FALSE;
#endif
}

static unsigned int lcm_esd_recover(void)
{
#ifndef BUILD_LK	
	lcm_init();
//	MDELAY(100);
//	lcm_init();
	printk("leanda test lcm_esd_recover\n");
#endif
	return TRUE;
}

LCM_DRIVER nt35521_hd720_dsi_vdo_lcm_drv =
{
    .name           = "nt35521_hd720_dsi_vdo",
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

