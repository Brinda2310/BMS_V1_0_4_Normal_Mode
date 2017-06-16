/**
 ******************************************************************************
 * @file    Main.c
 * @author  Nikhil Ingale
 * @version V1.0.0
 * @date    27-May-2017
 * @brief   Default main function.
 ******************************************************************************
*/
#include <BMS_ASIC.h>
#include <BMS_Data_Log.h>
#include <BMS_Serial_Communication.h>
#include <BMS_GPIOs.h>
#include <BMS_Timing.h>
#include <Power_Management.h>
#include <AP_Communication.h>

const uint8_t BMS_Firmware_Version[3] =
{
		1,			// Major release version--modified when code is being merged to Master branch.
		0,			// Current stable code release-- modified when code is being merged to Develop branch.
		1			// Beta code release--modified when code is being merged to test_develop branch.
};

/* Variable to keep the track of time elapsed when switch is pressed for short duration i.e. 2 seconds */
uint16_t Switch_Press_Time_Count = 0;
/* Variable to keep the track of time when there is no current consumption so as to force the ISL to sleep */
uint16_t BMS_Sleep_Time_Count = 0;
/* Variable to keep the track of time when ISL goes to sleep so as to put MCU in sleep mode */
uint16_t MCU_Sleep_Time_Count = 0;

/* Variable to keep the track of time since charging is started and increment the cycles accordingly */
uint16_t Charge_Time_Count = 0;
/* Variable to keep the track of time since discharging is started and increment the cycles accordingly */
uint16_t Discharge_Time_Count = 0;

/* Variable to avoid multiple increments of the same cycle either charging or discharging */
bool Update_Pack_Cycles = false;
uint8_t Start_Charging = -1;

uint8_t RecData = 0;

bool ISL_Sleep = false;
bool MCU_Sleep = false;
volatile bool Short_Time_Elapsed = false,Long_Time_Elapsed = false;

int main(void)
{
	/* Configure the sysTick interrupt to 1mS(default) and Set the NVIC group priority to 4 */
	HAL_Init();

	/* Configure the system clock frequency (Peripherals clock) to 80MHz */
	Set_System_Clock_Frequency();

	/* Delay of 2 Seconds is required to make sure BMS is not polled before it's POR cycle otherwise
	 * BMS I2C will be locked */
	Delay_Millis(2000);

	/* Initialize the timer to 40mS and the same is used to achieve different loop rates */
	BMS_Timers_Init();

	/* Initialize the USART to 115200 baud rate to debug the code */
#if DEBUG_COM == ENABLE
	BMS_Debug_COM_Init();
#endif
	/* Initialize the status LEDs which indicates the SOC and SOH */
	BMS_Status_LEDs_Init();

	/* Configure the switch as input to wake up the BMS in case of sleep and same will be used
	 * to show the SOC and SOH on status LEDs*/
	BMS_Switch_Init();

	/* Configure the ISL94203 I2C communication to 100KHz */
	BMS_ASIC_Init();

	/* Initialize the communication between AP and BMS; Current version of BMS supports SMBUS protocol */
	AP_COM_Init(AP_COM_SMBUS_MODE);

	/* Initialize the RTC and set the RTC time and date to the date and time received from GPS */
	RTC_Init();

	/* Set the current gain in the BMS ASIC register */
	BMS_Set_Current_Gain(CURRENT_GAIN_5X);

	/* Read the pack voltage to calculate the battery capacity used/remaining */
	BMS_Read_Pack_Voltage();

	/* Create the LOG file on SD card by reading the count from log summary file */
	if(BMS_Log_Init() == RESULT_OK)
	{
#if  DEBUG_MANDATORY == ENABLE
		BMS_Debug_COM_Write_Data("Log_file_Created\r", 17);
#endif
	}
	else
	{
#if DEBUG_MANDATORY == ENABLE
		BMS_Debug_COM_Write_Data("SD Card Not Present\r", 20);
#endif
	}

	/* Calculate the battery capacity used and remaining so that same value will be used to estimate
	 * next values */
	BMS_Estimate_Initial_Capacity();

	/* Initial state of charging/discharging required to calculate the pack cycles used */
	if(Get_BMS_Charge_Discharge_Status() == CHARGING)
	{
		Last_Charge_Disharge_Status = CHARGING;
	}
	else
	{
		Last_Charge_Disharge_Status = DISCHARGING;
	}

	/* Section of the code just to see the correct value of battery capacity remaining over USART */
#if DEBUG_OPTIONAL == ENABLE
	char Buffer[30],Len = 0;
	Len = sprintf(Buffer,"%f",Get_BMS_Capacity_Remaining());
	BMS_Debug_COM_Write_Data(Buffer, Len);
#endif

	while(1)
	{
		if(Wakeup_From_Sleep == true)
		{
			/* Initialize the communication between AP and BMS; Current version of BMS supports SMBUS protocol */
			AP_COM_Init(AP_COM_SMBUS_MODE);

			/* Initialize the RTC and set the RTC time and date to the date and time received from GPS */
			RTC_Init();

			/* Set the current gain in the BMS ASIC register */
			BMS_Set_Current_Gain(CURRENT_GAIN_5X);

			/* Read the pack voltage to calculate the battery capacity used/remaining */
			BMS_Read_Pack_Voltage();

			/* Create the LOG file on SD card by reading the count from log summary file */
			if(BMS_Log_Init() == RESULT_OK)
			{
				Wakeup_From_Sleep = false;
		#if  DEBUG_MANDATORY == ENABLE
				BMS_Debug_COM_Write_Data("Log_file_Created\r", 17);
		#endif
			}
			else
			{
		#if DEBUG_MANDATORY == ENABLE
				BMS_Debug_COM_Write_Data("SD Card Not Present\r", 20);
		#endif
				Wakeup_From_Sleep = false;
			}

			/* Calculate the battery capacity used and remaining so that same value will be used to estimate
			 * next values */
			BMS_Estimate_Initial_Capacity();

			/* Initial state of charging/discharging required to calculate the pack cycles used */
			if(Get_BMS_Charge_Discharge_Status() == CHARGING)
			{
				Last_Charge_Disharge_Status = CHARGING;
			}
			else
			{
				Last_Charge_Disharge_Status = DISCHARGING;
			}


		}
		BMS_Debug_COM_Read_Data(&RecData,1);

		if(RecData == 'A')
		{
			NVIC_SystemReset();
		}
		RecData = 0;
		/* This flag will be true after every 40mS in timer application file */
		if (_30Hz_Flag == true)
		{
			/* If the switch is pressed for shorter period of time(2 Seconds) then show SOC on status LEDs
			 * else if it is pressed for long4er period of time (4 Seconds) then show SOH status on LEDs */
			if (BMS_Read_Switch_Status() == PRESSED)
			{
				Switch_Press_Time_Count++;
				if(Switch_Press_Time_Count >= LONG_PEROID)
				{
					Long_Time_Elapsed = true;
				}

				if(Switch_Press_Time_Count >= SHORT_PERIOD)
				{
					Short_Time_Elapsed = true;
				}
			}
			else
			{
				/* If switch is immediately released then reset time count to zero */
				Switch_Press_Time_Count = 0;
			}

			/* If switch is pressed for more than 2 Seconds then show SOH status on LEDs*/
			if (Long_Time_Elapsed == true)
			{
				BMS_Show_LED_Pattern(SOH);
				Short_Time_Elapsed = false;
				Long_Time_Elapsed = false;
				Switch_Press_Time_Count = 0;
			}

			/* If switch is pressed for 1 seconds and released then show the SOC status on LEDs */
			if (Short_Time_Elapsed == true)
			{
				BMS_Show_LED_Pattern(SOC);
				Short_Time_Elapsed = false;
			}

			/* Query the BMS data at 30Hz; All cell voltages, pack voltage, pack current, pack temperature
			 * all status flags and calculate the battery capacity used */
			BMS_Read_Cell_Voltages();
			BMS_Read_Pack_Voltage();
			BMS_Read_Pack_Current();
			BMS_Read_Pack_Temperature();
			BMS_Read_RAM_Status_Register();
			BMS_Estimate_Capacity_Used();

			BMS_Status_LED_Toggle();
			/* If current consumption is less than 100mA for continuous 5 minutes and if BMS IC is not
			 * in sleep mode then MCU forces the BMS IC to sleep mode */
			if(((uint16_t)Get_BMS_Pack_Current() < MINIMUM_CURRENT_CONSUMPTION) && Status_Flag.BMS_In_Sleep == NO)
			{
				BMS_Sleep_Time_Count++;

				if(BMS_Sleep_Time_Count >= LOW_CONSUMPTION_DELAY)
				{
					BMS_Sleep_Time_Count = 0;
					BMS_Force_Sleep();
				}
			}
			else if (((uint16_t)Get_BMS_Pack_Current() > MINIMUM_CURRENT_CONSUMPTION))
			{
			   /* If BMS consumes more than 100mA in between then reset the time count to zero */
				BMS_Sleep_Time_Count = 0;
				ISL_Sleep = false;
			}

			/* If BMS IC is forced to sleep mode then MCU goes to sleep mode after 20 Seconds */
			if(Status_Flag.BMS_In_Sleep == YES)
			{
				ISL_Sleep = true;
				MCU_Sleep_Time_Count++;
				if(MCU_Sleep_Time_Count >= MCU_GO_TO_SLEEP_DELAY)
				{
					MCU_Sleep = true;
					MCU_Sleep_Time_Count = 0;
#if DEBUG_MANDATORY == ENABLE
					BMS_Debug_COM_Write_Data("MCU Went to sleep\r",18);
					Delay_Millis(5);
#endif
					MCU_Enter_Sleep_Mode();
				}
			}
			else
			{
				MCU_Sleep_Time_Count = 0;
			}

			/* If external charger is connected to the BMS then keep continuous track of it*/
			if(Get_BMS_Charge_Discharge_Status() == CHARGING)
			{
				Discharge_Time_Count = 0;
				/* If current coming into the pack is more than 1amperes then start counting the time */
				if(Get_BMS_Pack_Current() > CHARGE_CURRENT_CONSUMPTION && Update_Pack_Cycles == false)
				{
					Start_Charging = 1;
					Charge_Time_Count++;
				}
				else
				{
					Charge_Time_Count = 0;
				}

				/* If current coming into the pack is more than 1amperes for more than 5mins(CHARGE_TIME_DELAY), then increment
				 * the charge cycles count and make the status to of variable to true so as to keep track of last state of the pack i.e. charging/discharging  */
				if(Charge_Time_Count >= CHARGE_TIME_DELAY && Last_Charge_Disharge_Status != CHARGING)
				{
					BMS_Data.Pack_Charge_Cycles++;
					Update_Pack_Cycles = true;
					Last_Charge_Disharge_Status = CHARGING;
					BMS_Update_Pack_Cycles();
					Charge_Time_Count = 0;
				}
			}
			/* If status of the BMS is discharging then keep continuing the track of it  */
			else if (Get_BMS_Charge_Discharge_Status() == DISCHARGING)
			{
				/* Make the count used for charging to zero to get the exact duration of 5mins in case of charging */
				Charge_Time_Count = 0;
				/* If pack going out of the pack is more than 1 amperes then start counting the time */
				if(Get_BMS_Pack_Current() > DISCHARGE_CURRENT_CONSUMPTION && Update_Pack_Cycles == false)
				{
					Start_Charging = 2;
					Discharge_Time_Count++;
				}
				else
				{
					Discharge_Time_Count = 0;
				}

				/* If discharge current is more than 1 amperes for more than 5 minutes then increment the discharge cycles count and also make sure that
				 * the previous state of the pack was not discharging */
				if(Discharge_Time_Count >= DISCHARGE_TIME_DELAY && Last_Charge_Disharge_Status != DISCHARGING)
				{
					BMS_Data.Pack_Discharge_Cycles++;
					Update_Pack_Cycles = true;
					Last_Charge_Disharge_Status = DISCHARGING;
					BMS_Update_Pack_Cycles();
					Discharge_Time_Count = 0;
				}
			}
			else
			{
				/* If BMS is in low power consumption mode then clear all the timer counts and
				 * do not update any cycles count */
				Charge_Time_Count = 0;
				Discharge_Time_Count = 0;
				Update_Pack_Cycles = false;
			}

			_30Hz_Flag = false;
		}

		/* Log the BMS variables on SD card at 1Hz */
		if(_1Hz_Flag == true)
		{
			if(Start_Charging == 1)
			{
#if DEBUG_MANDATORY == ENABLE
				BMS_Debug_COM_Write_Data("Charging\r",9);
#endif
			}
			else if (Start_Charging == 2)
			{
#if DEBUG_MANDATORY == ENABLE
				BMS_Debug_COM_Write_Data("Discharging\r",12);
				Delay_Millis(5);
#endif
			}

			if(ISL_Sleep == false)
			{
#if DEBUG_MANDATORY == ENABLE
				char Buffer[200];
				Delay_Millis(2);
				uint8_t Length1 = sprintf(Buffer,"Pack_Voltage = %0.3fV\r",Get_BMS_Pack_Voltage());
				BMS_Debug_COM_Write_Data(Buffer,Length1);
				Delay_Millis(5);

				Length1 = sprintf(Buffer,"Pack_Current = %0.3fmA\r",Get_BMS_Pack_Current());
				BMS_Debug_COM_Write_Data(Buffer,Length1);
				Delay_Millis(5);

//				Length1 = sprintf(Buffer,"Pack_Temp = %0.3f Degrees\r",Get_BMS_Pack_Temperature());
//				BMS_Debug_COM_Write_Data(Buffer,Length1);
//				Delay_Millis(3);
//
//				uint16_t Length = 0;
//				Length += sprintf(Buffer, "Batt used = %0.3fmAH\r", Get_BMS_Capacity_Used());
//				Length += sprintf(&Buffer[Length], "Batt remaining = %0.3f\r", Get_BMS_Capacity_Remaining());
//				Length += sprintf(Buffer, "Cell1_V = %0.3fV\r", Get_Cell1_Voltage());
//				Length += sprintf(&Buffer[Length], "Cell2_V = %0.3fV\r",Get_Cell2_Voltage());
//				Length += sprintf(&Buffer[Length], "Cell3_V = %0.3fV\r",Get_Cell3_Voltage());
//				Length += sprintf(&Buffer[Length], "Cell6_V = %0.3fV\r",Get_Cell6_Voltage());
//				Length += sprintf(&Buffer[Length], "Cell7_V = %0.3fV\r",Get_Cell7_Voltage());
//				Length += sprintf(&Buffer[Length], "Cell8_V = %0.3fV\r",Get_Cell8_Voltage());

//				BMS_Debug_COM_Write_Data(Buffer, Length);
//				Delay_Millis(10);
#endif
			}
			if (Log_All_Data() == RESULT_OK)
			{
#if DEBUG_MANDATORY == ENABLE
				BMS_Debug_COM_Write_Data("Written\r",8);
#endif
			}
			else
			{
#if DEBUG_MANDATORY == ENABLE
				BMS_Debug_COM_Write_Data("Write Error\r",12);
#endif
			}
			_1Hz_Flag = false;
		}

		Check_AP_Request();
	}
}

