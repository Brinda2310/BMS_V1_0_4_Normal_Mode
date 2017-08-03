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
uint32_t Switch_Press_Time_Count = 0;
/* Variable to keep the track of time when there is no current consumption so as to force the ISL to sleep */
uint32_t BMS_Sleep_Time_Count = 0;
/* Variable to keep the track of time when ISL goes to sleep so as to put MCU in sleep mode */
uint32_t MCU_Sleep_Time_Count = 0;

/* Variable to keep the track of time since charging is started and increment the cycles accordingly */
uint32_t Charge_Time_Count = 0;
/* Variable to keep the track of time since discharging is started and increment the cycles accordingly */
uint32_t Discharge_Time_Count = 0;

/* Variable to avoid multiple increments of the same cycle either charging or discharging */
bool Update_Pack_Cycles = false;

/* Debug code variables definition; to be removed after testing */
char Buffer[400];
uint8_t Length = 0;
uint8_t RecData = 0;
uint8_t Start_Charging = -1;

/* Debug code to be removed after testing the RTC working */
uint8_t RTC_Time[20];

/* Variable to hold the timing value to force the BMS IC to sleep mode; Values are set using macros defined
 * in BMS_Timing.h file. If MCU is awaken from sleep mode then check load presence for 10 seconds
 * otherwise check load presence for 1 minute(normal operation) */
uint16_t Timer_Value = 0;

/* Debug variable to be removed after testing */
bool ISL_Sleep = false;
bool Display_Volt_Current = false;
bool Data_Written = false;

int main(void)
{
	/* Configure the sysTick interrupt to 1mS(default) and Set the NVIC group priority to 4 */
	HAL_Init();

	/* Configure the system clock frequency (Peripherals clock) to 80MHz */
	Set_System_Clock_Frequency();

	/* Delay of 2 Seconds is required to make sure BMS is not polled before it's POR cycle otherwise
	 * BMS I2C will be locked */
	Delay_Millis(2000);

	/* Initialize the timer to 33mS(30Hz) and the same is used to achieve different loop rates */
	BMS_Timers_Init();

	/* Initialize the USART to 115200 baud rate to debug the code */
	BMS_Debug_COM_Init();

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

	/* Set the current gain in the BMS ASIC register. After having number of iterations and analyzing
	 * the curves we will decide which gain is suitable for which current range(Amperes) */
	BMS_Set_Current_Gain(CURRENT_GAIN_5X);

	/* Read the pack voltage to calculate the battery capacity used/remaining */
	BMS_Read_Pack_Voltage();

	/* Create the LOG file on SD card by reading the count from log summary file */
	if(BMS_Log_Init() == RESULT_OK)
	{
//		BMS_Debug_COM_Write_Data("Log_file_Created\r", 17);
	}
	else
	{
//		BMS_Debug_COM_Write_Data("SD Card Not Present\r", 20);
	}

	/* Calculate the battery capacity used and remaining so that same value will be used to estimate
	 * next values */
	BMS_Estimate_Initial_Capacity();

	/* Initial state of charging/discharging required to calculate the pack cycles(C/D) used */
	if(Get_BMS_Charge_Discharge_Status() == CHARGING)
	{
		Last_Charge_Disharge_Status = CHARGING;
	}
	else
	{
		Last_Charge_Disharge_Status = DISCHARGING;
	}

	/* Debug code just to see the correct value of battery capacity remaining which can be seen on USART */
//	Length = sprintf(Buffer,"Batt Remaining %f",Get_BMS_Capacity_Remaining());
//	BMS_Debug_COM_Write_Data(Buffer, Length);

	/* Every time when MCU restarts, set the load check timer value to 1 minute; If MCU is awaken from sleep
	 * mode then set this value to 10 seconds; If load is not present for 10 seconds then force BMS to
	 * sleep mode again */
	Timer_Value = LOW_CONSUMPTION_DELAY;

	while(1)
	{
		/* If MCU is awaken from sleep mode then we have to reinitialize all the peripherals */
		if(Wakeup_From_Sleep == true)
		{
			/* This variable decides the time after which MCU will go to sleep if load is not present */
			Timer_Value = LOW_CONSUMPTION_DELAY_AFTER_WAKEUP;

			/* Initialize the communication between AP and BMS; Current version of BMS supports SMBUS protocol */
			AP_COM_Init(AP_COM_SMBUS_MODE);

			/* Initialize the RTC and set the RTC time and date to the date and time received from GPS */
			RTC_Init();

			/* Set the current gain in the BMS ASIC register. After having number of iterations and
			 * analyzing the curves we will decide which gain is suitable for which current range(Amperes) */
			BMS_Set_Current_Gain(CURRENT_GAIN_5X);

			/* Read the pack voltage to calculate the battery capacity used/remaining */
			BMS_Read_Pack_Voltage();

			/* Create the LOG file on SD card by reading the count from log summary file; Debug code is
			 * to be removed after testing; We can decide to show some statuses on LEDs if there is
			 * problem with the SD card */
			if(BMS_Log_Init() == RESULT_OK)
			{
				BMS_Debug_COM_Write_Data("Log_file_Created\r", 17);
			}
			else
			{
				BMS_Debug_COM_Write_Data("SD Card Not Present\r", 20);
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
			/* This flag must be cleared to avoid reinitializing all the peripherals again and again */
			Wakeup_From_Sleep = false;
			ISL_Sleep = false;

			/* Debug code; To be removed after testing */
			BMS_Debug_COM_Write_Data("Wake up from sleep\r",19);
		}

		/* Debug code to be removed after testing */
		BMS_Debug_COM_Read_Data(&RecData,1);

		if(RecData == 'A')
		{
			/* Debug code to set the last charge/discharge status to charging to see whether discharge
			 * pack cycles are getting updated properly or not;Because discharging cycles will be updated
			 * only if last state was charging */
			NVIC_SystemReset();
		}
		else if (RecData == 'B')
		{
			AP_COM_Init(AP_COM_SMBUS_MODE);
		}
		else if(RecData == 'C')
		{
			Last_Charge_Disharge_Status = CHARGING;
		}
		else if (RecData == 'D')
		{
			Last_Charge_Disharge_Status = DISCHARGING;
		}
		else if (RecData == 'E')
		{
			Stop_Log();
		}
		else if (RecData == 'F')
		{
			BMS_Log_Init();
		}
		RecData = 0;

		/* This flag will be true after every 33mS(30Hz) in timer application file */
		if (_30Hz_Flag == true)
		{
			/* If switch is pressed then start counting the time */
			if (BMS_Read_Switch_Status() == PRESSED)
			{
				Switch_Press_Time_Count++;
			}
			else
			{
				/* As soon as switch is released, check for timer count value. If count is in between
				 * 500ms and 1 seconds then show the SOC status on LEDs and if count is more than 2
				 * seconds then show SOH status on LEDs */
				if(Switch_Press_Time_Count >= SHORT_PERIOD && Switch_Press_Time_Count <= LONG_PEROID)
				{
					BMS_Show_LED_Pattern(SOC);
				}
				else if(Switch_Press_Time_Count >= LONG_PEROID)
				{
					BMS_Show_LED_Pattern(SOH);
				}

				/* If switch is immediately released then reset time count to zero */
				Switch_Press_Time_Count = 0;
			}

			/* Query the BMS data at 30Hz; All cell voltages, pack voltage, pack current, pack temperature
			 * all status flags and calculate the battery capacity used */
			BMS_Read_Cell_Voltages();
			BMS_Read_Pack_Voltage();
			BMS_Read_Pack_Current();
			BMS_Read_Pack_Temperature();
			BMS_Read_RAM_Status_Register();
			BMS_Estimate_Capacity_Used();

			/* If current consumption is less than 200mA and BMS IC is not in sleep mode then start
			 * counting the timer value */
		if(Sleep_Mode_Funtionality == ENABLE)
		{
			if(((uint16_t)Get_BMS_Pack_Current() < MINIMUM_CURRENT_CONSUMPTION) && Status_Flag.BMS_In_Sleep == NO)
			{
				BMS_Sleep_Time_Count++;

				/* If MCU is awaken by external switch or load then check the load only for 10 seconds.
				 * If load is not present for continuous 10 seconds then force BMS IC again to sleep mode
				 * and if load is present then check the presence of load for continuous 1 minute */
				if(BMS_Sleep_Time_Count >= Timer_Value)
				{
					BMS_Sleep_Time_Count = 0;
					/* Set the corresponding flag in BMS IC to force it to sleep mode */
					BMS_Force_Sleep();
				}
			}
			/* If some load is present then always clear the timer counts to zero */
			else if (((uint16_t)Get_BMS_Pack_Current() > MINIMUM_CURRENT_CONSUMPTION))
			{
			   /* If BMS consumes more than 200mA in between then reset the time count to zero */
				BMS_Sleep_Time_Count = 0;
				/* Debug flag to be removed after testing is completed */
				ISL_Sleep = false;
				/* If load is present then change the timer check value for sleep to 1 minute */
				Timer_Value = LOW_CONSUMPTION_DELAY;
			}

			/* If BMS IC is forced to sleep mode then start counting the timer value; If BMS IC goes to
			 * sleep mode immediately after wake up then MCU will go to sleep after 5 seconds */
			if(Status_Flag.BMS_In_Sleep == YES)
			{
				ISL_Sleep = true;
				MCU_Sleep_Time_Count++;
				/* When BMS IC goes to sleep then MCU also goes to sleep mode after 5 Seconds */
				if(MCU_Sleep_Time_Count >= MCU_GO_TO_SLEEP_DELAY)
				{
					MCU_Sleep_Time_Count = 0;
					/* This flag makes sure that controller is wake up from sleep mode only */
					Sleep_Mode = true;
					/* Debug code to be removed after testing is completed */
					BMS_Debug_COM_Write_Data("MCU Went to sleep\r",18);
					/* Configures the external trigger events which will wake up the MCU and then goes to
					 * sleep mode */
//					MCU_Enter_Sleep_Mode();
				}
			}
			/* If BMS IC is not in sleep mode then always reset the timer count to zero so as to get
			 * exact timer value for next iterations */
			else
			{
				MCU_Sleep_Time_Count = 0;
				/* Debug flag to be removed after testing */
				ISL_Sleep = false;
			}
		}
		else
		{
			/* Need to clear the counts as soon as sleep mode is disabled so that from next time onwards
			 * it will start from zero count and we will get the exact timing counts */
			BMS_Sleep_Time_Count = 0;
			MCU_Sleep_Time_Count = 0;
		}

			/* Once MCU wakes up, it starts the execution from where it had left off thats why it is necessary
			 * to check whether MCU started from sleep mode or executing it normal way */
			if(Wakeup_From_Sleep == false)
			{
				/* If external charger is connected to the BMS then keep continuous track of it*/
				if(Get_BMS_Charge_Discharge_Status() == CHARGING)
				{
					/* Make the count used for charging to zero to get the exact duration of 5mins while
					 * executing the discharge section of the code */
					Discharge_Time_Count = 0;
					/* If current coming into the pack is more than 1amperes then start counting the time */
					if(Get_BMS_Pack_Current() > CHARGE_CURRENT_CONSUMPTION && Update_Pack_Cycles == false)
					{
						Charge_Time_Count++;
					}
					else
					{
						/* BMS is charging but with value less than mentioned(1A)then reset the timer
						 * count to zero */
						Charge_Time_Count = 0;
					}

					/* If current coming into the pack is more than 1amperes for more than 5mins(CHARGE_TIME_DELAY),
					 * then increment the charge cycles count and make the status to of variable to true so as to
					 * keep track of last state of the pack i.e. charging/discharging  */
					if(Charge_Time_Count >= CHARGE_TIME_DELAY && Last_Charge_Disharge_Status != CHARGING)
					{
						Start_Charging = 1;
						BMS_Data.Pack_Charge_Cycles++;
						Update_Pack_Cycles = true;
						Last_Charge_Disharge_Status = CHARGING;
						BMS_Update_Pack_Cycles();
//						char Charge[10];
//						uint8_t Len = sprintf(Charge,"C cycles = %d\r",(int)BMS_Data.Pack_Total_Cycles);
//						BMS_Debug_COM_Write_Data(Charge,Len);
//						Delay_Millis(5);
					}
				}
				/* If status of the BMS is discharging then keep continuous track of it */
				else if (Get_BMS_Charge_Discharge_Status() == DISCHARGING)
				{
					/* Make the count used for charging to zero to get the exact duration of 5mins while
					 * executing the charging section of the code */
					Charge_Time_Count = 0;
					/* If current going out of the pack is more than 1 amperes then start counting the time */
					if(Get_BMS_Pack_Current() > DISCHARGE_CURRENT_CONSUMPTION && Update_Pack_Cycles == false)
					{
						Discharge_Time_Count++;
					}
					else
					{
						Discharge_Time_Count = 0;
					}

					/* If discharge current is more than 1 amperes for more than 5 minutes then increment
					 * the discharge cycles count by ensuring that the previous state of the pack was
					 * not discharging */
					if(Discharge_Time_Count >= DISCHARGE_TIME_DELAY && Last_Charge_Disharge_Status != DISCHARGING)
					{
						Start_Charging = 2;
						BMS_Data.Pack_Discharge_Cycles++;
						Update_Pack_Cycles = true;
						Last_Charge_Disharge_Status = DISCHARGING;
						BMS_Update_Pack_Cycles();
//						char Charge[10];
//						uint8_t Len = sprintf(Charge,"C cycles = %d\r",(int)BMS_Data.Pack_Total_Cycles);
//						BMS_Debug_COM_Write_Data(Charge,Len);
//						Delay_Millis(5);

					}
				}
				else
				{
					/* If BMS is in low power consumption mode then clear all the timer counts and
					 * do not update any cycles(C/D) count */
					Charge_Time_Count = 0;
					Discharge_Time_Count = 0;
					Update_Pack_Cycles = false;
					Start_Charging = 0;
					Display_Volt_Current = false;
				}
			}

			if (Log_All_Data() != RESULT_OK)
			{
				BMS_Debug_COM_Write_Data("Write error\r",12);
			}
			else
			{
				Data_Written = true;
			}

			if(BMS_Check_COM_Health() != HEALTH_OK)
			{
				BMS_ASIC_Init();
				BMS_Debug_COM_Write_Data("ASIC Restart\r",13);
				Delay_Millis(3);
			}
//			BMS_Status_LED_Toggle();

			_30Hz_Flag = false;
		}
		/* Log the BMS variables on SD card at 1Hz;Make sure that MCU has initialized all the peripherals
		 * before using it.After wake up this flag will be true and will become false once all peripherals
		 * are initialized properly */
		if(_1Hz_Flag == true && Wakeup_From_Sleep == false)
		{
			if(Data_Written == true)
			{
				Data_Written = false;
//				BMS_Debug_COM_Write_Data("Written\r",8);
//				Length = sprintf(Buffer,"Volt = %0.3fV\r",Get_BMS_Pack_Voltage());
//				Length += sprintf(&Buffer[Length], "Cell1_V = %0.3fV\r",Get_Cell1_Voltage());
//				BMS_Debug_COM_Write_Data(Buffer, Length);
			}
			/* Debug code to be removed after testing is done;
			 * If BMS IC is in sleep mode then throw "Sleep Mode" string on USART otherwise throw
			 * "Non Sleep Mode" string on USART */
//			if(Get_BMS_Sleep_Mode_Status() == SLEEP_MODE)
//			{
//				BMS_Debug_COM_Write_Data("Sleep Mode\r",11);
//			}
//			else if (Get_BMS_Sleep_Mode_Status() == NON_SLEEP_MODE)
//			{
//				BMS_Debug_COM_Write_Data("Non Sleep Mode\r",15);
//			}

//			Length = sprintf(Buffer,"State = %d\r",Last_Charge_Disharge_Status);
//			BMS_Debug_COM_Write_Data(Buffer,Length);
//			Delay_Millis(2);

			if(Start_Charging == 1)
			{
//				Length = sprintf(Buffer,"C T = %d\r",Charge_Time_Count);
//				BMS_Debug_COM_Write_Data(Buffer,Length);
//				Delay_Millis(2);
//				BMS_Debug_COM_Write_Data("Charging\r",9);
				Display_Volt_Current = true;
//				Delay_Millis(2);
//				Start_Charging = 0;
			}
			else if(Start_Charging == 2)
			{
//				Length = sprintf(Buffer,"DC T = %d\r",Discharge_Time_Count);
//				BMS_Debug_COM_Write_Data(Buffer,Length);
//				Delay_Millis(2);
//				Start_Charging = 0;
//				BMS_Debug_COM_Write_Data("Discharging\r",12);
//				Delay_Millis(2);
				Display_Volt_Current = true;
			}

			/* Debug code to be removed after testing is done;
			 * If BMS IC is not in sleep mode then only show all the data on USART */
			if(ISL_Sleep == true)
			{
				uint8_t Length = 0;
				if(Display_Volt_Current == true)
				{
					Length = sprintf(Buffer,"Volt = %0.3fV\r",Get_BMS_Pack_Voltage());
					Length += sprintf(&Buffer[Length],"Current = %0.3fmA\r",Get_BMS_Pack_Current());
				}
				else
				{
					Length = sprintf(Buffer,"Volt = %0.3fV\r",Get_BMS_Pack_Voltage());
					Length += sprintf(&Buffer[Length],"Current = %0.3fmA\r",Get_BMS_Pack_Current());
					Length += sprintf(&Buffer[Length],"Temp = %0.3f Degrees\r",Get_BMS_Pack_Temperature());
					Length += sprintf(&Buffer[Length], "Batt used = %0.3fmAH\r", Get_BMS_Capacity_Used());
					Length += sprintf(&Buffer[Length], "Batt remaining = %0.3f%c\r", Get_BMS_Capacity_Remaining(),'%');
					Length += sprintf(&Buffer[Length], "Cell1_V = %0.3fV\r",Get_Cell1_Voltage());
					Length += sprintf(&Buffer[Length], "Cell2_V = %0.3fV\r",Get_Cell2_Voltage());
					Length += sprintf(&Buffer[Length], "Cell3_V = %0.3fV\r",Get_Cell3_Voltage());
					Length += sprintf(&Buffer[Length], "Cell6_V = %0.3fV\r",Get_Cell6_Voltage());
					Length += sprintf(&Buffer[Length], "Cell7_V = %0.3fV\r",Get_Cell7_Voltage());
					Length += sprintf(&Buffer[Length], "Cell8_V = %0.3fV\r",Get_Cell8_Voltage());
				}
//				BMS_Debug_COM_Write_Data(Buffer, Length);
			}
			/* Debug code to be removed after testing the RTC working as per the date set by AP */
//			Delay_Millis(5);
//			RTC_TimeShow(RTC_Time);
//			BMS_Debug_COM_Write_Data(RTC_Time,18);
			if(Restart_SMBus == true)
			{
				Restart_SMBus = false;
			}

			BMS_Status_LED_Toggle();
			_1Hz_Flag = false;
		}

		/* Check for any request is received from AP; Also check for any data is received from AP which
		 * may be used to update the BMS RTC and GPS timings */
		if(Wakeup_From_Sleep == false && Restart_SMBus == false)
		{
			Check_AP_Request();
		}
	}
}


// Try SMBus deinit functionality before calling Init() function
