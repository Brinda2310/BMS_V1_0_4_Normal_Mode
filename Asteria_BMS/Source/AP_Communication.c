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
bool Restart_SMBus = false;

uint8_t AP_Request_Data[MAX_AP_DATA_SIZE];
float Float_Data[6]={0};

void BMS_Enable_Listen_Mode()
{
	SMBUS_Enable_Listen_Mode(BMS_SMBUS);
}

void BMS_Disable_Listen_Mode()
{
	SMBUS_Disable_Listen_Mode(BMS_SMBUS);
}

uint8_t AP_COM_Init(uint8_t Communication_Mode)
{
	uint8_t Result = 255;
	Result = I2C_Init(BMS_SMBUS, BMS_SMBUS_OWN_ADDRESS, I2C_100KHZ, I2C_SLAVE);
	BMS_Enable_Listen_Mode();

	return Result;
}
uint8_t AP_COM_DeInit()
{
	uint8_t Result = 255;
	Result = I2C_DeInit(BMS_SMBUS);
	BMS_Disable_Listen_Mode();
	return Result;
}
/* @brief	: Function to check the request from AP. AP queries the information like all the cell voltages,pack voltage, pack current, individual cell
 * 			  voltages. BMS replies for all the responses in terms of 4 bytes except for GPS data and all cell voltages data
 * @params 	: None
 * @return 	: None
 * */
void Check_AP_Request()
{
	uint8_t Result = 255;
	Send_Byte_Count = 0;
	static bool Read_Request = true;
	static uint8_t State = 255;

	/* Check if any data is received from AP */
	Result = SMBUS_Request_Check(AP_Request_Data);

	if( Result == SMBUS_REQ_SUCCESSFUL)
	{
		/* Whenever any request comes from AP, first byte will be always the command code. So state machine
		 * starts with command code. If there are more number of bytes coming from AP, then state should
		 * be maintained to it's previous command code e.g. 0x0A is command code for GPS_DATE_TIME. AP will
		 * send the date and time following the command code. So state should remained to 0x0A to parse
		 * the data */
		if(Byte_Count == MINIMUM_PACKET_SIZE)
		{
			State = AP_Request_Data[0];
			Read_Request = true;
		}

		switch (State)
		{
			case CELL1_VOLTAGE_REG:
				Float_Data[0] = Get_Cell1_Voltage();
				Set_Bytes_Count(&Byte_Count);
				break;
			case CELL2_VOLTAGE_REG:
				Float_Data[0] = Get_Cell2_Voltage();
				Set_Bytes_Count(&Byte_Count);
				break;
			case CELL3_VOLTAGE_REG:
				Float_Data[0] = Get_Cell3_Voltage();
				Set_Bytes_Count(&Byte_Count);
				break;
			case CELL4_VOLTAGE_REG:
				Float_Data[0] = Get_Cell6_Voltage();
				Set_Bytes_Count(&Byte_Count);
				break;
			case CELL5_VOLTAGE_REG:
				Float_Data[0] = Get_Cell7_Voltage();
				Set_Bytes_Count(&Byte_Count);
				break;
			case CELL6_VOLTAGE_REG:
				Float_Data[0] = Get_Cell8_Voltage();
				Set_Bytes_Count(&Byte_Count);
				break;
			case PACK_CURRENT_REG:
				Float_Data[0] = Get_BMS_Pack_Current();
				Set_Bytes_Count(&Byte_Count);
				break;
			case PACK_VOLTAGE_REG:
				Float_Data[0] = Get_BMS_Pack_Voltage();
				Set_Bytes_Count(&Byte_Count);
				break;
			case GPS_PACKET_REG:
				if(Read_Request == true)
				{
					/* Number of bytes to be received from AP GPS packet command code */
					Byte_Count = GPS_DATE_TIME_DATA_SIZE;
					Read_Request = false;
				}
				else
				{
					/* Set the receive byte size to 1 as command code is of one byte only */
					Byte_Count = MINIMUM_PACKET_SIZE;
				}
				Set_Bytes_Count(&Byte_Count);
				break;
			case FLIGHT_STATUS_REG:
				if(Read_Request == true)
				{
					/* Number of bytes to be received from AP FLIGHT_STATUS packet command code */
					Byte_Count = FLIGHT_STATUS_DATA_SIZE;
					Read_Request = false;
				}
				else
				{
#if DEBUG_OPTIONAL == ENABLE
				BMS_Debug_COM_Write_Data(&AP_Request_Data[0],2);
#endif
					AP_Status= AP_Request_Data[0];
					Byte_Count = MINIMUM_PACKET_SIZE;
				}
				break;

			default:
				break;
		}
//		Float_Data[0] = 4.1145;
		Restart_SMBus = false;
		SMBUS_Reboot_Count = 0;
	}
	else if(Result == SMBUS_REQ_TIMEOUT && SMBUS_Reboot_Count <= 5)
	{
		SMBUS_Reboot_Count++;
		AP_COM_DeInit();
		if(AP_COM_Init(AP_COM_SMBUS_MODE) == RESULT_OK)
		{
			Restart_SMBus = true;
			/* Keep the track of how many times communication is reseted. It will be used to disable the
			 * sleep mode function as AP is not able to communicate with BMS */
			BMS_Debug_COM_Write_Data("I2C reboot\r",11);
		}
	}
	/* Number of response bytes to be sent to AP. Most of the replies are float so value isset to 4 */
	if(BMS_Send_Data == true)
	{
		if(Byte_Count == 1)
		{
			SMBUS_Serve_Request((uint8_t*)Float_Data, 4);
		}
		else if (Byte_Count != 1)
		{
			SMBUS_Serve_Request((uint8_t*)&Reply_Byte, 1);
		}
		BMS_Send_Data = false;
	}
	/* If AP wants to write some data to BMS, then BMS reply with 0xFF byte to AP to indicate that
	 * it can send the required data(It is write request from AP) */

//	if (AP_Status == DISARMED_GROUND)
//	{
//		/* Enable the sleep mode functionality only if aircraft is on ground and in disarmed state */
//		Sleep_Mode_Funtionality = ENABLE;
//	}
//	else
//	{
//		Sleep_Mode_Funtionality = DISABLE;
//	}
//	/* Disable sleep mode function of BMS in case SMBUS has restarted 15 times(45 seconds). 45 seconds
//	 * time is because that if load is not present for continuous 1 minute then MCU will go to sleep
//	 * failing to execute the remaining logic. If previous request from AP was DISARM and GROUND and for
//	 * some reason AP is not able to communicate with BMS then BMS should not go to sleep mode */
//	if(SMBUS_Reboot_Count >= MAX_SMBUS_REBOOT_COUNT && Sleep_Mode_Funtionality == ENABLE)
//	{
//		Sleep_Mode_Funtionality = DISABLE;
//		/* Debug code to be removed */
//		BMS_Debug_COM_Write_Data("Sleep disable\r",14);
//		SMBUS_Reboot_Count = 0;
//	}
	/* There is possibility that sleep mode functionality is enabled due to previous request from AP
	 * for DISARM and GROUND, So it is necessary to reset the status to zero */
	AP_Status = 0;

}


