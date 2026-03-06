/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_usbx.c
  * @author  MCD Application Team
  * @brief   USBX applicative file
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
#include "app_usbx.h"
#include "ux_device_cdc_acm.h"
#include "ux_host_cdc_acm.h"

/* USBX system memory pool (required by ux_system_initialize). */
static UCHAR usbx_memory_pool[USBX_APP_MEM_POOL_SIZE];
/* Cache-safe pool for USB DMA access. Linker maps .noncacheable in RAM. */
#if defined(__GNUC__)
__attribute__((section(".noncacheable"), aligned(32)))
#endif
static UCHAR usbx_cache_safe_pool[8 * 1024];

/* Prevent double initialization when multiple tasks call MX_USBX_Init. */
static UINT usbx_initialized = 0U;

/**
  * @brief  Application USBX Initialization.
  * @param  none
  * @retval status
  */
UINT MX_USBX_Init(VOID)
{
  UINT ret = UX_SUCCESS;

  if (usbx_initialized != 0U)
  {
    return UX_SUCCESS;
  }

  /* USER CODE BEGIN MX_USBX_Init0 */

  /* USER CODE END MX_USBX_Init0 */

  ret = ux_system_initialize(usbx_memory_pool,
                             sizeof(usbx_memory_pool),
                             usbx_cache_safe_pool,
                             sizeof(usbx_cache_safe_pool));
  if(ret != UX_SUCCESS)
  {
    printf("USBX System Init Error: %u\n", ret);
    return ret;
  }

  ret = MX_USBX_Device_Init();
  if(ret != UX_SUCCESS)
  {
    printf("USBX Device Init Error: %u\n", ret);
    return ret;
  }

  // ret = MX_USBX_Host_Init();
  // if(ret != UX_SUCCESS)
  // {
  //   printf("USBX Host Init Error: %u\n", ret);
  //   return ret;
  // }

  /* USER CODE BEGIN MX_USBX_Init1 */

  usbx_initialized = 1U;

  /* USER CODE END MX_USBX_Init1 */

  return ret;
}
/**
  * @brief  _ux_utility_interrupt_disable
  *         USB utility interrupt disable.
  * @param  none
  * @retval none
  */
ALIGN_TYPE _ux_utility_interrupt_disable(VOID)
{
  UINT interrupt_save = 0;
  /* USER CODE BEGIN _ux_utility_interrupt_disable */
  interrupt_save = __get_PRIMASK();
  __disable_irq();
  /* USER CODE END _ux_utility_interrupt_disable */

  return interrupt_save;
}

/**
  * @brief  _ux_utility_interrupt_restore
  *         USB utility interrupt restore.
  * @param  flags
  * @retval none
  */
VOID _ux_utility_interrupt_restore(ALIGN_TYPE flags)
{

  /* USER CODE BEGIN _ux_utility_interrupt_restore */
  __set_PRIMASK(flags);
  /* USER CODE END _ux_utility_interrupt_restore */
}

/**
  * @brief  _ux_utility_time_get
  *         Get Time Tick for host timing.
  * @param  none
  * @retval time tick
  */
ULONG _ux_utility_time_get(VOID)
{
  ULONG time_tick = 0U;

  /* USER CODE BEGIN _ux_utility_time_get */
  time_tick = HAL_GetTick();

  /* USER CODE END _ux_utility_time_get */

  return time_tick;
}

/* USER CODE BEGIN 1 */

/* ── USB Device Helper Functions ────────────────────────────────────── */
/**
  * @brief  Send data via USB Device CDC ACM
  * @param  buffer: pointer to data buffer
  * @param  length: data length
  * @retval UX_SUCCESS if OK, UX_ERROR otherwise
  */
UINT usb_device_send(uint8_t *buffer, uint32_t length)
{
  UX_SLAVE_CLASS_CDC_ACM *cdc_acm = ux_device_cdc_acm_get_instance();
  ULONG actual_length = 0;
  UINT status;

  if (cdc_acm == UX_NULL)
    return UX_ERROR;

#if defined(UX_DEVICE_STANDALONE)
  do
  {
    status = ux_device_class_cdc_acm_write_run(cdc_acm, buffer, length, &actual_length);
  } while (status == UX_STATE_WAIT);
  return status;
#else
  return ux_device_class_cdc_acm_write(cdc_acm, buffer, length, &actual_length);
#endif
}

/**
  * @brief  Receive data via USB Device CDC ACM
  * @param  buffer: pointer to data buffer
  * @param  length: max data length
  * @param  timeout_ms: timeout in milliseconds (not used currently)
  * @retval number of bytes received
  */
UINT usb_device_receive(uint8_t *buffer, uint32_t max_length, uint32_t timeout_ms)
{
  UX_SLAVE_CLASS_CDC_ACM *cdc_acm = ux_device_cdc_acm_get_instance();
  ULONG actual_length = 0;
  UINT status;

  (void)timeout_ms;

  if (cdc_acm == UX_NULL)
    return 0;

#if defined(UX_DEVICE_STANDALONE)
  do
  {
    status = ux_device_class_cdc_acm_read_run(cdc_acm, buffer, max_length, &actual_length);
  } while (status == UX_STATE_WAIT);

  if (status == UX_SUCCESS)
    return (UINT)actual_length;
#else
  if (ux_device_class_cdc_acm_read(cdc_acm, buffer, max_length, &actual_length) == UX_SUCCESS)
    return actual_length;
#endif

  return 0;
}

/**
  * @brief  Check if USB Device is connected
  * @retval UX_TRUE if connected, UX_FALSE otherwise
  */
UINT usb_device_is_connected(void)
{
  UX_SLAVE_CLASS_CDC_ACM *cdc_acm = ux_device_cdc_acm_get_instance();

  if (cdc_acm == UX_NULL)
    return UX_FALSE;

  /*
   * CDC instance is set on USBX activate callback (after host configures the
   * device). DTR depends on the host serial app and can stay low even when the
   * cable is connected and enumeration is successful.
   */
  return UX_TRUE;
}

/* ── USB Host Helper Functions ──────────────────────────────────────── */
/**
  * @brief  Send data via USB Host CDC ACM
  * @param  buffer: pointer to data buffer
  * @param  length: data length
  * @retval UX_SUCCESS if OK, UX_ERROR otherwise
  */
UINT usb_host_send(uint8_t *buffer, uint32_t length)
{
  UX_HOST_CLASS_CDC_ACM *cdc_acm = ux_host_cdc_acm_get_instance();
  ULONG actual_length = 0;
  
  if (cdc_acm == UX_NULL)
    return UX_ERROR;
    
  return ux_host_class_cdc_acm_write(cdc_acm, buffer, length, &actual_length);
}

/**
  * @brief  Receive data via USB Host CDC ACM
  * @param  buffer: pointer to data buffer
  * @param  length: max data length
  * @retval number of bytes received
  */
UINT usb_host_receive(uint8_t *buffer, uint32_t max_length)
{
  UX_HOST_CLASS_CDC_ACM *cdc_acm = ux_host_cdc_acm_get_instance();
  ULONG actual_length = 0;
  
  if (cdc_acm == UX_NULL)
    return 0;
    
  if (ux_host_class_cdc_acm_read(cdc_acm, buffer, max_length, &actual_length) == UX_SUCCESS)
    return actual_length;
    
  return 0;
}

/**
  * @brief  Check if USB Host device is connected
  * @retval UX_TRUE if connected, UX_FALSE otherwise
  */
UINT usb_host_is_connected(void)
{
  UX_HOST_CLASS_CDC_ACM *cdc_acm = ux_host_cdc_acm_get_instance();
  
  return (cdc_acm != UX_NULL) ? UX_TRUE : UX_FALSE;
}

/* USER CODE END 1 */
