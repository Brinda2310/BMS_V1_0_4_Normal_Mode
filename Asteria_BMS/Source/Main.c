/**
 ******************************************************************************
 * @file    Main.c
 * @author  Nikhil Ingale
 * @version V1.0.2
 * @date    25-Nov-2017
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
#include <BMS_Watchdog.h>

/* Structure for GPIO */
GPIO_InitTypeDef    GPIO_InitStruct;

/* this variable increment every 1 ms time interval. Here this is use for BMS configuration time counting */
extern __IO uint32_t uwTick;

void Gpio_Confi(void);

#define TEST_DEBUG_GPS_INFO									/* Character A*/
#define TEST_DEBUG_START_TIME								/* Character B*/
#define TEST_DEBUG_ALL_PACK_DATA							/* Character C*/
#define TEST_DEBUG_PACK_CURRENT_ADJ_CD_RATE					/* Character D*/
#define TEST_DEBUG_CAPACITY_USED_REMAINING_TOTAL			/* Character E*/
#define TEST_DEBUG_C_D_TOTAL_PACK_CYLES						/* Character F*/
#define TEST_DEBUG_HEALTH_I2C_ERROR							/* Character G*/
#define TEST_DEBUG_TEMPERATURE								/* Character H*/
#define TEST_DEBUG_WATCHDOG_TEST							/* Character I*/
#define TEST_DEBUG_CODE_RESET								/* Character J*/
//#define TEST_CHARGE_DISCHARGE_SOFTWARE					/* Character K*/
#define TEST_DEBUG_LOG_FILE_INFO							/* Character N*/
#define TEST_DEBUG_STOP_LOG									/* Character O*/

#define TEST_DEBUG_WATCHDOG_RESET_TIME						2100

#define _2_SECONDS_TIME										50		/* Time for which SOC to be shown (50 * 40ms) */
#define _1_SECONDS_TIME										(_2_SECONDS_TIME/2)

const uint8_t BMS_Firmware_Version[3] =
{
		1,			// Major release version--modified when code is being merged to Master branch.
		0,			// Current stable code release-- modified when code is being merged to Develop branch.
		4			// Beta code release--modified when code is being merged to test_develop branch.
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

/* Debug code variables definition; Allocate the buffer size only if debugging is to be done */
char Buffer[400];
uint8_t RecData = 0;

/* This variable counts the time for which log was unsuccessful; If it more than 125ms then SD card is
 * reinitialized */
uint8_t Log_Init_Counter = 0 ;

/* These flags are used to indicate the external button has been pressed for more than specified time
 * so that SOC or SOH status can be shown on LEDs */
bool SOC_Flag = false,SOH_Flag = false,Debug_Mode_Function = false;
bool Display_SOH = false,Display_SOC = false;

/* Variable to hold the timing value to force the BMS IC to sleep mode; Values are set using macros defined
 * in BMS_Timing.h file. If MCU is awaken from sleep mode then check load presence for 10 seconds
 * otherwise check load presence for 1 minute(normal operation) */
uint16_t Timer_Value = 0,Time_Count = 0;

/* This flag is used for re-initialization of SD card in case of it's non presence in the slot */
bool SD_Card_ReInit = false;

/* This variable monitors the logging is happening on SD card or not. If it is true then there is no any problem for logging otherwise
 * there is some problem in the logging which will be displayed over USART */
bool Log_Status = false,Log_Stopped = false;

/* These flags are used to test the software logic for charging and discharging pack cycles. These flags will be true only if user sends 'K'
 * or 'L' character over USART. Once it is sent, charge/discharge timers will start counting till the max time, after which it will considered as cycle */
#ifdef TEST_CHARGE_DISCHARGE_SOFTWARE
	bool Start_Charging = false,Start_Discharging = false;
#endif

/* This flag becomes true when all the configuration parameters are written to the EEPROM of ISL ASIC */
bool BMS_Configuration_OK = false;

uint8_t Critical_Batt_V_Counter = 0;

int main(void)
{
	/* loop rate counter value store buffer*/
	uint8_t loop_buff[2] = {0};
	uint8_t time_buff[16] = {0};

	uint32_t l_old_time = 0, l_final_time = 0;

	/* Configure the sysTick interrupt to 1mS(default) and Set the NVIC group priority to 4 */
	HAL_Init();

	/* Configure the system clock frequency (Peripherals clock) to 20MHz */
	Set_System_Clock_Frequency();

	/* Delay of 1000 milliSeconds is required to make sure BMS is not polled before it's POR cycle otherwise
	 * BMS I2C will be locked */
	Delay_Millis(1000);

	Gpio_Confi();

	/* Initialize the timer to 40mS(25Hz) and the same is used to achieve different loop rates */
	BMS_Timers_Init();

	/* Initialize the Status and Error LEDs connected on the BMS board */
//	BMS_Status_Error_LED_Init();

	/* Initialize the status LEDs which indicates the SOC and SOH */
	if(Debug_COM_Enable == false)
	{
		BMS_SOH_SOC_LEDs_Init();
	}
	{
		/* Initialize the USART to 115200 baud rate to debug the code */
		BMS_Debug_COM_Init();
	}

	/* Configure the switch as input to wake up the BMS in case of sleep and same will be used
	 * to show the SOC and SOH on status LEDs*/
	BMS_Switch_Init();

	/* Configure the ISL94203 I2C communication to 100KHz */
	BMS_ASIC_Init();

	/* Initialize the communication between AP and BMS; Current version of BMS supports SMBUS protocol */
	AP_COM_Init(AP_COM_SMBUS_MODE);

	/* Initialize the RTC and set the RTC time and date to the date and time received from GPS */
	RTC_Init();

	/* Get the source for the previous reset of MCU */
	if(Get_Reset_Source() == SOFTWARE)
	{
		BMS_Debug_COM_Write_Data("It is Software Reset...!!!\r",27);
	}

	if (Get_Reset_Source() == WATCHDOG)
	{
		BMS_Debug_COM_Write_Data("It is Watchdog Reset...!!!\r",27);
	}

	l_old_time = uwTick;

	/* Sets the parameters in the ISL94203 to raise the flag and log the same in SD card */
	if(BMS_Configure_Parameters() != RESULT_OK)
	{
		BMS_Configuration_OK = false;
	}
	else
	{
		BMS_Configuration_OK = true;
	}

	l_final_time = uwTick - l_old_time;

	sprintf(time_buff,"total time = %d\n",l_final_time);

	BMS_Debug_COM_Write_Data(time_buff,16);

	/* Set the current gain in the BMS ASIC register. After having number of iterations and analyzing
	 * the curves we will decide which gain is suitable for which current range(Amperes) */
	BMS_Set_Current_Gain(CURRENT_GAIN_5X);

	/* Read the pack voltage to calculate the battery capacity used/remaining */
	BMS_Read_Pack_Voltage();

	/* Create the LOG file on SD card by reading the count from log summary file */
	BMS_Log_Init();

	/* Calculate the battery capacity used and remaining so that same value will be used to estimate
	 * next values */
	BMS_Estimate_Initial_Capacity();

	/* Every time when MCU restarts, set the load check timer value to 1 minute; If MCU is awaken from sleep
	 * mode then set this value to 10 seconds; If load is not present for 10 seconds then force BMS to
	 * sleep mode again */
	Timer_Value = LOW_CONSUMPTION_DELAY;

	/* Initialize the watchdog timer to 2 seconds i.e. if system hangs for some reason then it will
	 * automatically restart the code */
//	BMS_watchdog_Init();

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
			BMS_Log_Init();

			/* Calculate the battery capacity used and remaining so that same value will be used to estimate
			 * next values */
			BMS_Estimate_Initial_Capacity();

			/* Initialize the watchdog timer to 2 seconds i.e. if system hangs for some reason then it will
			 * automatically restart the code */
			BMS_watchdog_Init();

			/* This flag must be cleared to avoid reinitializing all the peripherals again and again */
			Wakeup_From_Sleep = false;
		}

		/* Debug code to be removed after testing */
		BMS_Debug_COM_Read_Data(&RecData,1);

		/* This flag will be true after every 40ms(25Hz) in timer application file */
		if (_25Hz_Flag == true)
		{
			/* LED 5 (PB5) Toggle every 40ms */
			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_5);

			/* If there is any problem in configuring the parameters in the ISL then we will try it again and again. Once it is done then
			 * set BMS_Configuration_OK flag to true */
			if(BMS_Configuration_OK == false)
			{
				if(BMS_Configure_Parameters() == RESULT_OK)
				{
					BMS_Configuration_OK = true;
				}
			}

			/* Continuously monitor the cells configuration from the ISL. If ISL has gone into factory reset settings then it will give number
			 * of cells equal to 3. If number of cells are equal to the value which is configured earlier then write the default parameters again
			 * into the ISL EEPROM */
			if(BMS_Read_Number_Of_Cells_Configuration() != RESULT_OK)
			{
				if(BMS_Configure_Parameters() == RESULT_OK)
				{
					BMS_Configuration_OK = true;
				}
				else
				{
					BMS_Configuration_OK = false;
				}
			}
			else
			{
				BMS_Configuration_OK = true;
			}

			/* Monitor the SD card's existence in the slot */
			SD_Status();

			/* If switch is pressed then start counting the time (40ms is the tick peroid) */
			if (BMS_Read_Switch_Status() == PRESSED)
			{
				Switch_Press_Time_Count++;
				/* If switch press count is more than 500ms and less than 2 seconds then make SOC_Flag true to display
				 * the SOC status on LEDs as soon as switch is released */
				if (Switch_Press_Time_Count >= SHORT_PERIOD && Switch_Press_Time_Count <= LONG_PEROID)
				{
					SOC_Flag = true;
				}
				/* If switch press count is more than 2 seconds then make SOH_Flag variable true to display the
				 * SOH status on LEDs as soos as switch is released */
				if (Switch_Press_Time_Count >= LONG_PEROID)
				{
					SOH_Flag = true;
					SOC_Flag = false;
				}

				/* If switch is pressed for more than 5 seconds then debug functionality will be toggled with SOH_SOC
				 * functionality. It will start displaying the data which is being sent over USART at 1Hz*/
				if (Switch_Press_Time_Count >= DEBUG_FUNCTION_ENABLE_PERIOD && Switch_Press_Time_Count <= FACTORY_DEFAULT_PEROID)
				{
					SOH_Flag = false;
					SOC_Flag = false;
					Debug_Mode_Function = true;
				}
				/* If external switch is pressed for more than 10 seconds then code is reseted to factory default setting */
				else if(Switch_Press_Time_Count > FACTORY_DEFAULT_PEROID)
				{
					NVIC_SystemReset();
				}
				else
				{
					Time_Count = 0;
				}
			}
			else
			{
				/* If switch is immediately released then reset time count to zero */
				Switch_Press_Time_Count = 0;

				/* If switch is pressed more than 2 seconds then display only SOH status on LEDs and reset the time
				 * count which is used to keep LEDs ON for specified time */
				if(SOH_Flag == true)
				{
					SOH_Flag = false;
					Display_SOH = true;
					Time_Count = 0;
				}
				/* If switch is pressed more than 500ms and less than 2 seconds then display only SOC status on LEDs
				 * and reset the time count which is used to keep LEDs ON for specified time */
				if(SOC_Flag == true)
				{
					SOC_Flag = false;
					Display_SOC = true;
					Time_Count = 0;
				}
				/* Toggle the Debug USART and SOH_SOC functionality based on switch press for more than 5 seconds */
				if(Debug_Mode_Function == true)
				{
					Display_SOC = false;
					Display_SOH = false;
					Debug_Mode_Function = false;
					if(Debug_COM_Enable == true)
					{
						Debug_COM_Enable = false;
						BMS_SOH_SOC_LEDs_Init();
					}
					else
					{
						Debug_COM_Enable = true;
						BMS_Debug_COM_Init();
					}
				}
			}

			/* If switch is pressed for more than 500ms and less than 2 seconds then show the SOC status on LEDs*/
			if(Display_SOC == true)
			{
				if(Time_Count <= _2_SECONDS_TIME)
				{
					Time_Count++;
					BMS_Show_LED_Pattern(SOC,SHOW_STATUS);
				}
				else
				{
					Display_SOC = false;
					Time_Count = 0;
					BMS_Show_LED_Pattern(SOC,HIDE_STATUS);
				}
			}
			/* If switch is pressed for more than 2 seconds and less than 5 seconds then show the SOH status on LEDs*/
			else if (Display_SOH == true)
			{
				if (Time_Count <= _2_SECONDS_TIME)
				{
					Time_Count++;
					BMS_Show_LED_Pattern(SOH, SHOW_STATUS);
				}
				else
				{
					Display_SOH = false;
					Time_Count = 0;
					BMS_Show_LED_Pattern(SOH, HIDE_STATUS);
				}
			}

			/* Query the BMS data at 25Hz; All cell voltages, pack voltage, pack current, pack temperature
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
				if(((uint16_t)Get_BMS_Pack_Current_Adj() < MINIMUM_CURRENT_CONSUMPTION) && Status_Flag.BMS_In_Sleep == NO)
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
				else if (((uint16_t)Get_BMS_Pack_Current_Adj() > MINIMUM_CURRENT_CONSUMPTION))
				{
				   /* If BMS consumes more than 200mA in between then reset the time count to zero */
					BMS_Sleep_Time_Count = 0;
					/* If load is present then change the timer check value for sleep to 1 minute */
					Timer_Value = LOW_CONSUMPTION_DELAY;
				}

				/* If BMS IC is forced to sleep mode then start counting the timer value; If BMS IC goes to
				 * sleep mode immediately after wake up then MCU will go to sleep after 5 seconds */
				if(Status_Flag.BMS_In_Sleep == YES)
				{
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
#ifdef TEST_CHARGE_DISCHARGE_SOFTWARE
				if(Get_BMS_Charge_Discharge_Status() == CHARGING || Start_Charging == true)
#else
				/* If external charger is connected to the BMS then keep continuous track of it */
				if(Get_BMS_Charge_Discharge_Status() == CHARGING)
#endif
				{
					/* Make the count used for charging to zero to get the exact duration of 5mins while
					 * executing the discharge section of the code */
					Discharge_Time_Count = 0;

#ifdef TEST_CHARGE_DISCHARGE_SOFTWARE
					Charge_Time_Count++;
#else
					/* If current coming into the pack is more than 1A then start counting the time */
					if(Get_BMS_Pack_Current_Adj() > CHARGE_CURRENT_CONSUMPTION && Update_Pack_Cycles == false)
					{
						Charge_Time_Count++;
					}
					else
					{
						/* BMS is charging but with value less than mentioned(1A) current then reset the charge timer
						 * count to zero */
						Charge_Time_Count = 0;
					}
#endif
					/* If current coming into the pack is more than 1A for more than 5mins(CHARGE_TIME_DELAY),then increment
					 * the charge cycles count only if previous pack cycle was discharging and make the last_charge_discharge_status
					 * variable to charging to keep track of last state of the pack i.e. charging/discharging */
					if(Charge_Time_Count >= CHARGE_TIME_DELAY)
					{
						/* At very first when battery is connected to the BMS we do not know the last charge and discharge status
						 * So change the status to charging if current is more than 1A for continuous 5 minutes */
						if(Last_Charge_Disharge_Status == LOW_POWER_CONSUMPTION)
						{
							Last_Charge_Disharge_Status = CHARGING;
						}
						else if(Last_Charge_Disharge_Status == DISCHARGING)
						{
							BMS_Data.Pack_Charge_Cycles++;
							Update_Pack_Cycles = true;
							Last_Charge_Disharge_Status = CHARGING;
							BMS_Update_Pack_Cycles();
						}
#ifdef TEST_CHARGE_DISCHARGE_SOFTWARE
						Charge_Time_Count = 0;
#endif
					}
				}
#ifdef TEST_CHARGE_DISCHARGE_SOFTWARE
				else if (Get_BMS_Charge_Discharge_Status() == DISCHARGING || Start_Discharging == true)
#else
				/* If status of the BMS is discharging then keep continuous track of it */
				else if (Get_BMS_Charge_Discharge_Status() == DISCHARGING)
#endif
					{
					/* Make the count used for charging to zero to get the exact duration of 5mins while
					 * executing the charging section of the code */
					Charge_Time_Count = 0;
#ifdef TEST_CHARGE_DISCHARGE_SOFTWARE
					Discharge_Time_Count++;
#else
					/* If current going out of the pack is more than 1 amperes then start counting the time */
					if(Get_BMS_Pack_Current_Adj() > DISCHARGE_CURRENT_CONSUMPTION && Update_Pack_Cycles == false)
					{
						Discharge_Time_Count++;
					}
					else
					{
						Discharge_Time_Count = 0;
					}
#endif
					/* If discharge current is more than 1A for more than 5 minutes then increment the discharge
					 * cycles count only if previous pack cycle was charging and make the last_charge_discharge_status
					 * variable to discharging to keep track of last state of the pack i.e. charging/discharging */
					if(Discharge_Time_Count >= DISCHARGE_TIME_DELAY)
					{
						/* At very first when battery is connected to the BMS we do not know the last charge and discharge status
						 * So change the status to discharging if current is more than 1A for continuous 5 minutes */
						if(Last_Charge_Disharge_Status == LOW_POWER_CONSUMPTION)
						{
							Last_Charge_Disharge_Status = DISCHARGING;
						}
						else if (Last_Charge_Disharge_Status == CHARGING)
						{
							BMS_Data.Pack_Discharge_Cycles++;
							Update_Pack_Cycles = true;
							Last_Charge_Disharge_Status = DISCHARGING;
							BMS_Update_Pack_Cycles();
						}
#ifdef TEST_CHARGE_DISCHARGE_SOFTWARE
						Discharge_Time_Count = 0;
#endif
					}

				}
				else
				{
					/* If BMS is in low power consumption mode then clear all the timer counts and
					 * do not update any cycles(C/D) count */
					Charge_Time_Count = 0;
					Discharge_Time_Count = 0;
					Update_Pack_Cycles = false;
				}
			}

			/* SD card logging will happen only if SD card is present in the slot; This thing will also avoid
			 * code stuck due to insertion of SD card while running the code */
			if(SdStatus == SD_PRESENT)
			{
				/* If SD card is not removed from the slot then only start immediate logging */
				if(SD_Card_ReInit == false)
				{
					/* Log all the variable in the SD card */
					if(Log_All_Data() != RESULT_OK)
					{
						/* If logging is failed for more than 5 successive counts (125ms) then reinitialize
						 * the SD card functionality */
						Log_Init_Counter++;
						Log_Status = false;
						if (Log_Init_Counter >= (_1_SECONDS_TIME/5))
						{
							Log_Init_Counter = 0;
							BMS_Log_Init();
						}
					}
					else
					{
						Log_Status = true;
					}
				}
				else
				{
					/* As soon as SD card is inserted in the slot, we should initialize the SD card and then start
					 * logging the data. After initializing the SD card, wait for 1000 milliseconds then start
					 * logging to avoid problem in the logging */
					static uint8_t Counter = 0;
					if(Counter++ >= _1_SECONDS_TIME)
					{
						BMS_Log_Init();
						SD_Card_ReInit = false;
						Counter = 0;
					}
				}
			}
			else if(SdStatus == SD_NOT_PRESENT)
			{
				Log_Status = false;
				SD_Card_ReInit = true;
			}

			/* If any time there is problem in querying the data from ISL94203 then restart the
			 * I2C communication with ISL94203 */
			if(BMS_Check_COM_Health() != HEALTH_OK)
			{
				BMS_ASIC_Init();
				/* Variable to log the number of time ISL restarted during its operation */
				ASIC_Restart_Count++;
			}

			/* Variable to log the loop rate */
			Loop_Rate_Counter++;

			/* Reload the watchdog timer value to avoid resetting of code */
			BMS_Watchdog_Refresh();

					if(RecData == 'S')
					{
						BMS_Data.Pack_Voltage = 19.8;
					}

			if(BMS_Check_Critical_Voltage() == BATT_CRITICAL_LEVEL_REACHED)
			{
				if(Critical_Batt_V_Counter++ >= 10)
				{
					BMS_Debug_COM_Write_Data("Sleep Mode\r",11);
					Sleep_Mode_Entered = true;
					Log_All_Data();
					Delay_Millis(10);
					MCU_Enter_Sleep_Mode();
				}
			}
			else
			{
				Critical_Batt_V_Counter = 0;
			}

			_25Hz_Flag = false;
		}
		/* 1Hz loop which displays the information on USART. It is used for debugging purpose only
		 * (inputs are provided as per the test cases) */
		if(_1Hz_Flag == true && Wakeup_From_Sleep == false)
		{
			memset(Buffer,0,sizeof(Buffer));
			uint8_t Length = 0;

			/* This flag true after 20 second */
			if(BMS_Mode_status == true)
			{
				BMS_Mode_status = false;

				/* Set the corresponding flag in BMS IC to force it to Doze mode */
				BMS_Force_Doze();

				if(I2C_Error_Flag.I2C_Force_Doze == 0)
				{
					BMS_Debug_COM_Write_Data("BMS in Doze Mode",16);
				}
			}

			/* Debug code to be removed after testing */
			switch(RecData)
			{
#ifdef TEST_DEBUG_GPS_INFO
				case 'A':
					Length += RTC_TimeShow((uint8_t*)&Buffer[Length],DATE_TIME_COMBINED);
					break;
#endif

#ifdef TEST_DEBUG_START_TIME
				case 'B':
					Length += sprintf(&Buffer[Length],"MCU Time:%d\r",(int)Get_System_Time_Millis());
					break;
#endif

#ifdef TEST_DEBUG_ALL_PACK_DATA
				case 'C':
					Length += sprintf(&Buffer[Length],"C1 = %0.2fV\rC2 = %0.2fV\rC3 = %0.2fV\r",Get_Cell1_Voltage(),Get_Cell2_Voltage(),Get_Cell3_Voltage());
					Length += sprintf(&Buffer[Length],"C4 = %0.2fV\rC5 = %0.2fV\rC6 = %0.2fV\r",Get_Cell6_Voltage(),Get_Cell7_Voltage(),Get_Cell8_Voltage());
					Length += sprintf(&Buffer[Length],"Pack Volt = %0.3fV\r",Get_BMS_Pack_Voltage());
					Length += sprintf(&Buffer[Length],"Pack Curr = %0.3fmA\r",Get_BMS_Pack_Current());
					break;
#endif

#ifdef TEST_DEBUG_PACK_CURRENT_ADJ_CD_RATE
				case 'D':
					Length += sprintf(&Buffer[Length],"Pack_Curr_Adj :%0.3fmA\r",Get_BMS_Pack_Current_Adj());
					Length += sprintf(&Buffer[Length],"C_D_Current :%0.4fmA",C_D_Accumulated_mAH);
					if(Get_BMS_Charge_Discharge_Status() == CHARGING)
					{
						Length += sprintf(&Buffer[Length]," IN\r");
					}
					else
					{
						Length += sprintf(&Buffer[Length]," OUT\r");
					}

					break;
#endif

#ifdef TEST_DEBUG_CAPACITY_USED_REMAINING_TOTAL
				case 'E':
					Length += sprintf(&Buffer[Length],"Total Capacity :%0.2fmA\r",(float)BATTERY_CAPACITY);
					Length += sprintf(&Buffer[Length],"Capacity Used :%0.3fmAH\r",Get_BMS_Capacity_Used());
					Length += sprintf(&Buffer[Length],"Capacity Remaining :%0.3f%c\r",Get_BMS_Capacity_Remaining(),0x25);
					break;
#endif

#ifdef TEST_DEBUG_C_D_TOTAL_PACK_CYLES
				case 'F':
					Length += sprintf(&Buffer[Length],"Charge Cycles :%d\r",(int)BMS_Data.Pack_Charge_Cycles);
					Length += sprintf(&Buffer[Length],"Discharge Cycles :%d\r",(int)BMS_Data.Pack_Discharge_Cycles);
					Length += sprintf(&Buffer[Length],"Total Cycles :%d\r",(int)Get_BMS_Total_Pack_Cycles());
					break;
#endif

#ifdef TEST_DEBUG_HEALTH_I2C_ERROR
				case 'G':
					Length += sprintf(&Buffer[Length],"Health Info :%s\r",BMS_Data.Health_Status_Info);
					Length += sprintf(&Buffer[Length],"I2C Error Info :%s\r",BMS_Data.I2C_Error_Info);
					break;
#endif

#ifdef TEST_DEBUG_TEMPERATURE
				case 'H':
					Length += sprintf(&Buffer[Length],"Pack_Temp :%f degrees\r",BMS_Data.Pack_Temperature_Degrees);
					break;
#endif

#ifdef TEST_DEBUG_WATCHDOG_TEST
				case 'I':
					Delay_Millis(TEST_DEBUG_WATCHDOG_RESET_TIME);
					break;
#endif

#ifdef TEST_DEBUG_CODE_RESET
				case 'J':
					NVIC_SystemReset();
					break;
#endif

#ifdef TEST_CHARGE_DISCHARGE_SOFTWARE
				case 'K':
					Start_Charging = true;
					Start_Discharging = false;
					break;

				case 'L':
					Start_Charging = false;
					Start_Discharging = true;
					break;
				case 'M':
					Start_Charging = false;
					Start_Discharging = false;
					break;
#endif

#ifdef TEST_DEBUG_LOG_FILE_INFO
				case 'N':
					Length += sprintf(&Buffer[Length],"Power Num :%d\r",SD_Summary_Data.Power_Up_Number);
					Length += sprintf(&Buffer[Length],"File Num :%d\r",SD_Summary_Data.Total_Num_of_Files);
					break;
#endif
#ifdef TEST_DEBUG_STOP_LOG
				case 'O':
					Stop_Log();
					Log_Stopped = true;
					break;
				case 'P':
					BMS_Log_Init();
					Log_Stopped = false;
					break;

				case '?':
					AP_COM_Init(AP_COM_SMBUS_MODE);
					break;
#endif
			}

			/* If logging is happening without any problem then display SD Write OK string otherwise display
			 * SD Write Error string over debug port */
			if(Log_Status == true && Log_Stopped == false)
			{
				Length += sprintf(&Buffer[Length],"SD Write OK\r\r");
			}
			else
			{
				if(Log_Stopped == true)
					Length += sprintf(&Buffer[Length],"Write Stopped\r\r");
				else
					Length += sprintf(&Buffer[Length],"SD Write Error\r\r");
			}

			BMS_Debug_COM_Write_Data(Buffer, Length);

			_1Hz_Flag = false;
		}

		/* If MCU receives the proper data from Autopilot then this variable becomes true.
		 * This section of code extracts the date and time information and set it to the RTC */
		if(GPS_Data_Received == true)
		{
			uint8_t Index = 0;
			RTC_Info.Day = ((GPS_Data[Index++] - '0') << 4);
			RTC_Info.Day |= (GPS_Data[Index++] - '0');

			RTC_Info.Date = ((GPS_Data[Index++] - '0') << 4);
			RTC_Info.Date |= (GPS_Data[Index++] - '0');

			RTC_Info.Month = ((GPS_Data[Index++] - '0') << 4);
			RTC_Info.Month |= (GPS_Data[Index++] - '0');

			Index++;
			Index++;

			RTC_Info.Year = ((GPS_Data[Index++] - '0') << 4);
			RTC_Info.Year |= (GPS_Data[Index++] - '0');

			Index++;

			RTC_Info.Hours = ((GPS_Data[Index++] - '0') << 4);
			RTC_Info.Hours |= (GPS_Data[Index++] - '0');

			RTC_Info.Minutes = ((GPS_Data[Index++] - '0') << 4);
			RTC_Info.Minutes |= (GPS_Data[Index++] - '0');

			RTC_Info.Seconds = ((GPS_Data[Index++] - '0') << 4);
			RTC_Info.Seconds |= (GPS_Data[Index++] - '0');

			/* Set the date and time received from AP into the BMS RTC */
			RTC_Set_Date(&RTC_Info.Day, &RTC_Info.Date, &RTC_Info.Month,&RTC_Info.Year);
			RTC_Set_Time(&RTC_Info.Hours, &RTC_Info.Minutes,&RTC_Info.Seconds);

			uint8_t Len = 0;

			/* This section copies the GPS date and time received from AP into the data buffer which is being used to log the date in SD card */
			if(RTC_Info.Year <= 0x18)
			{
				memcpy(&GPS_Date_Time[Len],&GPS_Data[2],2);
				Len += 2;
				GPS_Date_Time[Len++] = '-';
				memcpy(&GPS_Date_Time[Len],&GPS_Data[4],2);
				Len += 2;
				GPS_Date_Time[Len++] = '-';
				memcpy(&GPS_Date_Time[Len],&GPS_Data[6],4);
				Len += 4;
				GPS_Date_Time[Len++] = '-';
				BMS_Debug_COM_Write_Data("Date Set\r",9);
			}

			/* Make this flag to false so as to serve next GPS date and time update request from AP */
			GPS_Data_Received = false;
		}
		/* If AP sends the Flight status then this variable will be true in driver file and the same status
		 * is logged in SD card */
		if(Flight_Stat_Received == true)
		{
			/* Debug info sent over USART to check the status sent by AP */
			BMS_Debug_COM_Write_Data(&AP_Stat_Data.bytes[0],FLIGHT_STATUS_DATA_SIZE);
			/* Make this flag false to get the updated status from AP */
			Flight_Stat_Received = false;
		}

		/* flag true every 1 second time interval (timer_6 event). This flag is use for loop rate counting every 1 second time interval */
		if(flag == true)
		{
			/*flag clear */
			flag = false;

			/* loop rate counter value store into another variable */
			Loop_Rate_Log_Counter = Loop_Rate_Counter;

			/* clear variable */
			Loop_Rate_Counter = 0;

			/* integer value convert into string format */
			sprintf(loop_buff, "%d", Loop_Rate_Log_Counter);

			/* loop count value print on uart */
			BMS_Debug_COM_Write_Data(loop_buff,2);
		}
	}
}

void Gpio_Confi(void)
{
	/* Enable the clock for the GPIOB */
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/* Configure IO in output push-pull mode to drive external LEDs */
	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull  = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

	GPIO_InitStruct.Pin = GPIO_PIN_5;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,RESET);
}

