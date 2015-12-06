#ifndef __CUST_LTR553_H__
#define __CUST_LTR553_H__

/* For ps calibration */
#define CUST_LTR553_PS_THRES_CLOSE		50	 // Close 
#define CUST_LTR553_PS_THRES_FAR		20	 // Far away 

/*LTR553 ps cust setting*/
#define CUST_LTR553_PS_CONTR_ENABLE		0x03 // PS operation mode control			//PS Gain X16
#define CUST_LTR553_PS_LED				0x6B // PS LED setting					//50mA, 50%, 60kHz 
#define CUST_LTR553_PS_N_PULSES			0x0A // PS number of pulses				//Number of pulses = 10 
#define CUST_LTR553_PS_MEAS_RATE		0x00 // PS measurement rate in active mode	 //50 ms	   
#define CUST_LTR553_INTERRUPT_PERSIST	0x00 // ALS / PS Interrupt persist setting	 //every 	


/*******************************************/

/*LTR553 als cust setting*/
#define CUST_LTR553_ALS_CONTR_ENABLE	0x0D // ALS operation mode control SW reset	 //Gain 8X -> 0.125lux to 8k lux
#define CUST_LTR553_ALS_MEAS_RATE		0x03 // ALS measurement rate in active mode  //rate=500ms, time=100ms 

#endif
