/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_usbx_device.c
  * @author  MCD Application Team
  * @brief   USBX Device applicative file
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

/* Includes ------------------------------------------------------------------*/
#include "app_usbx_device.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ux_device_descriptors.h"
#include "ux_device_cdc_acm.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
extern PCD_HandleTypeDef           hpcd_USB_OTG_HS1;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/**
  * @brief  Application USBX Device Initialization.
  * @param  none
  * @retval status
  */

UINT MX_USBX_Device_Init(VOID)
{
  UINT ret = UX_SUCCESS;
  /* USER CODE BEGIN MX_USBX_Device_Init 0 */
  /* USER CODE END MX_USBX_Device_Init 0 */

  /* Initialize the Stack USB Device*/
  if (MX_USBX_Device_Stack_Init() != UX_SUCCESS)
  {
    /* USER CODE BEGIN MAIN_INITIALIZE_STACK_ERROR */
    return UX_ERROR;
    /* USER CODE END MAIN_INITIALIZE_STACK_ERROR */
  }

  /* USER CODE BEGIN MX_USBX_Device_Init 1 */
  /* USER CODE END MX_USBX_Device_Init 1 */

  /* USER CODE BEGIN MX_USBX_Device_Init 2 */
  /* USER CODE END MX_USBX_Device_Init 2 */

  return ret;
}

/**
  * @brief  Application USBX Device Initialization.
  * @param  None
  * @retval ret
  */
UINT MX_USBX_Device_Stack_Init(void)
{
  UINT ret = UX_SUCCESS;

  /* USER CODE BEGIN MX_USBX_Device_Stack_Init 0 */

  /* USER CODE END MX_USBX_Device_Stack_Init 0 */

  /* Avoid IRQ race while USBX device structures are being created. */
  HAL_NVIC_DisableIRQ(USB1_OTG_HS_IRQn);
  /* USER CODE BEGIN MX_USBX_Device_Stack_Init_PostTreatment */
  
  /* Get device framework descriptors */
  ULONG hs_length, fs_length, string_length, language_length;
  UCHAR *device_framework_high_speed = USBD_Get_Device_Framework_Speed(USBD_HIGH_SPEED, &hs_length);
  UCHAR *device_framework_full_speed = USBD_Get_Device_Framework_Speed(USBD_FULL_SPEED, &fs_length);
  UCHAR *string_framework = USBD_Get_String_Framework(&string_length);
  UCHAR *language_id_framework = USBD_Get_Language_Id_Framework(&language_length);

  if ((device_framework_high_speed == UX_NULL) || (hs_length == 0U) ||
      (device_framework_full_speed == UX_NULL) || (fs_length == 0U) ||
      (string_framework == UX_NULL) || (string_length == 0U) ||
      (language_id_framework == UX_NULL) || (language_length == 0U))
  {
    HAL_NVIC_EnableIRQ(USB1_OTG_HS_IRQn);
    return UX_ERROR;
  }

  /* Initialize the USB device stack */
  if (ux_device_stack_initialize(device_framework_high_speed, hs_length,
                                 device_framework_full_speed, fs_length,
                                 string_framework, string_length,
                                 language_id_framework, language_length,
                                 UX_NULL) != UX_SUCCESS)
  {
    HAL_NVIC_EnableIRQ(USB1_OTG_HS_IRQn);
    return UX_ERROR;
  }

  /* Register CDC ACM class */
  extern UX_SLAVE_CLASS_CDC_ACM_PARAMETER cdc_acm_parameter;
  if (ux_device_stack_class_register(_ux_system_slave_class_cdc_acm_name,
                                     _ux_device_class_cdc_acm_entry,
                                     1, 0,
                                     &cdc_acm_parameter) != UX_SUCCESS)
  {
    HAL_NVIC_EnableIRQ(USB1_OTG_HS_IRQn);
    return UX_ERROR;
  }

  /* Initialize and link controller HAL driver after stack setup. */
  if (ux_dcd_stm32_initialize((ULONG)USB1_OTG_HS, (ULONG)&hpcd_USB_OTG_HS1) != UX_SUCCESS)
  {
    HAL_NVIC_EnableIRQ(USB1_OTG_HS_IRQn);
    return UX_ERROR;
  }

  HAL_NVIC_EnableIRQ(USB1_OTG_HS_IRQn);
  
  /* USER CODE END MX_USBX_Device_Stack_Init_PostTreatment */

  /* USER CODE BEGIN MX_USBX_Device_Stack_Init 1 */

  /* USER CODE END MX_USBX_Device_Stack_Init 1 */

  return ret;
}

/**
  * @brief MX_USBX_Device_Stack_DeInit
  *        Unitialization of USB Device.
  * uninitialize the device stack, unregister of device class stack
  * unregister of the usb device controller
  * @retval None
  */
UINT MX_USBX_Device_Stack_DeInit(void)
{
  UINT ret = UX_SUCCESS;

  /* USER CODE BEGIN MX_USBX_Device_Stack_DeInit_PreTreatment_0 */
  /* USER CODE END MX_USBX_Device_Stack_DeInit_PreTreatment_0 */

  /* Unregister USB device controller. */

  if (_ux_dcd_stm32_uninitialize((ULONG)USB1_OTG_HS, (ULONG)&hpcd_USB_OTG_HS1) != UX_SUCCESS)
  {
    return UX_ERROR;
  }

  /* The code below is required for uninstalling the device portion of USBX.  */
  if (ux_device_stack_uninitialize() != UX_SUCCESS)
  {
    return UX_ERROR;
  }

  /* USER CODE BEGIN MX_USBX_Device_Stack_DeInit_PreTreatment_1 */
  /* USER CODE END MX_USBX_Device_Stack_DeInit_PreTreatment_1 */

  /* USER CODE BEGIN MX_USBX_Device_Stack_DeInit_PostTreatment */
  /* USER CODE END MX_USBX_Device_Stack_DeInit_PostTreatment */

  return ret;
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
