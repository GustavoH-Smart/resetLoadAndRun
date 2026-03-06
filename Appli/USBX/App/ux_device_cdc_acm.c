/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_device_cdc_acm.c
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
#include "ux_device_cdc_acm.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
/* Global CDC ACM device instance */
static UX_SLAVE_CLASS_CDC_ACM *g_cdc_acm_device = NULL;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */
/* CDC ACM Device Callbacks */
VOID ux_device_cdc_acm_instance_activate(VOID *cdc_acm_instance);
VOID ux_device_cdc_acm_instance_deactivate(VOID *cdc_acm_instance);

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * @brief  CDC ACM Instance Activate callback
  * @param  cdc_acm_instance: pointer to CDC ACM instance
  * @retval None
  */
VOID ux_device_cdc_acm_instance_activate(VOID *cdc_acm_instance)
{
  g_cdc_acm_device = (UX_SLAVE_CLASS_CDC_ACM *)cdc_acm_instance;
}

/**
  * @brief  CDC ACM Instance Deactivate callback
  * @param  cdc_acm_instance: pointer to CDC ACM instance
  * @retval None
  */
VOID ux_device_cdc_acm_instance_deactivate(VOID *cdc_acm_instance)
{
  g_cdc_acm_device = NULL;
}

/**
  * @brief  Get CDC ACM device instance
  * @retval pointer to CDC ACM instance or NULL
  */
UX_SLAVE_CLASS_CDC_ACM *ux_device_cdc_acm_get_instance(void)
{
  return g_cdc_acm_device;
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
/* Export activation/deactivation callbacks */
UX_SLAVE_CLASS_CDC_ACM_PARAMETER cdc_acm_parameter = {
  .ux_slave_class_cdc_acm_instance_activate = ux_device_cdc_acm_instance_activate,
  .ux_slave_class_cdc_acm_instance_deactivate = ux_device_cdc_acm_instance_deactivate
};

