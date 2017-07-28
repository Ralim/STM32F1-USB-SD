/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H
/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "stdbool.h"
#include "math.h"

#define byte uint8_t
/* Private define ------------------------------------------------------------*/

#define SD_CS_Pin GPIO_PIN_4
#define SD_CS_GPIO_Port GPIOA
#define SD_SCK_Pin GPIO_PIN_5
#define SD_SCK_GPIO_Port GPIOA
#define SD_MISO_Pin GPIO_PIN_6
#define SD_MISO_GPIO_Port GPIOA
#define SD_MOSI_Pin GPIO_PIN_7
#define SD_MOSI_GPIO_Port GPIOA

#ifdef __cplusplus
extern "C" {

#endif
bool SDCardWriteSect(uint8_t* bufferOut, uint32_t sector, uint16_t count);
bool SDCardReadSect(uint8_t* bufferOut, uint32_t sector, uint16_t count);
uint32_t SDCardSectorCount();
#ifdef __cplusplus
}
#endif
#endif /* __MAIN_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
