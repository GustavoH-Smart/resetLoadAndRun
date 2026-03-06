/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_usbx_host.c
  * @author  MCD Application Team
  * @brief   USBX host applicative file
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
#include "app_usbx_host.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ux_host_cdc_acm.h"

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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

extern HCD_HandleTypeDef hhcd_USB_OTG_HS2;

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */
/* CDC ACM Host callback */
static UINT cdc_acm_host_callback(ULONG event, UX_HOST_CLASS *host_class, VOID *instance);

/**
  * @brief  Application USBX Host Initialization.
  * @param  none
  * @retval status
  */
UINT MX_USBX_Host_Init(VOID)
{
  UINT ret = UX_SUCCESS;

  /* USER CODE BEGIN MX_USBX_Host_Init0 */
  /* USER CODE END MX_USBX_Host_Init0 */
  /* Initialize the Stack Host USB*/
  if (MX_USBX_Host_Stack_Init() != UX_SUCCESS)
  {
    /* USER CODE BEGIN MAIN_INITIALIZE_STACK_ERROR */
    return UX_ERROR;
    /* USER CODE END MAIN_INITIALIZE_STACK_ERROR */
  }

  /* USER CODE BEGIN MX_USBX_Host_Init 1 */

  /* USER CODE END MX_USBX_Host_Init 1 */

  /* USER CODE BEGIN MX_USBX_Host_Init 2 */

  /* USER CODE END MX_USBX_Host_Init 2 */

  return ret;
}

/**
  * @brief MX_USBX_Host_Stack_Init
  *        Initialization of USB host stack.
  *        Init USB Host stack, add register the host class stack
  * @retval None
  */
UINT MX_USBX_Host_Stack_Init(void)
{
  UINT ret = UX_SUCCESS;
  /* USER CODE BEGIN MX_USBX_Host_Stack_Init_PreTreatment_0 */
  /* Prevent HAL host callbacks from firing before USBX host context is ready. */
  HAL_NVIC_DisableIRQ(USB2_OTG_HS_IRQn);
  /* USER CODE END MX_USBX_Host_Stack_Init_PreTreatment_0 */

  /* Initialize host stack */
  printf("[USBX-HOST] ux_host_stack_initialize...\r\n");
  if (ux_host_stack_initialize(cdc_acm_host_callback) != UX_SUCCESS)
  {
    printf("[USBX-HOST] ux_host_stack_initialize FAILED\r\n");
    HAL_NVIC_EnableIRQ(USB2_OTG_HS_IRQn);
    return UX_ERROR;
  }

  /* Register HCD controller */
  printf("[USBX-HOST] ux_host_stack_hcd_register...\r\n");
  if (ux_host_stack_hcd_register(_ux_system_host_hcd_stm32_name,
                                  ux_hcd_stm32_initialize,
                                  USB2_OTG_HS_BASE,
                                  (ULONG)&hhcd_USB_OTG_HS2) != UX_SUCCESS)
  {
    printf("[USBX-HOST] ux_host_stack_hcd_register FAILED\r\n");
    HAL_NVIC_EnableIRQ(USB2_OTG_HS_IRQn);
    return UX_ERROR;
  }

  /* Register CDC ACM class */
  printf("[USBX-HOST] ux_host_stack_class_register...\r\n");
  if (ux_host_stack_class_register(_ux_system_host_class_cdc_acm_name, ux_host_class_cdc_acm_entry) != UX_SUCCESS)
  {
    printf("[USBX-HOST] ux_host_stack_class_register FAILED\r\n");
    HAL_NVIC_EnableIRQ(USB2_OTG_HS_IRQn);
    return UX_ERROR;
  }

  printf("[USBX-HOST] host stack init OK\r\n");
  HAL_NVIC_EnableIRQ(USB2_OTG_HS_IRQn);

  /* USER CODE BEGIN MX_USBX_Host_Stack_Init_PreTreatment_1 */
  /* USER CODE END MX_USBX_Host_Stack_Init_PreTreatment_1 */

  /* USER CODE BEGIN MX_USBX_Host_Stack_Init_PostTreatment */
  /* USER CODE END MX_USBX_Host_Stack_Init_PostTreatment */
  return ret ;
}

/**
  * @brief  MX_USBX_Host_Stack_DeInit
  *         Uninitialize of USB Host stack.
  *         Uninitialize the host stack, unregister of host class stack and
  *         unregister of the usb host controllers
  * @retval None
  */
UINT MX_USBX_Host_Stack_DeInit(void)
{
  UINT ret = UX_SUCCESS;

  /* USER CODE BEGIN MX_USBX_Host_Stack_DeInit_PreTreatment_0 */

  /* USER CODE END MX_USBX_Host_Stack_DeInit_PreTreatment_0 */

  /* Unregister all the USB host controllers available in this system. */
  ux_host_stack_hcd_unregister(_ux_system_host_hcd_stm32_name,
                               USB2_OTG_HS_BASE,
                               (ULONG)&hhcd_USB_OTG_HS2);

  /* The code below is required for uninstalling the host portion of USBX.  */
  if (ux_host_stack_uninitialize() != UX_SUCCESS)
  {
    return UX_ERROR;
  }

  /* USER CODE BEGIN MX_USBX_Host_Stack_DeInit_PreTreatment_1 */
  /* USER CODE END MX_USBX_Host_Stack_DeInit_PreTreatment_1 */

  /* USER CODE BEGIN MX_USBX_Host_Stack_DeInit_PostTreatment */
  /* USER CODE END MX_USBX_Host_Stack_DeInit_PostTreatment */
  return ret ;
}
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/**
  * @brief  CDC ACM Host callback
  * @param  event: Event type (UX_DEVICE_INSERTION or UX_DEVICE_REMOVAL)
  * @param  host_class: Pointer to host class
  * @param  instance: Pointer to class instance
  * @retval UX_SUCCESS
  */
static UINT cdc_acm_host_callback(ULONG event, UX_HOST_CLASS *host_class, VOID *instance)
{
  UX_HOST_CLASS_CDC_ACM *cdc_acm = (UX_HOST_CLASS_CDC_ACM *)instance;

  /* Check event type */
  if (event == UX_DEVICE_INSERTION)
  {
    /* Device inserted - activate */
    if (cdc_acm != UX_NULL)
    {
      ux_host_cdc_acm_instance_activate(instance);
    }
  }
  else if (event == UX_DEVICE_REMOVAL)
  {
    /* Device removed - deactivate */
    ux_host_cdc_acm_instance_deactivate(instance);
  }

  return UX_SUCCESS;
}
