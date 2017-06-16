/*
 * AP_Communication.c
 *
 *  Created on: 02-May-2017
 *      Author: NIKHIL
 */
#include "AP_Communication.h"

uint8_t Send_Data[4] = {0x01,0x02,0x03,0x04};

uint8_t AP_Request_Data[MAX_AP_DATA_SIZE];
float Float_Data[6];

static void BMS_Enable_Listen_Mode()
{
	SMBUS_Enable_Listen_Mode(BMS_SMBUS);
}

void AP_COM_Init(uint8_t Communication_Mode)
{
	switch (Communication_Mode)
	{
		case AP_COM_USART_MODE:
			break;

		case AP_COM_SPI_MODE:
			break;

		case AP_COM_SMBUS_MODE:
			I2C_Init(BMS_SMBUS,BMS_SMBUS_OWN_ADDRESS,I2C_100KHZ,I2C_SLAVE);
			BMS_Enable_Listen_Mode();
			break;

		default:
			break;
	}
}
/* @brief	: Function to check the request from AP. AP queries the information like all the cell voltages,pack voltage, pack current, individual cell
 * 			  voltages. BMS replies for all the responses in terms of 4 bytes except for GPS data and all cell voltages data
 * @params 	: None
 * @return 	: None
 * */
void Check_AP_Request()
{
	uint8_t Send_Byte_Count = 0;
	uint8_t Index = 0;

	if(SMBUS_Request_Check(AP_Request_Data) == true)
	{
		switch (AP_Request_Data[0])
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
				/* Data received by BMS is GPS data and its format is like DateMonthYear,HoursMinutesSeconds e.g. 140617,172529 means
				 * Date is 14/06/2017 and time is 17:25:29 etc */
				RTC_Info.Day = -1;
				RTC_Info.Date = ((AP_Request_Data[Index++] - '0') << 4) ;
				RTC_Info.Date |= (AP_Request_Data[Index++] - '0');

				RTC_Info.Month = ((AP_Request_Data[Index++] - '0') << 4);
				RTC_Info.Month |= (AP_Request_Data[Index++] - '0');

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
				RTC_Set_Date(&RTC_Info.Day,&RTC_Info.Date,&RTC_Info.Month,&RTC_Info.Year);
				RTC_Set_Time(&RTC_Info.Hours,&RTC_Info.Minutes,&RTC_Info.Seconds);

				break;
			default:

				break;
		}
	}

	/* Send the data filled in the buffer in terms of bytes to the AP; The size is automatically calculated in the above switch cases */
	SMBUS_Serve_Request((uint8_t*)Float_Data,Send_Byte_Count);
}


