/*
 * AP_Communication.c
 *
 *  Created on: 02-May-2017
 *      Author: NIKHIL
 */
#include "AP_Communication.h"

uint8_t Send_Data[4] = {0x01,0x02,0x03,0x04};
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

void Check_AP_Request(uint8_t *RxBuffer)
{
	bool All_Cells_Data = false;

	if(SMBUS_Request_Check(RxBuffer) == true)
	{
		switch (RxBuffer[0])
		{
			case ALL_CELL_VOLTAGES_REG:
				Float_Data[0] = Get_Cell1_Voltage();
				Float_Data[1] = Get_Cell2_Voltage();
				Float_Data[2] = Get_Cell3_Voltage();
				Float_Data[3] = Get_Cell6_Voltage();
				Float_Data[4] = Get_Cell7_Voltage();
				Float_Data[5] = Get_Cell8_Voltage();
				All_Cells_Data = true;
				break;
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

			case PACK_VOLTAGE_REG:
				Float_Data[0] = Get_BMS_Pack_Voltage();
				break;
			case PACK_CURRENT_REG:
				Float_Data[0] = Get_BMS_Pack_Current();
				break;
			default:
				break;
		}
//		if(All_Cells_Data == true)
//		{
//			SMBUS_Serve_Request((uint8_t*)&Float_Data[0],sizeof(Float_Data));
//		}
//		else
//		{
//			SMBUS_Serve_Request((uint8_t*)&Float_Data[0],4);
//		}
	}

	SMBUS_Serve_Request((uint8_t*)&Float_Data[0],4);
}


