/* Includes ------------------------------------------------------------------*/

#include "main.h"
#include "stm32f1xx_hal.h"
#include "usb_device.h"
#include "stm32f1xx_hal_msp.h"
#include "setup.h"
#include "SDCard.hpp"

SDCard sdCard(&hspi1, SD_CS_Pin, SD_CS_GPIO_Port);

void error();
uint8_t FreePercentage = 0;
int main(void) {
	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();
	setup();

	uint8_t count = 0;
	while (!sdCard.initalize()) {

		count++;
	}
	//we want to setup PA12 as output, drive low, then setup back as usb
	//This causes a reset on the hub so that the computer will rescan the usb devices info
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.Pin = GPIO_PIN_12;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	MX_USB_DEVICE_Init();

	for (;;) {
		HAL_Delay(1000);
	}

}

/***------- SD Card C-> C++ link functions ---------*/
extern "C" {
bool SDCardReadSect(uint8_t* bufferOut, uint32_t sector, uint16_t count) {
	if (!sdCard.readBlock(sector, bufferOut))
		return false;
	return true;
}
bool SDCardWriteSect(uint8_t* bufferOut, uint32_t sector, uint16_t count) {
	if (!sdCard.writeBlock(sector, bufferOut))
		return false;
	return true;
}
uint32_t SDCardSectorCount() {

	return sdCard.getSize();
}

}

void error() {

}
