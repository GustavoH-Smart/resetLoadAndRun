/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_usbx.h
  * @author  MCD Application Team
  * @brief   USBX applicative header file
  ******************************************************************************
   * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_USBX_H__
#define __APP_USBX_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ux_api.h"

#include "app_usbx_device.h"
#include "app_usbx_host.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
#define USBX_APP_MEM_POOL_SIZE       (32 * 1024)
#define USBX_MEMORY_STACK_SIZE       1024

/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported functions prototypes ---------------------------------------------*/
UINT MX_USBX_Init(VOID);

/* USB Device Helper Functions */
UINT usb_device_send(uint8_t *buffer, uint32_t length);
UINT usb_device_receive(uint8_t *buffer, uint32_t max_length, uint32_t timeout_ms);
UINT usb_device_is_connected(void);

/* USB Host Helper Functions */
UINT usb_host_send(uint8_t *buffer, uint32_t length);
UINT usb_host_receive(uint8_t *buffer, uint32_t max_length);
UINT usb_host_is_connected(void);

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

#ifdef __cplusplus
}
#endif
#endif /* __APP_USBX_H__ */
