
/* File Info : -----------------------------------------------------------------
1. How to use this driver:
--------------------------
   - This driver does not need a specific component driver for the micro SD device
     to be included with.

2. Driver description:
---------------------
  + Initialization steps:
     o Initialize the micro SD card using the SD_Init() function.
     o Checking the SD card presence is not managed because SD detection pin is
       not physically mapped on the Adafruit shield.
     o The function SD_GetCardInfo() is used to get the micro SD card information
       which is stored in the structure "SD_CardInfo".

  + Micro SD card operations
     o The micro SD card can be accessed with read/write block(s) operations once
       it is ready for access. The access can be performed in polling
       mode by calling the functions SD_ReadBlocks()/SD_WriteBlocks()

     o The SD erase block(s) is performed using the function SD_Erase() with
       specifying the number of blocks to erase.
     o The SD runtime status is returned when calling the function SD_GetStatus().

------------------------------------------------------------------------------*/

/* Includes ------------------------------------------------------------------*/
#include "SD_SPI.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
  uint8_t r1;
  uint8_t r2;
  uint8_t r3;
  uint8_t r4;
  uint8_t r5;
} SD_CmdAnswer_typedef;

#define SD_DUMMY_BYTE            0xFF

#define SD_MAX_FRAME_LENGTH        17    /* Lenght = 16 + 1 */
#define SD_CMD_LENGTH               6

#define SD_MAX_TRY                100    /* Number of try */

#define SD_CSD_STRUCT_V1          0x2    /* CSD struct version V1 */
#define SD_CSD_STRUCT_V2          0x1    /* CSD struct version V2 */


typedef enum {
 SD_ANSWER_R1_EXPECTED,
 SD_ANSWER_R1B_EXPECTED,
 SD_ANSWER_R2_EXPECTED,
 SD_ANSWER_R3_EXPECTED,
 SD_ANSWER_R4R5_EXPECTED,
 SD_ANSWER_R7_EXPECTED,
}SD_Answer_type;

#define SD_TOKEN_START_DATA_SINGLE_BLOCK_READ    0xFE  /* Data token start byte, Start Single Block Read */
#define SD_TOKEN_START_DATA_MULTIPLE_BLOCK_READ  0xFE  /* Data token start byte, Start Multiple Block Read */
#define SD_TOKEN_START_DATA_SINGLE_BLOCK_WRITE   0xFE  /* Data token start byte, Start Single Block Write */
#define SD_TOKEN_START_DATA_MULTIPLE_BLOCK_WRITE 0xFD  /* Data token start byte, Start Multiple Block Write */
#define SD_TOKEN_STOP_DATA_MULTIPLE_BLOCK_WRITE  0xFD  /* Data toke stop byte, Stop Multiple Block Write */

#define SD_CMD_GO_IDLE_STATE          0   /* CMD0 = 0x40  */
#define SD_CMD_SEND_OP_COND           1   /* CMD1 = 0x41  */
#define SD_CMD_SEND_IF_COND           8   /* CMD8 = 0x48  */
#define SD_CMD_SEND_CSD               9   /* CMD9 = 0x49  */
#define SD_CMD_SEND_CID               10  /* CMD10 = 0x4A */
#define SD_CMD_STOP_TRANSMISSION      12  /* CMD12 = 0x4C */
#define SD_CMD_SEND_STATUS            13  /* CMD13 = 0x4D */
#define SD_CMD_SET_BLOCKLEN           16  /* CMD16 = 0x50 */
#define SD_CMD_READ_SINGLE_BLOCK      17  /* CMD17 = 0x51 */
#define SD_CMD_READ_MULT_BLOCK        18  /* CMD18 = 0x52 */
#define SD_CMD_SET_BLOCK_COUNT        23  /* CMD23 = 0x57 */
#define SD_CMD_WRITE_SINGLE_BLOCK     24  /* CMD24 = 0x58 */
#define SD_CMD_WRITE_MULT_BLOCK       25  /* CMD25 = 0x59 */
#define SD_CMD_PROG_CSD               27  /* CMD27 = 0x5B */
#define SD_CMD_SET_WRITE_PROT         28  /* CMD28 = 0x5C */
#define SD_CMD_CLR_WRITE_PROT         29  /* CMD29 = 0x5D */
#define SD_CMD_SEND_WRITE_PROT        30  /* CMD30 = 0x5E */
#define SD_CMD_SD_ERASE_GRP_START     32  /* CMD32 = 0x60 */
#define SD_CMD_SD_ERASE_GRP_END       33  /* CMD33 = 0x61 */
#define SD_CMD_UNTAG_SECTOR           34  /* CMD34 = 0x62 */
#define SD_CMD_ERASE_GRP_START        35  /* CMD35 = 0x63 */
#define SD_CMD_ERASE_GRP_END          36  /* CMD36 = 0x64 */
#define SD_CMD_UNTAG_ERASE_GROUP      37  /* CMD37 = 0x65 */
#define SD_CMD_ERASE                  38  /* CMD38 = 0x66 */
#define SD_CMD_SD_APP_OP_COND         41  /* CMD41 = 0x69 */
#define SD_CMD_APP_CMD                55  /* CMD55 = 0x77 */
#define SD_CMD_READ_OCR               58  /* CMD55 = 0x79 */

typedef enum
{
/* R1 answer value */
  SD_R1_NO_ERROR            = (0x00),
  SD_R1_IN_IDLE_STATE       = (0x01),
  SD_R1_ERASE_RESET         = (0x02),
  SD_R1_ILLEGAL_COMMAND     = (0x04),
  SD_R1_COM_CRC_ERROR       = (0x08),
  SD_R1_ERASE_SEQUENCE_ERROR= (0x10),
  SD_R1_ADDRESS_ERROR       = (0x20),
  SD_R1_PARAMETER_ERROR     = (0x40),

/* R2 answer value */
  SD_R2_NO_ERROR            = 0x00,
  SD_R2_CARD_LOCKED         = 0x01,
  SD_R2_LOCKUNLOCK_ERROR    = 0x02,
  SD_R2_ERROR               = 0x04,
  SD_R2_CC_ERROR            = 0x08,
  SD_R2_CARD_ECC_FAILED     = 0x10,
  SD_R2_WP_VIOLATION        = 0x20,
  SD_R2_ERASE_PARAM         = 0x40,
  SD_R2_OUTOFRANGE          = 0x80,

/* brief  Data response error  */
  SD_DATA_OK                = (0x05),
  SD_DATA_CRC_ERROR         = (0x0B),
  SD_DATA_WRITE_ERROR       = (0x0D),
  SD_DATA_OTHER_ERROR       = (0xFF)
} SD_Error;

/* Private variables ---------------------------------------------------------*/

__IO uint8_t SdStatus = SD_NOT_PRESENT;

/* flag_SDHC :
      0 :  Standard capacity
      1 : High capacity
*/
uint16_t flag_SDHC = 0;


/* Private function prototypes -----------------------------------------------*/
static uint8_t SD_GetCIDRegister(SD_CID* Cid);
static uint8_t SD_GetCSDRegister(SD_CSD* Csd);
static uint8_t SD_GetDataResponse(void);
static uint8_t SD_GoIdleState(void);
static SD_CmdAnswer_typedef SD_SendCmd(uint8_t Cmd, uint32_t Arg, uint8_t Crc, uint8_t Answer);
static uint8_t SD_WaitData(uint8_t data);
static uint8_t SD_ReadData(void);

uint8_t SD_Init(void)
{
  /* Configure IO functionalities for SD pin */
  SD_IO_Init();

  SD_Status();

  /* SD initialized and set to SPI mode properly */
  SPI_Init(SPI_1,DIVIDE_4,SPI_MASTER);
  return SD_GoIdleState();
}

uint8_t SD_GetCardInfo(SD_CardInfo *pCardInfo)
{
  uint8_t status;

  status = SD_GetCSDRegister(&(pCardInfo->Csd));
  status|= SD_GetCIDRegister(&(pCardInfo->Cid));
  if(flag_SDHC == 1 )
  {
    pCardInfo->CardBlockSize = 512;
    pCardInfo->CardCapacity = (pCardInfo->Csd.version.v2.DeviceSize + 1) * pCardInfo->CardBlockSize;
  }
  else
  {
    pCardInfo->CardCapacity = (pCardInfo->Csd.version.v1.DeviceSize + 1) ;
    pCardInfo->CardCapacity *= (1 << (pCardInfo->Csd.version.v1.DeviceSizeMul + 2));
    pCardInfo->CardBlockSize = 1 << (pCardInfo->Csd.RdBlockLen);
    pCardInfo->CardCapacity *= pCardInfo->CardBlockSize;
  }

  return status;
}

uint8_t SD_ReadBlocks(uint32_t* pData, uint32_t ReadAddr, uint16_t BlockSize, uint32_t NumberOfBlocks)
{
  uint32_t offset = 0;
  uint8_t retr = SD_CARD_ERR;
  uint8_t *ptr = NULL;
  SD_CmdAnswer_typedef response;

  /* Send CMD16 (SD_CMD_SET_BLOCKLEN) to set the size of the block and
     Check if the SD acknowledged the set block length command: R1 response (0x00: no errors) */
  response = SD_SendCmd(SD_CMD_SET_BLOCKLEN, BlockSize, 0xFF, SD_ANSWER_R1_EXPECTED);
  SD_Deselect();
  SPI_Transmit_Byte(SD_DUMMY_BYTE);
  if ( response.r1 != SD_R1_NO_ERROR)
  {
     goto error;
  }

  ptr = malloc(sizeof(uint8_t)*BlockSize);
  if( ptr == NULL )
  {
     goto error;
  }
  memset(ptr, SD_DUMMY_BYTE, sizeof(uint8_t)*BlockSize);

  /* Data transfer */
  while (NumberOfBlocks--)
  {
    /* Send CMD17 (SD_CMD_READ_SINGLE_BLOCK) to read one block */
    /* Check if the SD acknowledged the read block command: R1 response (0x00: no errors) */
    response = SD_SendCmd(SD_CMD_READ_SINGLE_BLOCK, (ReadAddr + offset)/(flag_SDHC == 1 ?BlockSize: 1), 0xFF, SD_ANSWER_R1_EXPECTED);
    if ( response.r1 != SD_R1_NO_ERROR)
    {
      goto error;
    }

    /* Now look for the data token to signify the start of the data */
    if (SD_WaitData(SD_TOKEN_START_DATA_SINGLE_BLOCK_READ) == SD_CARD_OK)
    {
      /* Read the SD block data : read NumByteToRead data */
    	SPI_Transmit(ptr, (uint8_t*)pData + offset, BlockSize);	;
      //SD_IO_WriteReadData(ptr, (uint8_t*)pData + offset, BlockSize);

      /* Set next read address*/
      offset += BlockSize;
      /* get CRC bytes (not really needed by us, but required by SD) */
      SPI_Transmit_Byte(SD_DUMMY_BYTE);
      SPI_Transmit_Byte(SD_DUMMY_BYTE);
    }
    else
    {
      goto error;
    }

    /* End the command data read cycle */
    SD_Deselect();
    SPI_Transmit_Byte(SD_DUMMY_BYTE);
  }

  retr = SD_CARD_OK;

error :
  /* Send dummy byte: 8 Clock pulses of delay */
  SD_Deselect();
  SPI_Transmit_Byte(SD_DUMMY_BYTE);
  if(ptr != NULL) free(ptr);

  /* Return the response */
  return retr;
}

uint8_t SD_WriteBlocks(uint32_t* pData, uint32_t WriteAddr, uint16_t BlockSize, uint32_t NumberOfBlocks)
{
  uint32_t offset = 0;
  uint8_t retr = SD_CARD_ERR;
  uint8_t *ptr = NULL;
  SD_CmdAnswer_typedef response;

  /* Send CMD16 (SD_CMD_SET_BLOCKLEN) to set the size of the block and
     Check if the SD acknowledged the set block length command: R1 response (0x00: no errors) */
  response = SD_SendCmd(SD_CMD_SET_BLOCKLEN, BlockSize, 0xFF, SD_ANSWER_R1_EXPECTED);
  SD_Deselect();
  SPI_Transmit_Byte(SD_DUMMY_BYTE);
  if ( response.r1 != SD_R1_NO_ERROR)
  {
    goto error;
  }

  ptr = malloc(sizeof(uint8_t)*BlockSize);
  if (ptr == NULL)
  {
    goto error;
  }

  /* Data transfer */
  while (NumberOfBlocks--)
  {
    /* Send CMD24 (SD_CMD_WRITE_SINGLE_BLOCK) to write blocks  and
       Check if the SD acknowledged the write block command: R1 response (0x00: no errors) */
    response = SD_SendCmd(SD_CMD_WRITE_SINGLE_BLOCK, (WriteAddr + offset)/(flag_SDHC == 1 ? BlockSize: 1), 0xFF, SD_ANSWER_R1_EXPECTED);
    if (response.r1 != SD_R1_NO_ERROR)
    {
      goto error;
    }

    /* Send dummy byte for NWR timing : one byte between CMDWRITE and TOKEN */
    SPI_Transmit_Byte(SD_DUMMY_BYTE);
    SPI_Transmit_Byte(SD_DUMMY_BYTE);

    /* Send the data token to signify the start of the data */
    SPI_Transmit_Byte(SD_TOKEN_START_DATA_SINGLE_BLOCK_WRITE);

    /* Write the block data to SD */
    SPI_Transmit((uint8_t*)pData + offset, ptr, BlockSize);

    /* Set next write address */
    offset += BlockSize;

    /* Put CRC bytes (not really needed by us, but required by SD) */
    SPI_Transmit_Byte(SD_DUMMY_BYTE);
    SPI_Transmit_Byte(SD_DUMMY_BYTE);

    /* Read data response */
    if (SD_GetDataResponse() != SD_DATA_OK)
    {
      /* Set response value to failure */
      goto error;
    }

    SD_Deselect();
    SPI_Transmit_Byte(SD_DUMMY_BYTE);
  }
  retr = SD_CARD_OK;

error :
  if(ptr != NULL) free(ptr);
  /* Send dummy byte: 8 Clock pulses of delay */
  SD_Deselect();
  SPI_Transmit_Byte(SD_DUMMY_BYTE);

  /* Return the response */
  return retr;
}

uint8_t SD_Erase(uint32_t StartAddr, uint32_t EndAddr)
{
  uint8_t retr = SD_CARD_ERR;
  SD_CmdAnswer_typedef response;

  /* Send CMD32 (Erase group start) and check if the SD acknowledged the erase command: R1 response (0x00: no errors) */
  response = SD_SendCmd(SD_CMD_SD_ERASE_GRP_START, StartAddr, 0xFF, SD_ANSWER_R1_EXPECTED);
  SD_Deselect();
  SPI_Transmit_Byte(SD_DUMMY_BYTE);  if (response.r1 == SD_R1_NO_ERROR)
  {
    /* Send CMD33 (Erase group end) and Check if the SD acknowledged the erase command: R1 response (0x00: no errors) */
    response = SD_SendCmd(SD_CMD_SD_ERASE_GRP_END, EndAddr, 0xFF, SD_ANSWER_R1_EXPECTED);
    SD_Deselect();
    SPI_Transmit_Byte(SD_DUMMY_BYTE);
    if (response.r1 == SD_R1_NO_ERROR)
    {
      /* Send CMD38 (Erase) and Check if the SD acknowledged the erase command: R1 response (0x00: no errors) */
      response = SD_SendCmd(SD_CMD_ERASE, 0, 0xFF, SD_ANSWER_R1B_EXPECTED);
      if (response.r1 == SD_R1_NO_ERROR)
      {
        retr = SD_CARD_OK;
      }
      SD_Deselect();
      SPI_Transmit_Byte(SD_DUMMY_BYTE);
    }
  }

  /* Return the response */
  return retr;
}

uint8_t SD_GetStatus(void)
{
  SD_CmdAnswer_typedef retr;

  /* Send CMD13 (SD_SEND_STATUS) to get SD status */
  retr = SD_SendCmd(SD_CMD_SEND_STATUS, 0, 0xFF, SD_ANSWER_R2_EXPECTED);
  SD_Deselect();
  SPI_Transmit_Byte(SD_DUMMY_BYTE);

  /* Find SD status according to card state */
  if(( retr.r1 == SD_R1_NO_ERROR) && ( retr.r2 == SD_R2_NO_ERROR))
  {
    return SD_CARD_OK;
  }

  return SD_CARD_ERR;
}

uint8_t SD_GetCSDRegister(SD_CSD* Csd)
{
  uint16_t counter = 0;
  uint8_t CSD_Tab[16];
  uint8_t retr = SD_CARD_ERR;
  SD_CmdAnswer_typedef response;

  /* Send CMD9 (CSD register) or CMD10(CSD register) and Wait for response in the R1 format (0x00 is no errors) */
  response = SD_SendCmd(SD_CMD_SEND_CSD, 0, 0xFF, SD_ANSWER_R1_EXPECTED);
  if(response.r1 == SD_R1_NO_ERROR)
  {
    if (SD_WaitData(SD_TOKEN_START_DATA_SINGLE_BLOCK_READ) == SD_CARD_OK)
    {
      for (counter = 0; counter < 16; counter++)
      {
        /* Store CSD register value on CSD_Tab */
        CSD_Tab[counter] = SPI_Transmit_Byte(SD_DUMMY_BYTE);
      }

      /* Get CRC bytes (not really needed by us, but required by SD) */
      SPI_Transmit_Byte(SD_DUMMY_BYTE);
      SPI_Transmit_Byte(SD_DUMMY_BYTE);

      /*************************************************************************
        CSD header decoding
      *************************************************************************/

      /* Byte 0 */
      Csd->CSDStruct = (CSD_Tab[0] & 0xC0) >> 6;
      Csd->Reserved1 =  CSD_Tab[0] & 0x3F;

      /* Byte 1 */
      Csd->TAAC = CSD_Tab[1];

      /* Byte 2 */
      Csd->NSAC = CSD_Tab[2];

      /* Byte 3 */
      Csd->MaxBusClkFrec = CSD_Tab[3];

      /* Byte 4/5 */
      Csd->CardComdClasses = (CSD_Tab[4] << 4) | ((CSD_Tab[5] & 0xF0) >> 4);
      Csd->RdBlockLen = CSD_Tab[5] & 0x0F;

      /* Byte 6 */
      Csd->PartBlockRead   = (CSD_Tab[6] & 0x80) >> 7;
      Csd->WrBlockMisalign = (CSD_Tab[6] & 0x40) >> 6;
      Csd->RdBlockMisalign = (CSD_Tab[6] & 0x20) >> 5;
      Csd->DSRImpl         = (CSD_Tab[6] & 0x10) >> 4;

      /*************************************************************************
        CSD v1/v2 decoding
      *************************************************************************/

      if(flag_SDHC == 0)
      {
        Csd->version.v1.Reserved1 = ((CSD_Tab[6] & 0x0C) >> 2);

        Csd->version.v1.DeviceSize =  ((CSD_Tab[6] & 0x03) << 10)
                                    |  (CSD_Tab[7] << 2)
                                    | ((CSD_Tab[8] & 0xC0) >> 6);
        Csd->version.v1.MaxRdCurrentVDDMin = (CSD_Tab[8] & 0x38) >> 3;
        Csd->version.v1.MaxRdCurrentVDDMax = (CSD_Tab[8] & 0x07);
        Csd->version.v1.MaxWrCurrentVDDMin = (CSD_Tab[9] & 0xE0) >> 5;
        Csd->version.v1.MaxWrCurrentVDDMax = (CSD_Tab[9] & 0x1C) >> 2;
        Csd->version.v1.DeviceSizeMul = ((CSD_Tab[9] & 0x03) << 1)
                                       |((CSD_Tab[10] & 0x80) >> 7);
      }
      else
      {
        Csd->version.v2.Reserved1 = ((CSD_Tab[6] & 0x0F) << 2) | ((CSD_Tab[7] & 0xC0) >> 6);
        Csd->version.v2.DeviceSize= ((CSD_Tab[7] & 0x3F) << 16) | (CSD_Tab[8] << 8) | CSD_Tab[9];
        Csd->version.v2.Reserved2 = ((CSD_Tab[10] & 0x80) >> 8);
      }

      Csd->EraseSingleBlockEnable = (CSD_Tab[10] & 0x40) >> 6;
      Csd->EraseSectorSize   = ((CSD_Tab[10] & 0x3F) << 1)
                              |((CSD_Tab[11] & 0x80) >> 7);
      Csd->WrProtectGrSize   = (CSD_Tab[11] & 0x7F);
      Csd->WrProtectGrEnable = (CSD_Tab[12] & 0x80) >> 7;
      Csd->Reserved2         = (CSD_Tab[12] & 0x60) >> 5;
      Csd->WrSpeedFact       = (CSD_Tab[12] & 0x1C) >> 2;
      Csd->MaxWrBlockLen     = ((CSD_Tab[12] & 0x03) << 2)
                              |((CSD_Tab[13] & 0xC0) >> 6);
      Csd->WriteBlockPartial = (CSD_Tab[13] & 0x20) >> 5;
      Csd->Reserved3         = (CSD_Tab[13] & 0x1F);
      Csd->FileFormatGrouop  = (CSD_Tab[14] & 0x80) >> 7;
      Csd->CopyFlag          = (CSD_Tab[14] & 0x40) >> 6;
      Csd->PermWrProtect     = (CSD_Tab[14] & 0x20) >> 5;
      Csd->TempWrProtect     = (CSD_Tab[14] & 0x10) >> 4;
      Csd->FileFormat        = (CSD_Tab[14] & 0x0C) >> 2;
      Csd->Reserved4         = (CSD_Tab[14] & 0x03);
      Csd->crc               = (CSD_Tab[15] & 0xFE) >> 1;
      Csd->Reserved5         = (CSD_Tab[15] & 0x01);

      retr = SD_CARD_OK;
    }
  }

  /* Send dummy byte: 8 Clock pulses of delay */
  SD_Deselect();
  SPI_Transmit_Byte(SD_DUMMY_BYTE);

  /* Return the response */
  return retr;
}

uint8_t SD_GetCIDRegister(SD_CID* Cid)
{
  uint32_t counter = 0;
  uint8_t retr = SD_CARD_ERR;
  uint8_t CID_Tab[16];
  SD_CmdAnswer_typedef response;

  /* Send CMD10 (CID register) and Wait for response in the R1 format (0x00 is no errors) */
  response = SD_SendCmd(SD_CMD_SEND_CID, 0, 0xFF, SD_ANSWER_R1_EXPECTED);
  if(response.r1 == SD_R1_NO_ERROR)
  {
    if(SD_WaitData(SD_TOKEN_START_DATA_SINGLE_BLOCK_READ) == SD_CARD_OK)
    {
      /* Store CID register value on CID_Tab */
      for (counter = 0; counter < 16; counter++)
      {
        CID_Tab[counter] = SPI_Transmit_Byte(SD_DUMMY_BYTE);
      }

      /* Get CRC bytes (not really needed by us, but required by SD) */
      SPI_Transmit_Byte(SD_DUMMY_BYTE);
      SPI_Transmit_Byte(SD_DUMMY_BYTE);

      /* Byte 0 */
      Cid->ManufacturerID = CID_Tab[0];

      /* Byte 1 */
      Cid->OEM_AppliID = CID_Tab[1] << 8;

      /* Byte 2 */
      Cid->OEM_AppliID |= CID_Tab[2];

      /* Byte 3 */
      Cid->ProdName1 = CID_Tab[3] << 24;

      /* Byte 4 */
      Cid->ProdName1 |= CID_Tab[4] << 16;

      /* Byte 5 */
      Cid->ProdName1 |= CID_Tab[5] << 8;

      /* Byte 6 */
      Cid->ProdName1 |= CID_Tab[6];

      /* Byte 7 */
      Cid->ProdName2 = CID_Tab[7];

      /* Byte 8 */
      Cid->ProdRev = CID_Tab[8];

      /* Byte 9 */
      Cid->ProdSN = CID_Tab[9] << 24;

      /* Byte 10 */
      Cid->ProdSN |= CID_Tab[10] << 16;

      /* Byte 11 */
      Cid->ProdSN |= CID_Tab[11] << 8;

      /* Byte 12 */
      Cid->ProdSN |= CID_Tab[12];

      /* Byte 13 */
      Cid->Reserved1 |= (CID_Tab[13] & 0xF0) >> 4;
      Cid->ManufactDate = (CID_Tab[13] & 0x0F) << 8;

      /* Byte 14 */
      Cid->ManufactDate |= CID_Tab[14];

      /* Byte 15 */
      Cid->CID_CRC = (CID_Tab[15] & 0xFE) >> 1;
      Cid->Reserved2 = 1;

      retr = SD_CARD_OK;
    }
  }

  /* Send dummy byte: 8 Clock pulses of delay */
  SD_Deselect();
  SPI_Transmit_Byte(SD_DUMMY_BYTE);

  /* Return the reponse */
  return retr;
}

SD_CmdAnswer_typedef SD_SendCmd(uint8_t Cmd, uint32_t Arg, uint8_t Crc, uint8_t Answer)
{
  uint8_t frame[SD_CMD_LENGTH], frameout[SD_CMD_LENGTH];
  SD_CmdAnswer_typedef retr = {0xFF, 0xFF , 0xFF, 0xFF, 0xFF};

  /* R1 Lenght = NCS(0)+ 6 Bytes command + NCR(min1 max8) + 1 Bytes answer + NEC(0) = 15bytes */
  /* R1b identical to R1 + Busy information                                                   */
  /* R2 Lenght = NCS(0)+ 6 Bytes command + NCR(min1 max8) + 2 Bytes answer + NEC(0) = 16bytes */

  /* Prepare Frame to send */
  frame[0] = (Cmd | 0x40);         /* Construct byte 1 */
  frame[1] = (uint8_t)(Arg >> 24); /* Construct byte 2 */
  frame[2] = (uint8_t)(Arg >> 16); /* Construct byte 3 */
  frame[3] = (uint8_t)(Arg >> 8);  /* Construct byte 4 */
  frame[4] = (uint8_t)(Arg);       /* Construct byte 5 */
  frame[5] = (Crc | 0x01);         /* Construct byte 6 */

  /* Send the command */
  SD_Select();
  SPI_Transmit(frame, frameout, SD_CMD_LENGTH); /* Send the Cmd bytes */

  switch(Answer)
  {
  case SD_ANSWER_R1_EXPECTED :
    retr.r1 = SD_ReadData();
    break;
  case SD_ANSWER_R1B_EXPECTED :
    retr.r1 = SD_ReadData();
    retr.r2 = SPI_Transmit_Byte(SD_DUMMY_BYTE);
    /* Set CS High */
    SD_Deselect();
    Delay_Millis(1);
    /* Set CS Low */
    SD_Select();

    /* Wait IO line return 0xFF */
    while (SPI_Transmit_Byte(SD_DUMMY_BYTE) != 0xFF);
    break;
  case SD_ANSWER_R2_EXPECTED :
    retr.r1 = SD_ReadData();
    retr.r2 = SPI_Transmit_Byte(SD_DUMMY_BYTE);
    break;
  case SD_ANSWER_R3_EXPECTED :
  case SD_ANSWER_R7_EXPECTED :
    retr.r1 = SD_ReadData();
    retr.r2 = SPI_Transmit_Byte(SD_DUMMY_BYTE);
    retr.r3 = SPI_Transmit_Byte(SD_DUMMY_BYTE);
    retr.r4 = SPI_Transmit_Byte(SD_DUMMY_BYTE);
    retr.r5 = SPI_Transmit_Byte(SD_DUMMY_BYTE);
    break;
  default :
    break;
  }
  return retr;
}

uint8_t SD_GetDataResponse(void)
{
  uint8_t dataresponse;
  uint8_t rvalue = SD_DATA_OTHER_ERROR;

  dataresponse = SPI_Transmit_Byte(SD_DUMMY_BYTE);
  SPI_Transmit_Byte(SD_DUMMY_BYTE); /* read the busy response byte*/

  /* Mask unused bits */
  switch (dataresponse & 0x1F)
  {
  case SD_DATA_OK:
    rvalue = SD_DATA_OK;

    /* Set CS High */
    SD_Deselect();
    /* Set CS Low */
    SD_Select();

    /* Wait IO line return 0xFF */
    while (SPI_Transmit_Byte(SD_DUMMY_BYTE) != 0xFF);
    break;
  case SD_DATA_CRC_ERROR:
    rvalue =  SD_DATA_CRC_ERROR;
    break;
  case SD_DATA_WRITE_ERROR:
    rvalue = SD_DATA_WRITE_ERROR;
    break;
  default:
    break;
  }

  /* Return response */
  return rvalue;
}

uint8_t SD_GoIdleState(void)
{
  SD_CmdAnswer_typedef response;
  __IO uint8_t counter = 0;
  /* Send CMD0 (SD_CMD_GO_IDLE_STATE) to put SD in SPI mode and
     wait for In Idle State Response (R1 Format) equal to 0x01 */
  do{
    counter++;
    response = SD_SendCmd(SD_CMD_GO_IDLE_STATE, 0, 0x95, SD_ANSWER_R1_EXPECTED);
    SD_Deselect();
    SPI_Transmit_Byte(SD_DUMMY_BYTE);
    if(counter >= SD_MAX_TRY)
    {
      return SD_CARD_ERR;
    }
  }
  while(response.r1 != SD_R1_IN_IDLE_STATE);


  /* Send CMD8 (SD_CMD_SEND_IF_COND) to check the power supply status
     and wait until response (R7 Format) equal to 0xAA and */
  response = SD_SendCmd(SD_CMD_SEND_IF_COND, 0x1AA, 0x87, SD_ANSWER_R7_EXPECTED);
  SD_Deselect();
  SPI_Transmit_Byte(SD_DUMMY_BYTE);
  if((response.r1  & SD_R1_ILLEGAL_COMMAND) == SD_R1_ILLEGAL_COMMAND)
  {
    /* Initialize card V1 */
    do
    {
      /* initialise card V1 */
      /* Send CMD55 (SD_CMD_APP_CMD) before any ACMD command: R1 response (0x00: no errors) */
      response = SD_SendCmd(SD_CMD_APP_CMD, 0x00000000, 0xFF, SD_ANSWER_R1_EXPECTED);
      SD_Deselect();
      SPI_Transmit_Byte(SD_DUMMY_BYTE);

      /* Send ACMD41 (SD_CMD_SD_APP_OP_COND) to initialize SDHC or SDXC cards: R1 response (0x00: no errors) */
      response = SD_SendCmd(SD_CMD_SD_APP_OP_COND, 0x00000000, 0xFF, SD_ANSWER_R1_EXPECTED);
      SD_Deselect();
      SPI_Transmit_Byte(SD_DUMMY_BYTE);
    }
    while(response.r1 == SD_R1_IN_IDLE_STATE);
    flag_SDHC = 0;
  }
  else if(response.r1 == SD_R1_IN_IDLE_STATE)
  {
      /* Initialize card V2 */
    do {

      /* Send CMD55 (SD_CMD_APP_CMD) before any ACMD command: R1 response (0x00: no errors) */
      response = SD_SendCmd(SD_CMD_APP_CMD, 0, 0xFF, SD_ANSWER_R1_EXPECTED);
      SD_Deselect();
      SPI_Transmit_Byte(SD_DUMMY_BYTE);

      /* Send ACMD41 (SD_CMD_SD_APP_OP_COND) to initialize SDHC or SDXC cards: R1 response (0x00: no errors) */
      response = SD_SendCmd(SD_CMD_SD_APP_OP_COND, 0x40000000, 0xFF, SD_ANSWER_R1_EXPECTED);
      SD_Deselect();
      SPI_Transmit_Byte(SD_DUMMY_BYTE);
    }
    while(response.r1 == SD_R1_IN_IDLE_STATE);

    if((response.r1 & SD_R1_ILLEGAL_COMMAND) == SD_R1_ILLEGAL_COMMAND)
    {
      do {
        /* Send CMD55 (SD_CMD_APP_CMD) before any ACMD command: R1 response (0x00: no errors) */
        response = SD_SendCmd(SD_CMD_APP_CMD, 0, 0xFF, SD_ANSWER_R1_EXPECTED);
        SD_Deselect();
        SPI_Transmit_Byte(SD_DUMMY_BYTE);
        if(response.r1 != SD_R1_IN_IDLE_STATE)
        {
          return SD_CARD_ERR;
        }
        /* Send ACMD41 (SD_CMD_SD_APP_OP_COND) to initialize SDHC or SDXC cards: R1 response (0x00: no errors) */
        response = SD_SendCmd(SD_CMD_SD_APP_OP_COND, 0x00000000, 0xFF, SD_ANSWER_R1_EXPECTED);
        SD_Deselect();
        SPI_Transmit_Byte(SD_DUMMY_BYTE);
      }
      while(response.r1 == SD_R1_IN_IDLE_STATE);
    }

    /* Send CMD58 (SD_CMD_READ_OCR) to initialize SDHC or SDXC cards: R3 response (0x00: no errors) */
    response = SD_SendCmd(SD_CMD_READ_OCR, 0x00000000, 0xFF, SD_ANSWER_R3_EXPECTED);
    SD_Deselect();
    SPI_Transmit_Byte(SD_DUMMY_BYTE);
    if(response.r1 != SD_R1_NO_ERROR)
    {
      return SD_CARD_ERR;
    }
    flag_SDHC = (response.r2 & 0x40) >> 6;
  }
  else
  {
    return SD_CARD_ERR;
  }

  return SD_CARD_OK;
}

uint8_t SD_ReadData(void)
{
  uint8_t timeout = 0x08;
  uint8_t readvalue;

  /* Check if response is got or a timeout is happened */
  do
  {
    readvalue = SPI_Transmit_Byte(SD_DUMMY_BYTE);
    timeout--;

  }while ((readvalue == SD_DUMMY_BYTE) && timeout);

  /* Got correct response */
  return readvalue;
}

uint8_t SD_WaitData(uint8_t data)
{
  uint16_t timeout = 0xFFFF;
  uint8_t readvalue;

  /* Check if response is got or a timeout is happened */
  do
  {
    readvalue = SPI_Transmit_Byte(SD_DUMMY_BYTE);
    timeout--;
  }while ((readvalue != data) && timeout);

  if (timeout == 0)
  {
    /* After time out */
    return SD_CARD_TIMEOUT;
  }

  /* Got correct response */
  return SD_CARD_OK;
}

void SD_IO_Init(void)
{
  uint8_t counter = 0;

  GPIO_Init(GPIO_A,SD_CARD_CS,GPIO_OUTPUT,PULLUP);
  GPIO_Init(GPIO_B,SD_CARD_DETECT,GPIO_INPUT,PULLUP);


  /*------------Put SD in SPI mode--------------*/
  /* SD SPI Configuration */
  SPI_Init(SPI_1,DIVIDE_256,SPI_MASTER);

  /* SD chip select high */
  SD_Deselect();

  /* Send dummy byte 0xFF, 10 times with CS high */
  /* Rise CS and MOSI for 80 clocks cycles */
  for (counter = 0; counter <= 9; counter++)
  {
    /* Send dummy byte 0xFF */
    SPI_Transmit_Byte(SD_DUMMY_BYTE);
  }
}

void SD_IO_CSState(uint8_t val)
{
  if(val == 1)
  {
	  GPIO_Write(GPIO_A,SD_CARD_CS,PIN_HIGH);
  }
  else
  {
	  GPIO_Write(GPIO_A,SD_CARD_CS,PIN_LOW);;
  }
}

void SD_Select()
{
	GPIO_Write(GPIO_A,SD_CARD_CS,PIN_LOW);
}

void SD_Deselect()
{
	GPIO_Write(GPIO_A,SD_CARD_CS,PIN_HIGH);
}

void SD_Status()
{
	SdStatus = SD_NOT_PRESENT;

	if(GPIO_Read(GPIO_B,SD_CARD_DETECT) == PIN_LOW)
		SdStatus = SD_PRESENT;
}
