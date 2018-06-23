/*
 * BMS_ASIC.h
 *
 *  Created on: 20-Jan-2017
 *      Author: NIKHIL
 */

#ifndef BMS_ASIC_H_
#define BMS_ASIC_H_

#include <I2C_API.h>
#include <AP_Communication.h>

#define BMS_ADDRESS									0x50	/* ISL94203 slave address */
#define BMS_I2C										I2C_1	/* I2C bus of MCU to be used for ISL */
#define I2C_OWN_ADDRESS								0x0F	/* This is the MCU's own address */
#define EEPROM_PAGE_SIZE							4		/* Max size of EEPROM page in ISL */
#define EEPROM_WRITE_DELAY							30		/* Time gap between two successiveEEPROM writes */
#define READ_WRITE_DELAY							5

#define SLOPE_5X									0.07713f	/* Equation(slope M)coefficients derived from log analysis */
#define SLOPE_50X									0.0831f
#define SLOPE_500X									0.0774f

#define CONSTANT_5X									54.9544f	/* Equation (constant C) derived from log analysis */
#define CONSTANT_50X								127.6836f
#define CONSTANT_500X								96.6649f

/*
 * Coefficients required for calculating used mAh from the no-load battery voltage
 * y = 0.051156*z^5 -0.69723*z^4 + 1.9663*z^3 -3.0335^z2 + 14.781*z +79.795
 * where z = (x - 3.94) / 0.13453
 * where x is battery voltage;
 * y is estimated battery capacity remaining (in mAh)
 */
#define BATT_EST_COEFF_0 							79.795f		/* z^0 */
#define BATT_EST_COEFF_1 							14.781f		/* z^1 */
#define BATT_EST_COEFF_2 							-3.0335f	/* z^2 */
#define BATT_EST_COEFF_3 							1.9663f		/* z^3 */
#define BATT_EST_COEFF_4 							-0.69723f	/* z^4 */
#define BATT_EST_COEFF_5 							0.051156f	/* z^5 */
#define BATT_EST_Mu									3.94f		/* constant used 3.94 (mean value)in above equation */
#define BATT_EST_Sigma 								0.13453f	/* constant used 0.13453 (standard deviation) */

//#define TATTU_BATTERY_9000
//#define TATTU_BATTERY_10000
#define MAX_AMP_11000

#ifdef TATTU_BATTERY_9000									/* TATTU Battery has capacity of 9000mAH */
	#define BATTERY_CAPACITY							9000		/* Battery capacity in mAH */
#elif defined TATTU_BATTERY_10000
	#define BATTERY_CAPACITY							10000		/* Battery capacity in mAH */
#elif defined MAX_AMP_11000
	#define BATTERY_CAPACITY							11000.0f	/* Battery capacity in mAH */
#endif

/* BMS ISL94203 status flags Internal register's addresses */
#define OV_THRESHOLD_ADDR							0x00
#define OV_RECOVERY_ADDR							0x02
#define UV_THROSHOLD_ADDR							0x04
#define UV_RECOVERY_ADDR							0x06
#define OV_LOCKOUT_THRESHOLD_ADDR					0x08
#define UV_LOCKOUT_THRESHOLD_ADDR					0x0A
#define EOC_THRESHOLD_ADDR							0x0C
#define OV_DELAY_TIMEOUT_ADDR						0x10
#define UV_DELAY_TIMEOUT_ADDR						0x12
#define OPEN_WIRING_TIMEOUT_ADDR					0x14

#define INTERNAL_OT_THRESHOLD_ADDR					0x40
#define INTERNAL_OT_RECOVERY_ADDR					0x42
#define NUMBER_OF_CELLS_ADDR						0x48

#define DISABLE_CELL_BALANCE_ADDR					0x87

#define USER_EEPROM_START_ADDR						0x50	/* ISL's user EEPROM start address */
#define RAM_STATUS_REG_ADDR							0x80	/* ISL's RAM status flags address */
#define PACK_CURRENT_ADDR							0x8E	/* ISL's pack current register address */
#define CELL_VOLTAGE_ADDR							0x90	/* ISL's cell voltages register address */
#define PACK_TEMPERATURE_ADDR						0xA0	/* ISL's internal temperature register address */
#define PACK_VOLTAGE_ADDR							0xA6	/* ISL's pack voltage register address */
#define CURRENT_GAIN_SETTING_ADDR					0x85	/* 4th and 5th bit in this register defined the current gain */

/* Here all the flags are defined which are required for Asteria BMS */
/* RAM location 0x80 status flags */
#define IS_ANY_CELL_V_OVER_THRESHOLD				(1 << 0)
#define IS_ANY_CELL_V_UNDER_THRESHOLD				(1 << 2)
#define IS_DISCHARGE_OVER_TEMP						(1 << 4)
#define IS_DISCHARGE_UNDER_TEMP						(1 << 5)
#define IS_CHARGE_OVER_TEMP							(1 << 6)
#define IS_CHARGE_UNDER_TEMP						(1 << 7)

/* RAM location 0x81 status flags */
#define IS_INTERNAL_OVER_TEMP						(1 << 8)
#define IS_CHARGE_OVER_CURRENT						(1 << 9)
#define IS_DISCHARGE_OVER_CURRENT					(1 << 10)
#define IS_DISHARGE_SHORT_CIRCUIT					(1 << 11)
#define IS_CELL_FAIL								(1 << 12)
#define IS_OPEN_WIRE								(1 << 13)
#define IS_END_OF_CHARGE							(1 << 15)

/* RAM location 0x82 status flags */
#define IS_PACK_CHARGING							(1 << 18)
#define IS_PACK_DISCHARGING							(1 << 19)
#define IS_INTERNAL_SCAN							(1 << 22)

/* RAM location 0x83 status flags */
#define IS_CELL_BALANCE_OVER_T						(1 << 24)
#define IS_CELL_BALANCE_UNDER_T						(1 << 25)
#define IS_CELL_BALANCE_OVER_V						(1 << 26)
#define IS_CELL_BALANCE_UNDER_V						(1 << 27)
#define IS_ISL_IN_IDLE								(1 << 28)
#define IS_ISL_IN_DOZE								(1 << 29)
#define IS_ISL_IN_SLEEP								(1 << 30)

#define NUMBER_OF_CELLS								8		/* The number of cells the battery has */
#define CELL_VOLTAGES_DATA_SIZE						(2*NUMBER_OF_CELLS)
#define SENSE_RESISTOR_VALUE						1e-3	/* Current sense resistor value used in hardware */
#define CURRENT_GAIN								5		/* Set the current gain as per sense resistor value */
#define MINIMUM_CURRENT_CONSUMPTION					200.0f		/* If current consumption is less than 50mA
 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	  for specific time then put the BMS to sleep */
#define MAXIMUM_PACK_VOLTAGE						25.0f		/* Maximum value of pack voltage */
#define CHARGE_CURRENT_CONSUMPTION					1000.0f
#define DISCHARGE_CURRENT_CONSUMPTION				CHARGE_CURRENT_CONSUMPTION

/* Battery Parameters to be logged in the SD card */
#define LI_ION										1
#define LI_POLYMER									2

#define BATT_NUMBER_OF_CELLS						6
#define BATT_MAH									BATTERY_CAPACITY
#define BATT_CELL_VOLT_MAX							4.20f
#define BATT_CELL_VOLT_MIN							3.60f
#define BATTERY_TYPE								LI_POLYMER
#define BATT_MAX_PACK_CYCLES						200


/* Battery Configuration Parameters to be stored in ISL EEPROM */
#define CELL_OVER_VOLTAGE_THR_VALUE					4.25f			// In Volt
#define CELL_OV_RECOVERY_VALUE						4.15f			// In Volt
#define CELL_UNDER_VOLTAGE_THR_VALUE				3.50f			// In Volt
#define CELL_UV_RECOVERY_VALUE						3.60f			// In Volt
#define CELL_OV_LOCKOUT_THR_VALUE					4.30f			// In Volt
#define CELL_UV_LOCKOUT_THR_VALUE					2.50f			// In Volt
#define CELL_EOC_THR_VALUE							4.20f			// In Volt
#define INTERNAL_OVER_TEMP_THR_VALUE				65				// In Degrees
#define INTERNAL_OT_RECOVERY_VALUE					50				// In Degrees
#define OV_DELAY_TIMEOUT_VALUE						1				// In seconds
#define	UV_DELAY_TIMEOUT_VALUE						1				// In seconds
#define OPEN_WIRING_TIMEOUT_VALUE					20				// In Millis

#define DELAY_TIMEOUT_MILLIS						0x04
#define DELAY_TIMEOUT_SECONDS						0x08
#define DELAY_TIMEOUT_MINUTES						0x0C

#define OPEN_WIRING_DELAY_TIMEOUT_MILLIS			0x02
#define OPEN_WIRING_DELAY_TIMEOUT_MICROS			0x00

#define OV_DELAY_TIMEOUT_RESOLUTION					DELAY_TIMEOUT_SECONDS
#define UV_DELAY_TIMEOUT_RESOLUTION					DELAY_TIMEOUT_SECONDS
#define OPEN_WIRING_TIMEOUT_RESOLUTION				OPEN_WIRING_DELAY_TIMEOUT_MILLIS


#define CRITICAL_BATT_VOLTAGE						20.1f

/* Enums to define the write results */
enum Write_Result
{
	WRITE_ERROR = 0,WRITE_OK
};

/* Enums to define the current battery status */
enum Pack_Status
{
	DISCHARGING = 0,CHARGING,LOW_POWER_CONSUMPTION
};

/* Enums to define the flags inside the BMS IC registers */
enum Flag_Results
{
	NO = 0, YES
};

/* Enums to define whether BMS is in sleep mode or not */
enum BMS_Sleep_Status
{
	NON_SLEEP_MODE = 0,SLEEP_MODE
};

/* Enums to define whether BMS is in Idle mode or not */
enum BMS_Idle_Status
{
	NON_IDLE_MODE = 0,IDLE_MODE
};

/* Enums to define whether BMS is in Doze mode or not */
enum BMS_Doze_Status
{
	NON_DOZE_MODE = 0,DOZE_MODE
};


/* Enums to define different gain values that can be set in the BMS ASIC */
enum Current_Gain_Values
{
	CURRENT_GAIN_5X = 5, CURRENT_GAIN_50X = 50,CURRENT_GAIN_500X = 500
};

/* Enums to define BMS Health status */
enum Health_Status
{
	HEALTH_NOT_OK = 0, HEALTH_OK
};

enum Operations
{
	WRITE_REGISTER = 0, READ_REGISTER
};

enum Battery_Critical_Status
{
	BATT_LEVEL_OK = 0,BATT_CRITICAL_LEVEL_REACHED
};
/* Structure holding all the flags which are queried from BMS IC and same will be updated to use in code
 * after reading them from registers */
typedef struct
{
	uint8_t Cell_Over_Voltage		:1;
	uint8_t Over_Voltage_Lockout  	:1;
	uint8_t Cell_Under_Voltage		:1;
	uint8_t Under_Voltage_Lockout 	:1;
	uint8_t Discharge_Over_Temp	  	:1;
	uint8_t Discharge_Under_Temp  	:1;
	uint8_t Charge_Over_Temp 	  	:1;
	uint8_t Charge_Under_Temp 	  	:1;

	uint8_t Internal_Over_Temp		:1;
	uint8_t Charge_Overcurrent  	:1;
	uint8_t Discharge_Overcurrent	:1;
	uint8_t Charge_Shortcircuit 	:1;
	uint8_t Cell_Fail			  	:1;
	uint8_t Open_Wire			  	:1;
	uint8_t Res1			 	  	:1;
	uint8_t End_Of_Charge	 	  	:1;

	uint8_t Load_Present			:1;
	uint8_t Charger_Present		  	:1;
	uint8_t Pack_Charging			:1;
	uint8_t Pack_Discharging	 	:1;
	uint8_t Res2					:1;
	uint8_t Res3					:1;
	uint8_t Internal_Scan_Progress 	:1;
	uint8_t Low_Voltage_Charge	  	:1;

	uint8_t Cell_Balance_OT			:1;
	uint8_t Cell_Balance_UT		  	:1;
	uint8_t Cell_Balance_OV			:1;
	uint8_t Cell_Balance_UV		 	:1;
	uint8_t BMS_In_Idle				:1;
	uint8_t BMS_In_Doze				:1;
	uint8_t BMS_In_Sleep		 	:1;
	uint8_t Res4				  	:1;

}BMS_Status_Flags;

/* Structure holding all the status flags for the queried data from BMS ASIC. These flags will give the
 * result of write and read operation happening over I2C */
typedef struct
{
	uint8_t I2C_Init_Flag:1;
	uint8_t I2C_Read_Cells_Flag:1;
	uint8_t I2C_Force_Idle:1;
	uint8_t I2C_Force_Doze:1;
	uint8_t I2C_Force_Sleep:1;
	uint8_t I2C_Read_Status_Flag:1;
	uint8_t I2C_Read_Pack_Volt_Flag:1;
	uint8_t I2C_Read_Pack_Current_Flag:1;
	uint8_t I2C_Read_Pack_Temp_Flag:1;
	uint8_t I2C_Set_Current_Gain_Flag:1;

	uint8_t I2C_Set_OV_Thresh_Flag:1;
	uint8_t I2C_Set_OV_Recovery_Flag:1;
	uint8_t I2C_Set_UV_Thresh_Flag:1;
	uint8_t I2C_Set_UV_Recovery_Flag:1;
	uint8_t I2C_Set_OV_Lockout_Thresh_Flag:1;
	uint8_t I2C_Set_UV_Lockout_Thresh_Flag:1;
	uint8_t I2C_Set_EOC_Thresh_Flag:1;
	uint8_t I2C_Set_IOT_Thresh_Flag:1;

	uint8_t I2C_Set_IOT_Recovery_Flag:1;
	uint8_t I2C_Disable_Cell_Balancing_Flag:1;
	uint8_t I2C_Set_OV_Delay_Timeout_Flag:1;
	uint8_t I2C_Set_UV_Delay_Timeout_Flag:1;
	uint8_t I2C_Set_Open_Wiring_Timeout_Flag:1;
	uint8_t I2C_Config_Number_of_Cells_Flag:1;

}I2C_Errors;

/* Structure holding all the variables related to the data queried from BMS ASIC and which are to be logged
 * on SD card and used in the code */
typedef struct
{
	float Cell1_Voltage;
	float Cell2_Voltage;
	float Cell3_Voltage;
	float Cell4_Voltage;
	float Cell5_Voltage;
	float Cell6_Voltage;
	float Cell7_Voltage;
	float Cell8_Voltage;

	float Pack_Voltage;
	float Pack_Current;
	float Pack_Current_Adjusted;
	float Pack_Temperature_Degrees;

	float Pack_Capacity_Remaining;
	float Pack_Capacity_Used;
	float Pack_Charge_Discharge_Rate;

	uint32_t Pack_Charge_Cycles;
	uint32_t Pack_Discharge_Cycles;
	uint32_t Pack_Total_Cycles;
	uint8_t Health_Status_Info[40];
	uint8_t I2C_Error_Info[40];

	uint8_t Charging_Discharging_Status;

}ISL_943203_Data;

extern ISL_943203_Data BMS_Data;

extern BMS_Status_Flags Status_Flag;

extern I2C_Errors I2C_Error_Flag;

extern uint8_t Last_Charge_Disharge_Status;

extern uint16_t Current_Gain;

extern double C_D_Accumulated_mAH;

extern uint32_t Error_Check_Data;

/* Constant battery parameters */
extern const uint8_t Battery_ID[];
extern const uint8_t BMS_Board_Serial_Number[];

/* BMS IC configurations functions which will get called only at the start of the code */
uint8_t BMS_Configure_Parameters(void);

/* Function prototypes defined in the BMS_ASIC.c file */
void BMS_ASIC_Init();
void BMS_Force_Idle();
void BMS_Force_Doze();
void BMS_Force_Sleep();
void BMS_Set_Current_Gain(uint16_t Gain_Setting);
void BMS_Update_Pack_Cycles(void);
void BMS_Read_RAM_Status_Register(void);
void BMS_Read_Cell_Voltages(void);
void BMS_Estimate_Initial_Capacity(void);
void BMS_Estimate_Capacity_Used(void);
void BMS_Read_Pack_Voltage(void);
void BMS_Read_Pack_Current(void);
void BMS_Read_Pack_Temperature(void);
uint8_t BMS_Read_Number_Of_Cells_Configuration();

float Get_BMS_Pack_Current_Adj();
float Get_BMS_Accumulated_Pack_Voltage();

float Constrain(float Variable, float Lower_Limit, float Upper_Limit);

uint8_t BMS_Check_COM_Health();
uint8_t BMS_Check_Critical_Voltage();

uint8_t Get_BMS_Charge_Discharge_Status(void);
uint8_t Get_BMS_Idle_Mode_Status();
uint8_t Get_BMS_Doze_Mode_Status();
uint8_t Get_BMS_Sleep_Mode_Status();
float Get_Cell1_Voltage(void);
float Get_Cell2_Voltage(void);
float Get_Cell3_Voltage(void);
float Get_Cell6_Voltage(void);
float Get_Cell7_Voltage(void);
float Get_Cell8_Voltage(void);
float Get_BMS_Pack_Voltage(void);
float Get_BMS_Capacity_Remaining();
float Get_BMS_Capacity_Used(void);
float Get_BMS_Pack_Current(void);
float Get_BMS_Pack_Temperature(void);
float Get_BMS_Charge_Discharge_Rate();
uint32_t Get_BMS_Total_Pack_Cycles();


/* Values to be loaded into the ISL for parameters
 * temperature in hex = 0x5A6 for 70 degrees (0.635mV)
 * */
#endif /* BMS_BMS_H_ */
