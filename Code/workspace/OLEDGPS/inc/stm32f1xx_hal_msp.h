/*
 * stm32f1xx_hal_msp.h
 *
 *  Created on: 20 Apr 2017
 *      Author: ralim
 */

#ifndef STM32F1XX_HAL_MSP_H_
#define STM32F1XX_HAL_MSP_H_
#ifdef __cplusplus
 extern "C" {
#endif
void HAL_MspInit(void);
void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c);
void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c);
void HAL_RTC_MspInit(RTC_HandleTypeDef* hrtc);
void HAL_RTC_MspDeInit(RTC_HandleTypeDef* hrtc);
void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi);
void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi);
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base);
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef* htim_pwm);
void HAL_TIM_Encoder_MspInit(TIM_HandleTypeDef* htim_encoder);
void HAL_TIM_MspPostInit(TIM_HandleTypeDef* htim);
void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base);
void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef* htim_pwm);
void HAL_TIM_Encoder_MspDeInit(TIM_HandleTypeDef* htim_encoder);
void HAL_UART_MspInit(UART_HandleTypeDef* huart);
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart);
#ifdef __cplusplus
 }
#endif
#endif /* STM32F1XX_HAL_MSP_H_ */
