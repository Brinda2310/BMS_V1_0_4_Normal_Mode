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
bool Data_Received = false;
uint8_t AP_Status;

uint8_t AP_Request_Data[MAX_AP_DATA_SIZE];
float Float_Data[6]={0};

void BMS_Enable_Listen_Mode()
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
	Send_Byte_Count = 0;
	static bool Read_Request = true;
	static uint8_t State = 255;

	/* Check if any data is received from AP */
	Result = SMBUS_Request_Check(AP_Request_Data);

	if( Result == SMBUS_REQ_SUCCESSFUL)
	{
		if(Byte_Count == 1)
		{
			State = AP_Request_Data[0];
			Read_Request = true;
		}
//		BMS_Debug_COM_Write_Data(&AP_Request_Data[0],Byte_Count);

		switch (State)
		{
			case CELL1_VOLTAGE_REG:
				Float_Data[0] = Get_Cell1_Voltage();
				break;
			case CELL2_VOLTAGE_REG:
				Float_Data[0] = Get_Cell2_Voltage();
				break;
			case CELL3_VOLTAGE_REG:
				Float_Data[0] = Get_Cell3_Voltage();
				break;
			case CELL4_VOLTAGE_REG:
				Float_Data[0] = Get_Cell6_Voltage();
				break;
			case CELL5_VOLTAGE_REG:
				Float_Data[0] = Get_Cell7_Voltage();
				break;
			case CELL6_VOLTAGE_REG:
				Float_Data[0] = Get_Cell8_Voltage();
				break;
			case PACK_CURRENT_REG:
				Float_Data[0] = Get_BMS_Pack_Current();
				break;
			case PACK_VOLTAGE_REG:
				Float_Data[0] = Get_BMS_Pack_Voltage();
				break;
			case GPS_PACKET_REG:
				if(Read_Request == true)
				{
					Byte_Count = 17;
					Read_Request = false;
				}
				else
				{
//					BMS_Debug_COM_Write_Data(&AP_Request_Data[0],17);
					Byte_Count = 1;
				}
				Set_Bytes_Count(&Byte_Count);
				break;
			case FLIGHT_STATUS_REG:
				if(Read_Request == true)
				{
					Byte_Count = 2;
					Read_Request = false;
				}
				else
				{
//					BMS_Debug_COM_Write_Data(&AP_Request_Data[0],2);
					Byte_Count = 1;
				}
				Set_Bytes_Count(&Byte_Count);
				break;

			default:
				break;
		}
		if(Read_Request == true)
		{
			/* Delay is required to set up the receiving side(AP) */
			Delay_Millis(4);
			SMBUS_Serve_Request((uint8_t*)Float_Data, 4);
		}
		else if (Read_Request == false && Byte_Count != 1)
		{
			Delay_Millis(4);
			SMBUS_Serve_Request((uint8_t*)&Reply_Byte, 1);
		}
	}
//	else if(Result == SMBUS_REQ_TIMEOUT)
//	{
//		if(AP_COM_Init(AP_COM_SMBUS_MODE) == RESULT_OK)
//		{
//			/* This count decides the sleep mode function to be enabled or not. It may be possible that
//			 * AP has sent the DISARM_GROUND status but later SMBUS stopped working,Just for safety purpose
//			 * disable sleep mode function */
//			SMBUS_Reboot_Count++;
//		}
//	}
	/* Disable sleep mode function of BMS in case SMBUS has restarted 15 times(45 seconds). 45 seconds
	 * time is because that if load is not present for continuous 1 minute then MCU will go to sleep
	 * failing to execute the remaining logic */
	if(SMBUS_Reboot_Count >= MAX_SMBUS_REBOOT_COUNT && Sleep_Mode_Funtionality == ENABLE)
	{
		Sleep_Mode_Funtionality = DISABLE;
		/* Debug code to be removed */
		BMS_Debug_COM_Write_Data("Sleep disable\r",14);
		SMBUS_Reboot_Count = 0;
	}

}


