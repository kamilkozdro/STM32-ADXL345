/*
 * ADXL345.c
 *
 *  Created on: 	Jan 25, 2025
 *  Author: 		Kamil Kozdro
 */

#include "ADXL345.h"


static void adxlWrite(ADXL_HandleTypeDef* adxlHandler, uint8_t regAddr, uint8_t dataToWrite)
{
	uint8_t writeBuffer[2] = {0,0};
	writeBuffer[0] = regAddr|ADXL_MULTIPLEBYTE_READ;
	writeBuffer[1] = dataToWrite;

	HAL_GPIO_WritePin(adxlHandler->CS_Port, adxlHandler->CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(adxlHandler->hspi, writeBuffer, sizeof(writeBuffer), ADXL_TRANSMISSION_TIMEOUT);
	HAL_GPIO_WritePin(adxlHandler->CS_Port, adxlHandler->CS_Pin, GPIO_PIN_SET);
}

static void adxlRead(ADXL_HandleTypeDef* adxlHandler, uint8_t regAddr, uint8_t* readBuffer, size_t dataLength)
{
	regAddr |= ADXL_READ;
	regAddr |= ADXL_MULTIPLEBYTE_READ;

	HAL_GPIO_WritePin(adxlHandler->CS_Port, adxlHandler->CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(adxlHandler->hspi, &regAddr, sizeof(regAddr), ADXL_TRANSMISSION_TIMEOUT);
	HAL_SPI_Receive(adxlHandler->hspi, readBuffer, dataLength, ADXL_TRANSMISSION_TIMEOUT);
	HAL_GPIO_WritePin(adxlHandler->CS_Port, adxlHandler->CS_Pin, GPIO_PIN_SET);
}

ADXL_StatusTypeDef adxlInit(ADXL_HandleTypeDef* adxlHandler, ADXL_InitTypeDef* adxlInitType)
{
	adxlHandler->CS_Port = adxlInitType->CS_Port;
	adxlHandler->CS_Pin = adxlInitType->CS_Pin;
	adxlHandler->hspi = adxlInitType->hspi;

	HAL_GPIO_WritePin(adxlHandler->CS_Port, adxlHandler->CS_Pin, GPIO_PIN_SET);
	// TODO: CHECK IF THIS DELAY IS NECESSARY
	HAL_Delay(5);
	uint8_t deviceID = 0;
	adxlRead(adxlHandler, ADXL_REG_DEVID, &deviceID, sizeof(deviceID));
	if(deviceID != ADXL_DEVICE_ID)
	{
		// TODO: REPORT ADXL COMM ERROR
		return ADXL_ERROR;
	}

	adxlStandby(adxlHandler);

	// Setting BW
	if(adxlInitType->bwRate > ADXL_BW_RATE_1600)
	{
		// TODO: REPORT ADXL BW_RATE ERROR
		return ADXL_ERROR;
	}
	adxlWrite(adxlHandler, ADXL_REG_BW_RATE, adxlInitType->bwRate);


	uint8_t writeBuffer = 0;

	// Setting DATAFORMAT
	writeBuffer = (adxlInitType->spiMode << ADXL_DATA_FORMAT_SPI_BIT)
					| (adxlInitType->intInvert << ADXL_DATA_FORMAT_INT_INVERT_BIT)
					| (adxlInitType->resMode << ADXL_DATA_FORMAT_FULL_RES_BIT)
					| (adxlInitType->justify << ADXL_DATA_FORMAT_JUSTIFY_BIT);
	writeBuffer += adxlInitType->range;
	adxlWrite(adxlHandler, ADXL_REG_BW_RATE, adxlInitType->bwRate);



	if (adxlInitType->resMode == ADXL_RES_MODE_10BIT)
	{
		switch (adxlInitType->range)
		{
			case ADXL_DATA_FORMAT_RANGE_2:
				adxlHandler->gain = 1/255.0f;
				break;
			case ADXL_DATA_FORMAT_RANGE_4:
				adxlHandler->gain = 1/128.0f;
				break;
			case ADXL_DATA_FORMAT_RANGE_8:
				adxlHandler->gain = 1/63.0f;
				break;
			case ADXL_DATA_FORMAT_RANGE_16:
				adxlHandler->gain = 1/31.0f;
				break;
		}
	}
	else
		adxlHandler->gain = 1/255.0f;

	return ADXL_OK;
}

void adxlStandby(ADXL_HandleTypeDef* adxlHandler)
{
	adxlWrite(adxlHandler, ADXL_REG_POWER_CTL, 0);
}

void adxlMeasure(ADXL_HandleTypeDef* adxlHandler)
{
	adxlWrite(adxlHandler, ADXL_REG_POWER_CTL, (1 << ADXL_POWER_CTL_MEASURE_BIT));
}

void adxlGetAccel(ADXL_HandleTypeDef* adxlHandler, float accel[3])
{
	uint8_t readBuffer[6] = {0,0,0,0,0,0};

	adxlRead(adxlHandler, ADXL_REG_DATAX0, readBuffer, sizeof(readBuffer));

	int16_t x = readBuffer[1] << 8 | readBuffer[0];
	int16_t y = readBuffer[3] << 8 | readBuffer[2];
	int16_t z = readBuffer[5] << 8 | readBuffer[4];

	accel[0] = (float)x*adxlHandler->gain;
	accel[1] = (float)y*adxlHandler->gain;
	accel[2] = (float)z*adxlHandler->gain;

}



