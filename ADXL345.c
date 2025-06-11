/*
 * ADXL345.c
 *
 *  Created on: 	Jan 25, 2025
 *  Author: 		Kamil Kozdro
 */

#include "ADXL345.h"

/* ***** Private functions ***** */

static void adxlWrite(ADXL_HandleTypeDef* adxlHandler, ADXL_Reg adxlReg, uint8_t dataToWrite)
{
	uint8_t writeBuffer[2] = {0,0};
	writeBuffer[0] = adxlReg|ADXL_MULTIPLEBYTE_READ;
	writeBuffer[1] = dataToWrite;

	HAL_GPIO_WritePin(adxlHandler->CS_Port, adxlHandler->CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(adxlHandler->hspi, writeBuffer, sizeof(writeBuffer), ADXL_TRANSMISSION_TIMEOUT);
	HAL_GPIO_WritePin(adxlHandler->CS_Port, adxlHandler->CS_Pin, GPIO_PIN_SET);
}

static void adxlRead(ADXL_HandleTypeDef* adxlHandler, ADXL_Reg adxlReg, uint8_t* readBuffer, size_t dataLength)
{
	adxlReg |= ADXL_READ;
	adxlReg |= ADXL_MULTIPLEBYTE_READ;

	HAL_GPIO_WritePin(adxlHandler->CS_Port, adxlHandler->CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(adxlHandler->hspi, &adxlReg, sizeof(adxlReg), ADXL_TRANSMISSION_TIMEOUT);
	HAL_SPI_Receive(adxlHandler->hspi, readBuffer, dataLength, ADXL_TRANSMISSION_TIMEOUT);
	HAL_GPIO_WritePin(adxlHandler->CS_Port, adxlHandler->CS_Pin, GPIO_PIN_SET);
}

/* ***** Public functions ***** */

ADXL_StatusTypeDef adxlInit(ADXL_HandleTypeDef* adxlHandler, ADXL_InitTypeDef* adxlInitType)
{
	adxlHandler->CS_Port = adxlInitType->CS_Port;
	adxlHandler->CS_Pin = adxlInitType->CS_Pin;
	adxlHandler->hspi = adxlInitType->hspi;

	HAL_GPIO_WritePin(adxlHandler->CS_Port, adxlHandler->CS_Pin, GPIO_PIN_SET);
	uint8_t deviceID = 0;
	adxlRead(adxlHandler, ADXL_REG_DEVID, &deviceID, sizeof(deviceID));
	if(deviceID != ADXL_DEVICE_ID)
		return ADXL_ERROR;
	adxlStandby(adxlHandler);

	/* --- Setting Bandwidth and PowerMode --- */
	uint8_t writeBuffer = 0;
	if(adxlInitType->powerMode == ADXL_POWER_MODE_LOW && adxlInitType->bwRate > ADXL_BW_RATE_400)
		adxlInitType->bwRate = ADXL_BW_RATE_200;
	writeBuffer = adxlInitType->bwRate;
	if(adxlInitType->powerMode == ADXL_POWER_MODE_LOW)
			writeBuffer |= (1 << ADXL_BW_RATE_LOW_POWER_BIT);
	adxlWrite(adxlHandler, ADXL_REG_BW_RATE, writeBuffer);

	/* --- Setting DataFormat --- */
	writeBuffer = 0;
	writeBuffer = (adxlInitType->spiMode << ADXL_DATA_FORMAT_SPI_BIT)
					| (adxlInitType->intInvert << ADXL_DATA_FORMAT_INT_INVERT_BIT)
					| (adxlInitType->resMode << ADXL_DATA_FORMAT_FULL_RES_BIT)
					| (adxlInitType->justify << ADXL_DATA_FORMAT_JUSTIFY_BIT);
	writeBuffer += adxlInitType->dataFormatRange;
	adxlWrite(adxlHandler, ADXL_REG_DATA_FORMAT, writeBuffer);

	if (adxlInitType->resMode == ADXL_RES_MODE_10BIT)
	{
		switch (adxlInitType->dataFormatRange)
		{
			case ADXL_DATA_FORMAT_RANGE_2G:
				adxlHandler->gain = 1/255.0f;
				break;
			case ADXL_DATA_FORMAT_RANGE_4G:
				adxlHandler->gain = 1/127.0f;
				break;
			case ADXL_DATA_FORMAT_RANGE_8G:
				adxlHandler->gain = 1/63.0f;
				break;
			case ADXL_DATA_FORMAT_RANGE_16G:
				adxlHandler->gain = 1/31.0f;
				break;
		}
	}
	else
		adxlHandler->gain = 1/255.0f;

	return ADXL_OK;
}

ADXL_StatusTypeDef adxlInitDefault(ADXL_HandleTypeDef* adxlHandler, GPIO_TypeDef* CS_GPIOPort, uint16_t CS_GPIOPin, SPI_HandleTypeDef* spiHandler)
{
	  ADXL_InitTypeDef adxlInitType;
	  adxlInitType.bwRate = ADXL_BW_RATE_100;
	  adxlInitType.intInvert = ADXL_INT_INVERT_HIGH;
	  adxlInitType.justify = ADXL_JUSTIFY_SIGNED;
	  adxlInitType.dataFormatRange = ADXL_DATA_FORMAT_RANGE_2G;
	  adxlInitType.resMode = ADXL_RES_MODE_10BIT;
	  adxlInitType.spiMode = ADXL_SPI_MODE_4WIRE;
	  adxlInitType.CS_Port = CS_GPIOPort;
	  adxlInitType.CS_Pin = CS_GPIOPin;
	  adxlInitType.hspi = spiHandler;

	  if(adxlInit(adxlHandler, &adxlInitType) != ADXL_OK)
		  return ADXL_ERROR;

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

//
void adxlSleep(ADXL_HandleTypeDef* adxlHandler, ADXL_PowerCTLWakeupType powerCTLWakeup)
{
	uint8_t writeBuf = 0;

	adxlWrite(adxlHandler, ADXL_REG_POWER_CTL, 0);	// Enter standby mode to ensure that the device is properly biased (according to doc)

	writeBuf = powerCTLWakeup;
	writeBuf |= (1 << ADXL_POWER_CTL_MEASURE_BIT);
	writeBuf |= (1 << ADXL_POWER_CTL_SLEEP_BIT);
	adxlWrite(adxlHandler, ADXL_REG_POWER_CTL, writeBuf);


}

float_t adxlGet1AxisAccel(ADXL_HandleTypeDef* adxlHandler, ADXL_Axis adxlAxis)
{
	uint8_t readBuffer[2] = {0,0};
	int16_t accelReadVal = 0;
	float_t fAccelReadVal = 0;
	ADXL_Reg adxlAxisReg = ADXL_REG_DATAX0;

	switch(adxlAxis)
	{
		case ADXL_AXIS_X:
			adxlAxisReg = ADXL_REG_DATAX0;
			break;
		case ADXL_AXIS_Y:
			adxlAxisReg = ADXL_REG_DATAY0;
			break;
		case ADXL_AXIS_Z:
			adxlAxisReg = ADXL_REG_DATAZ0;
			break;
	}


	adxlRead(adxlHandler, adxlAxisReg, readBuffer, sizeof(readBuffer));
	accelReadVal = readBuffer[1] << 8 | readBuffer[0];
	fAccelReadVal = (float_t)accelReadVal*adxlHandler->gain;

	return fAccelReadVal;
}

void adxlGet3AxisAccel(ADXL_HandleTypeDef* adxlHandler, float_t (*accel)[3])
{
	uint8_t readBuffer[6] = {0,0,0,0,0,0};

	adxlRead(adxlHandler, ADXL_REG_DATAX0, readBuffer, sizeof(readBuffer));

	int16_t x = readBuffer[1] << 8 | readBuffer[0];
	int16_t y = readBuffer[3] << 8 | readBuffer[2];
	int16_t z = readBuffer[5] << 8 | readBuffer[4];

	(*accel)[0] = (float_t)x*adxlHandler->gain;
	(*accel)[1] = (float_t)y*adxlHandler->gain;
	(*accel)[2] = (float_t)z*adxlHandler->gain;

}

/*
 * 	Max offset = +/-1981 mg
 */

void adxlSetOffsets(ADXL_HandleTypeDef* adxlHandler, int16_t OffsetX, int16_t OffsetY, int16_t OffsetZ)
{
	const float_t scaleFactor = 15.6;
	const int16_t maxOffset = 1981;
	const int16_t minOffset = -1981;

	OffsetX = OffsetX < minOffset ? minOffset : OffsetX;
	OffsetX = OffsetX > maxOffset ? maxOffset : OffsetX;
	OffsetY = OffsetY < minOffset ? minOffset : OffsetY;
	OffsetY = OffsetY > maxOffset ? maxOffset : OffsetY;
	OffsetZ = OffsetZ < minOffset ? minOffset : OffsetZ;
	OffsetZ = OffsetZ > maxOffset ? maxOffset : OffsetZ;


	adxlWrite(adxlHandler, ADXL_REG_OFSX, (uint8_t)roundf(OffsetX/scaleFactor));
	adxlWrite(adxlHandler, ADXL_REG_OFSY, (uint8_t)roundf(OffsetY/scaleFactor));
	adxlWrite(adxlHandler, ADXL_REG_OFSZ, (uint8_t)roundf(OffsetZ/scaleFactor));
}

uint8_t adxlReadRegister(ADXL_HandleTypeDef* adxlHandler, ADXL_Reg adxlReg)
{
	uint8_t readBuffer = 0;
	adxlRead(adxlHandler, adxlReg, &readBuffer, sizeof(readBuffer));
	return readBuffer;
}



