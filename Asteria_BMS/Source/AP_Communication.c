/*
 * AP_Communication.c
 *
 *  Created on: 02-May-2017
 *      Author: NIKHIL
 */
#include <AP_Communication.h>

bool Sleep_Mode_Funtionality = DISABLE;

uint8_t Send_Data[4] = {0x01,0x02,0x03,0x04};
uint8_t Send_Byte_Count = 0;
uint8_t Byte_Count = 1;
uint8_t Reply_Byte = 0xFF;
uint8_t SMBUS_Reboot_Count = 0;

uint8_t AP_Status;

uint8_t AP_Request_Data[MAX_AP_DATA_SIZE];
float Float_Data[6];

static void BMS_Enable_Listen_Mode()
{
	SMBUS_Enable_Listen_Mode(BMS_SMBUS);
}

uint8_t AP_COM_Init(uint8_t Communication_Mode)
{
	uint8_t Result = 255;
	switch (Communication_Mode)
	{
		case AP_COM_USART_MODE:
			break;

		case AP_COM_SPI_MODE:
			break;

		case AP_COM_SMBUS_MODE:
			Result = I2C_Init(BMS_SMBUS,BMS_SMBUS_OWN_ADDRESS,I2C_100KHZ,I2C_SLAVE);
			BMS_Enable_Listen_Mode();

			break;

		default:
			break;
	}
	return Result;
}
/* @brief	: Function to check the request from AP. AP queries the information like all the cell voltages,pack voltage, pack current, individual cell
 * 			  voltages. BMS replies for all the responses in terms of 4 bytes except for GPS data and all cell voltages data
 * @params 	: None
 * @return 	: None
 * */
void Check_AP_Request()
{
	uint8_t Index = 0,Result = 255;
	static uint8_t State = 126;
	static bool Data_Received = false;
	Send_Byte_Count = 0;

	/* Check if any data is received from AP */
	Result = SMBUS_Request_Check(AP_Request_Data);

	if( Result == SMBUS_REQ_SUCCESSFUL)
	{
		if(Byte_Count == MINIMUM_PACKET_SIZE)
		{
			State = AP_Request_Data[0];
		}
		else if (Byte_Count >= FLIGHT_STATUS_DATA_SIZE)
		{
			Data_Received = true;
		}
		switch (State)
		{
			case ALL_CELL_VOLTAGES_REG:
				/* If request is for sending all the cell voltage then fill the data buffer by reading the individual cell voltages */
				Float_Data[Send_Byte_Count++] = Get_Cell1_Voltage();
				Float_Data[Send_Byte_Count++] = Get_Cell2_Voltage();
				Float_Data[Send_Byte_Count++] = Get_Cell3_Voltage();
				Float_Data[Send_Byte_Count++] = Get_Cell6_Voltage();
				Float_Data[Send_Byte_Count++] = Get_Cell7_Voltage();
				Float_Data[Send_Byte_Count++] = Get_Cell8_Voltage();
				break;
			case CELL1_VOLTAGE_REG:
				Float_Data[Send_Byte_Count++] = Get_Cell1_Voltage();
				break;
			case CELL2_VOLTAGE_REG:
				Float_Data[Send_Byte_Count++] = Get_Cell2_Voltage();
				break;
			case CELL3_VOLTAGE_REG:
				Float_Data[Send_Byte_Count++] = Get_Cell3_Voltage();
				break;
			case CELL4_VOLTAGE_REG:
				Float_Data[Send_Byte_Count++] = Get_Cell6_Voltage();
				break;
			case CELL5_VOLTAGE_REG:
				Float_Data[Send_Byte_Count++] = Get_Cell7_Voltage();
				break;
			case CELL6_VOLTAGE_REG:
				Float_Data[Send_Byte_Count++] = Get_Cell8_Voltage();
				break;
			case PACK_CURRENT_REG:
				Float_Data[Send_Byte_Count++] = Get_BMS_Pack_Current();
				break;
			case PACK_VOLTAGE_REG:
				Float_Data[Send_Byte_Count++] = Get_BMS_Pack_Voltage();
				break;
			case GPS_PACKET_REG:
				/* Data received by BMS is GPS data and its format is like DayDateMonthYear,HoursMinutesSeconds e.g. 0414062017,172529 means
				 * Date is Thursday(04) 14/06/2017 and time is 17:25:29 etc */
				Byte_Count = GPS_DATE_TIME_DATA_SIZE;

				if(Data_Received == true)
				{
					RTC_Info.Day = ((AP_Request_Data[Index++] - '0') << 4);
					RTC_Info.Day |= (AP_Request_Data[Index++] - '0');

					RTC_Info.Date = ((AP_Request_Data[Index++] - '0') << 4);
					RTC_Info.Date |= (AP_Request_Data[Index++] - '0');

					RTC_Info.Month = ((AP_Request_Data[Index++] - '0') << 4);
					RTC_Info.Month |= (AP_Request_Data[Index++] - '0');

					Index++;
					Index++;
//					RTC_Info.Year = ((AP_Request_Data[Index++] - '0') << 12);
//					RTC_Info.Year |= ((AP_Request_Data[Index++] - '0') << 8);
					RTC_Info.Year = ((AP_Request_Data[Index++] - '0') << 4);
					RTC_Info.Year |= (AP_Request_Data[Index++] - '0');

					Index++;

					RTC_Info.Hours = ((AP_Request_Data[Index++] - '0') << 4);
					RTC_Info.Hours |= (AP_Request_Data[Index++] - '0');

					RTC_Info.Minutes = ((AP_Request_Data[Index++] - '0') << 4);
					RTC_Info.Minutes |= (AP_Request_Data[Index++] - '0');

					RTC_Info.Seconds = ((AP_Request_Data[Index++] - '0') << 4);
					RTC_Info.Seconds |= (AP_Request_Data[Index++] - '0');

					/* Set the date and time received from AP into the BMS RTC */
					RTC_Set_Date(&RTC_Info.Day, &RTC_Info.Date, &RTC_Info.Month,&RTC_Info.Year);
					RTC_Set_Time(&RTC_Info.Hours, &RTC_Info.Minutes,&RTC_Info.Seconds);

					/* This is necessary to set the packet size back to 1 because AP may query the
					 * other BMS data. So byte_count 1 is set in the lower layer driver of SMBUS */
					Byte_Count = MINIMUM_PACKET_SIZE;
				}
				break;
			case FLIGHT_STATUS_REG:
				/* Set the number of bytes to be received from AP; Bytes count needs to be changed as
				 * per the request received from AP and same should be set in SMBus driver receive
				 * interrupt routine */
				Byte_Count = FLIGHT_STATUS_DATA_SIZE;
				/* This will true only when AP wants to write some data. State machine will be in
				 * same state as previous state and it is retained to receive the bytes from AP */
				if(Data_Received == true)
				{
					AP_Status = AP_Request_Data[0];
					/* Sleep mode functionality should be enabled only when plane is in DISARMED state and
					 * it is on ground otherwise disable the sleep mode function */
					if(AP_Status == DISARMED_GROUND)
					{
						Sleep_Mode_Funtionality = ENABLE;
					}
					else
					{
						Sleep_Mode_Funtionality = DISABLE;
					}
					Byte_Count = MINIMUM_PACKET_SIZE;
				}
				break;

			default:
				break;
		}
		/* This function will set the number of bytes that AP is going to write to BMS. Based on packet
		 * index value,count will be set in the SMBus receive interrupt routine(low level driver) */
		Set_Bytes_Count(&Byte_Count);

		/* Send the data filled in the buffer in terms of bytes to the AP; The size is automatically calculated in the above switch cases */
		if(Byte_Count == MINIMUM_PACKET_SIZE && Data_Received == false)
		{
			/* Delay is required to set up the receiving side(AP) */
			Delay_Millis(3);
			SMBUS_Serve_Request((uint8_t*)&Float_Data[0],(Send_Byte_Count*4));
		}
		else
		{
			Delay_Millis(3);
			Reply_Byte = 0xFF;
			SMBUS_Serve_Request((uint8_t*)&Reply_Byte,1);
		}
		if (Data_Received == true)
		{
			/* Debug code to be removed after testing */
			if(AP_Status == DISARMED_GROUND)
			{
				BMS_Debug_COM_Write_Data("DISARMED_GROUND\r",16);
			}
			else if (AP_Status == DISARMED)
			{
				BMS_Debug_COM_Write_Data("DISARMED\r",9);
			}
			else if (AP_Status == ARMED)
			{
				BMS_Debug_COM_Write_Data("ARMED\r",6);
			}
			Data_Received = false;
		}
		SMBUS_Reboot_Count = 0;
		/* Clear the buffer in which AP data is being received */
		memset(AP_Request_Data,0,sizeof(AP_Request_Data));
		AP_Status = 0;
	}
	/* If no any data is received from AP in every 3 seconds then restart the SMBUS. BMS expects
	 * data to be received from AP every 3 seconds(heart beat) */
	else if(Result == SMBUS_REQ_TIMEOUT)
	{
		if(AP_COM_Init(AP_COM_SMBUS_MODE) == RESULT_OK)
		{
			/* This count decides the sleep mode function to be enabled or not. It may be possible that
			 * AP has sent the DISARM_GROUND status but later SMBUS stopped working,Just for safety purpose
			 * disable sleep mode function */
//			char buffer[5],length = 0;
			SMBUS_Reboot_Count++;
			/* Debug code to be removed */
//			length = sprintf(buffer,"%d",SMBUS_Reboot_Count);
//			BMS_Debug_COM_Write_Data(buffer,length);
		}
	}

	/* Disable sleep mode function of BMS in case SMBUS has restarted 15 times(45 seconds). 45 seconds
	 * time is because that if load is not present for continuous 1 minute then MCU will go to sleep
	 * failing to execute the remaining logic */
	if(SMBUS_Reboot_Count >= MAX_SMBUS_REBOOT_COUNT && Sleep_Mode_Funtionality == ENABLE)
	{
//		Sleep_Mode_Funtionality = DISABLE;
		/* Debug code to be removed */
		BMS_Debug_COM_Write_Data("Sleep disable\r",14);
		SMBUS_Reboot_Count = 0;
	}

}


