/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_host_cdc_acm.c
  * @author  MCD Application Team
  * @brief   USBX Host applicative file
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
#include "ux_host_cdc_acm.h"

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
/* Global CDC ACM host instance */
static UX_HOST_CLASS_CDC_ACM *g_cdc_acm_host = NULL;
/* CDC ACM Host Callbacks */
VOID ux_host_cdc_acm_instance_activate(VOID *cdc_acm_instance);
VOID ux_host_cdc_acm_instance_deactivate(VOID *cdc_acm_instance);

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * @brief  CDC ACM Host Instance Activate callback
  * @param  cdc_acm_instance: pointer to CDC ACM instance
  * @retval None
  */
VOID ux_host_cdc_acm_instance_activate(VOID *cdc_acm_instance)
{
  g_cdc_acm_host = (UX_HOST_CLASS_CDC_ACM *)cdc_acm_instance;
}

/**
  * @brief  CDC ACM Host Instance Deactivate callback
  * @param  cdc_acm_instance: pointer to CDC ACM instance
  * @retval None
  */
VOID ux_host_cdc_acm_instance_deactivate(VOID *cdc_acm_instance)
{
  g_cdc_acm_host = NULL;
}

/**
  * @brief  Get CDC ACM host instance
  * @retval pointer to CDC ACM instance or NULL
  */
UX_HOST_CLASS_CDC_ACM *ux_host_cdc_acm_get_instance(void)
{
  return g_cdc_acm_host;
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

