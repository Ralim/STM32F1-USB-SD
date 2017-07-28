/*
 * setup.h
 *
 *  Created on: 20 Apr 2017
 *      Author: ralim
 */

#ifndef SETUP_H_
#define SETUP_H_
#include "main.h"
#include "stm32f1xx_hal_msp.h"
#ifdef __cplusplus
 extern "C" {
#endif
extern I2C_HandleTypeDef hi2c1;

extern RTC_HandleTypeDef hrtc;

extern volatile SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;
extern DMA_HandleTypeDef hdma_spi2_tx;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;

extern UART_HandleTypeDef huart3;
extern DMA_HandleTypeDef hdma_usart3_rx;

void SystemClock_Config(void);
void Error_Handler(void);
void MX_GPIO_Init(void);
void MX_DMA_Init(void);
void MX_RTC_Init(void);
void MX_I2C1_Init(void);
void MX_SPI1_Init(void);
void MX_SPI2_Init(void);
void MX_TIM3_Init(void);
void MX_USART3_UART_Init(void);
void MX_TIM2_Init(void);
void MX_TIM1_Init(void);
void setup(void);

#ifdef __cplusplus
 }
#endif
#endif /* SETUP_H_ */
