/*
 * DualSD.cpp
 *
 *  Created on: 1 Nov 2016
 *      Author: ralim
 */

#include <SDCard.hpp>

SDCard::SDCard(volatile SPI_HandleTypeDef* hspi, uint16_t cs1Pin,
		GPIO_TypeDef* cs1Port) {
	_spi = hspi;
	_cs1Pin = cs1Pin;
	_cs1Port = cs1Port;
	_sdSize = 0;
}

uint32_t SDCard::getSize() {
	if (_sdSize == 0) {
		//We have not read the disk size just yet :O
		csd_t csd;
		if (!readCSD(&csd))
			return 0;
		if (csd.v1.csd_ver == 0) {
			uint8_t read_bl_len = csd.v1.read_bl_len;
			uint16_t c_size = (csd.v1.c_size_high << 10)
					| (csd.v1.c_size_mid << 2) | csd.v1.c_size_low;
			uint8_t c_size_mult = (csd.v1.c_size_mult_high << 1)
					| csd.v1.c_size_mult_low;
			c_size = (uint32_t) (c_size + 1) << (c_size_mult + read_bl_len - 7);
			c_size /= 1024;
		} else if (csd.v2.csd_ver == 1) {
			uint32_t c_size = ((uint32_t) csd.v2.c_size_high << 16)
					| (csd.v2.c_size_mid << 8) | csd.v2.c_size_low;
			c_size *= 1024;
			_sdSize = c_size;

		} else {
		}
	}
	return _sdSize;
}
/** read CID or CSR register */
uint8_t SDCard::readRegister(uint8_t cmd, void* buf) {
	uint8_t* dst = reinterpret_cast<uint8_t*>(buf);
	if (cardCommand(cmd, 0)) {
		deselectCard();
		return false;
	}
	uint8_t temp = 0xFF;
	while (temp == 0xFF) {
		HAL_SPI_Receive((SPI_HandleTypeDef*) _spi, &temp, 1, 100);
	}
	// transfer data
	SPI_Recieve(dst, 16);
	SPI_Recieve(&temp, 1); //CRC1
	SPI_Recieve(&temp, 1); //CRC2
	deselectCard();
	return true;
}
bool SDCard::readBlock(uint32_t blockaddr, uint8_t* buffer) {
	if (cardCommand(CMD17, blockaddr)) {
		/*
		 * Error
		 */
		deselectCard();
		return false;
	}
	uint8_t temp = 0xFF;
	while (temp == 0xFF) {
		HAL_SPI_Receive((SPI_HandleTypeDef*) _spi, &temp, 1, 100);
	}

	SPI_Recieve(buffer, 512);
	//eat the CRC
	temp = 0xFF;
	SPI_Recieve(&temp, 1);
	temp = 0xFF;
	SPI_Recieve(&temp, 1);
	deselectCard();
	return true;
}

bool SDCard::writeBlock(uint32_t blockaddr, uint8_t* buffer) {
	//The cardCommand will select the card so we have to make sure we clean up
	if (cardCommand(CMD24, blockaddr)) {
		/*
		 * Error
		 */
		deselectCard();
		return false;
	}
	/*
	 * Write the data
	 */
	uint8_t temp = DATA_START_BLOCK;
	HAL_SPI_Transmit((SPI_HandleTypeDef*) _spi, &temp, 1, 100);
	HAL_SPI_Transmit((SPI_HandleTypeDef*) _spi, buffer, 512, 100);
	temp = 0xFF;
	HAL_SPI_Transmit((SPI_HandleTypeDef*) _spi, &temp, 1, 100);
	HAL_SPI_Transmit((SPI_HandleTypeDef*) _spi, &temp, 1, 100);
	//read response
	SPI_Recieve(&temp, 1);
	if ((temp & DATA_RES_MASK) != DATA_RES_ACCEPTED) {
		/*
		 * Error
		 */
		deselectCard();
		return false;
	}
	// wait for flash programming to complete
	waitUntilReady();

	// response is r2 so get and check two bytes for nonzero
	if (cardCommand(CMD13, 0)) {
		/*
		 * Error
		 */
		deselectCard();
		return false;
	}
	SPI_Recieve(&temp, 1);
	if (temp) {
		/*
		 * Error
		 */
		deselectCard();
		return false;
	}
	deselectCard();
	return true;
}

void SDCard::waitUntilReady() {
	uint8_t ans[1] = { 0 };
	while (ans[0] != 0xFF) {
		SPI_Recieve(ans, 1);
	}
}

bool SDCard::initalize() {
	_spi->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256; //slow down at first
	HAL_SPI_Init((SPI_HandleTypeDef*) _spi); //apply the speed change
	deselectCard();
//We must supply at least 74 clocks with CS high
	uint8_t buffer[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
	HAL_SPI_Transmit((SPI_HandleTypeDef*) _spi, buffer, 4, 100);
	HAL_SPI_Transmit((SPI_HandleTypeDef*) _spi, buffer, 4, 100);
	HAL_SPI_Transmit((SPI_HandleTypeDef*) _spi, buffer, 4, 100);
	HAL_Delay(3);
	selectCard();
	uint8_t status;
	uint16_t attempts = 0;
	// command to go idle in SPI mode
	while ((status = cardCommand(CMD0, 0)) != R1_IDLE_STATE) {
		attempts++;
		if (attempts > 300)
			return false;
	}
	// check SD version
	if ((cardCommand(CMD8, 0x1AA) & R1_ILLEGAL_COMMAND)) {
		deselectCard();
		return false; //Unsupported
	} else {
		// only need last byte of r7 response
		HAL_SPI_Receive((SPI_HandleTypeDef*) _spi, buffer, 4, 100);
		if (buffer[3] != 0XAA) {
			return false; //failed check
		}

	}
	// initialize card and send host supports SDHC
	while ((status = cardAcmd(ACMD41, 0X40000000)) != R1_READY_STATE) {

	}
	// if SD2 read OCR register to check for SDHC card
	if (cardCommand(CMD58, 0)) {
		deselectCard();
		return false;
	}
	//discard OCR reg

	SPI_Recieve(buffer, 4);
	deselectCard();
	_spi->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4; //speed back up
	HAL_SPI_Init((SPI_HandleTypeDef*) _spi); //apply the speed change
	return true;
}

uint8_t SDCard::cardCommand(uint8_t command, uint32_t arg) {
	uint8_t res = 0xFF;
	/*HAL_SPI_Transmit(_spi, &res, 1, 100);
	 HAL_SPI_Transmit(_spi, &res, 1, 100);
	 HAL_SPI_Transmit(_spi, &res, 1, 100);
	 HAL_SPI_Transmit(_spi, &res, 1, 100);
	 HAL_SPI_Transmit(_spi, &res, 1, 100);
	 HAL_SPI_Transmit(_spi, &res, 1, 100);
	 HAL_SPI_Transmit(_spi, &res, 1, 100);*/

	waitUntilReady(); //wait for card to no longer be busy

	selectCard();
	uint8_t commandSequence[] = { (uint8_t) (command | 0x40), (uint8_t) (arg
			>> 24), (uint8_t) (arg >> 16), (uint8_t) (arg >> 8), (uint8_t) (arg
			& 0xFF), 0xFF };
	if (command == CMD0)
		commandSequence[5] = 0x95;
	else if (command == CMD8)
		commandSequence[5] = 0x87;
	HAL_SPI_Transmit((SPI_HandleTypeDef*) _spi, commandSequence, 6, 100);
	//Data sent, now await Response
	uint8_t count = 20;
	while ((res & 0x80) && count) {
		SPI_Recieve(&res, 1);
		count--;
	}
	return res;
}

void SDCard::selectCard() {
	HAL_GPIO_WritePin(_cs1Port, _cs1Pin, GPIO_PIN_RESET);
}

HAL_StatusTypeDef SDCard::SPI_Recieve(uint8_t* pData, uint16_t Size) {

	HAL_StatusTypeDef errorcode = HAL_OK;

	/* Process Locked */
	__HAL_LOCK(_spi);

	/* Don't overwrite in case of HAL_SPI_STATE_BUSY_RX */
	if (_spi->State == HAL_SPI_STATE_READY) {
		_spi->State = HAL_SPI_STATE_BUSY_TX_RX;
	}

	/* Set the transaction information */
	_spi->ErrorCode = HAL_SPI_ERROR_NONE;
	_spi->pRxBuffPtr = (uint8_t *) pData;
	_spi->RxXferCount = Size;
	_spi->RxXferSize = Size;
	_spi->pTxBuffPtr = NULL;
	_spi->TxXferCount = Size;
	_spi->TxXferSize = Size;

	/*Init field not used in handle to zero */
	_spi->RxISR = NULL;
	_spi->TxISR = NULL;
	/* Check if the SPI is already enabled */
	if ((_spi->Instance->CR1 & SPI_CR1_SPE) != SPI_CR1_SPE) {
		/* Enable SPI peripheral */
		__HAL_SPI_ENABLE(_spi);
	}
	/* Transmit and Receive data in 8 Bit mode */

	while ((_spi->RxXferCount > 0U)) {
		/* check TXE flag */
		if ((_spi->TxXferCount > 0U)) {
			while (!__HAL_SPI_GET_FLAG(_spi, SPI_FLAG_TXE))
				;
			*(__IO uint8_t *) &_spi->Instance->DR = 0xFF;
			_spi->TxXferCount--;

		}

		/* Wait until RXNE flag is reset */
		if ((_spi->RxXferCount > 0U)) {
			while (!(__HAL_SPI_GET_FLAG(_spi, SPI_FLAG_RXNE)))
				;
			(*(uint8_t *) pData++) = _spi->Instance->DR;
			_spi->RxXferCount--;
		}

	}

	if (lSPI_WaitFlagStateUntilTimeout((SPI_HandleTypeDef*) _spi, SPI_FLAG_BSY,
			RESET, 100, HAL_GetTick()) != HAL_OK) {
		_spi->ErrorCode |= HAL_SPI_ERROR_FLAG;

		errorcode = HAL_TIMEOUT;
	}

	_spi->State = HAL_SPI_STATE_READY;
	__HAL_UNLOCK(_spi);
	return errorcode;
}

void SDCard::deselectCard() {
	HAL_GPIO_WritePin(_cs1Port, _cs1Pin, GPIO_PIN_SET);
}

/**
 * @brief Handle SPI Communication Timeout.
 * @param hspi: pointer to a SPI_HandleTypeDef structure that contains
 *              the configuration information for SPI module.
 * @param Flag: SPI flag to check
 * @param State: flag state to check
 * @param Timeout: Timeout duration
 * @param Tickstart: tick start value
 * @retval HAL status
 */
HAL_StatusTypeDef SDCard::lSPI_WaitFlagStateUntilTimeout(
		SPI_HandleTypeDef *hspi, uint32_t Flag, uint32_t State,
		uint32_t Timeout, uint32_t Tickstart) {
	while ((hspi->Instance->SR & Flag) != State) {
		if (Timeout != HAL_MAX_DELAY) {
			if ((Timeout == 0U) || ((HAL_GetTick() - Tickstart) >= Timeout)) {
				/* Disable the SPI and reset the CRC: the CRC value should be cleared
				 on both master and slave sides in order to resynchronize the master
				 and slave for their respective CRC calculation */

				/* Disable TXE, RXNE and ERR interrupts for the interrupt process */
				__HAL_SPI_DISABLE_IT(hspi,
						(SPI_IT_TXE | SPI_IT_RXNE | SPI_IT_ERR));

				if ((hspi->Init.Mode == SPI_MODE_MASTER)
						&& ((hspi->Init.Direction == SPI_DIRECTION_1LINE)
								|| (hspi->Init.Direction
										== SPI_DIRECTION_2LINES_RXONLY))) {
					/* Disable SPI peripheral */
					__HAL_SPI_DISABLE(hspi);
				}

				/* Reset CRC Calculation */
				if (hspi->Init.CRCCalculation == SPI_CRCCALCULATION_ENABLE) {
					SPI_RESET_CRC(hspi);
				}

				hspi->State = HAL_SPI_STATE_READY;

				/* Process Unlocked */
				__HAL_UNLOCK(hspi);

				return HAL_TIMEOUT;
			}
		}
	}

	return HAL_OK;
}
