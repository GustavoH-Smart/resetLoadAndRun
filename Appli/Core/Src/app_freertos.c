/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : FreeRTOS applicative file
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
#include "app_freertos.h"
#include "app_usbx.h"
#include <stdio.h>
#include <string.h>

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
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 1024 * 4
};

/* Host task kept separate to control stack/priority explicitly. */
osThreadId_t usbHostTaskHandle;
const osThreadAttr_t usbHostTask_attributes = {
  .name = "usbHostTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 1024 * 6
};

osThreadId_t usbDeviceTaskHandle;
const osThreadAttr_t usbDeviceTask_attributes = {
  .name = "usbDeviceTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 1024 * 6
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

/* ── USB Device Test Task ──────────────────────────────────────────────── */
/**
  * @brief  USB Device test task - receives data and sends echo back
  * @param  argument: Not used
  * @retval None
  */
void usb_device_test_task(void *argument)
{
  UINT ux_ret;
  uint8_t usbx_ready = 0;
  uint8_t rx_buf[256];
  uint8_t tx_buf[256];
  uint32_t rx_len = 0;
  static uint32_t counter = 0;
  static uint8_t was_connected = 0;
  uint32_t tick_count = 0;
  uint32_t wait_log_div = 0;
  (void)argument;

  printf("[RTOS] usb device task started\r\n");
  osDelay(1000); /* Let clocks/peripherals settle before USBX init. */

  printf("[RTOS] calling MX_USBX_Init()\r\n");
  ux_ret = MX_USBX_Init();
  if (ux_ret == UX_SUCCESS) {
    usbx_ready = 1;
    printf("[RTOS] USBX Initialized\r\n");
  } else {
    printf("[RTOS] USBX Init Failed: %u\r\n", ux_ret);
  }
  
  while (1)
  {
#if defined(UX_STANDALONE)
    if (usbx_ready)
    {
      ux_device_stack_tasks_run();
    }
    else
    {
      osDelay(1);
      continue;
    }
#endif

    uint8_t connected = usb_device_is_connected();
    
    /* Log connection state transitions */
    if (connected && !was_connected) {
      printf("[USB_DEV] ✓ Host connected\n");
      was_connected = 1;
    } else if (!connected && was_connected) {
      printf("[USB_DEV] ✗ Host disconnected\n");
      was_connected = 0;
    }

    if(!connected) {
      if ((wait_log_div++ % 1000U) == 0U) {
        extern volatile uint32_t usb1_irq_count;
        extern PCD_HandleTypeDef hpcd_USB_OTG_HS1;
        USB_OTG_GlobalTypeDef *USBx = hpcd_USB_OTG_HS1.Instance;
        uint32_t gahbcfg = USBx->GAHBCFG;
        uint32_t gintmsk = USBx->GINTMSK;
        uint32_t gintsts = USBx->GINTSTS;
        uint32_t grxfsiz = USBx->GRXFSIZ;
        uint32_t gnptxfsiz = USBx->DIEPTXF0_HNPTXFSIZ;
        uint32_t dsts = ((USB_OTG_DeviceTypeDef *)((uint32_t)USBx + 0x800UL))->DSTS;
        uint32_t dctl = ((USB_OTG_DeviceTypeDef *)((uint32_t)USBx + 0x800UL))->DCTL;
        printf("[USB_DEV] IRQs=%lu GAHBCFG=0x%08lX GINTMSK=0x%08lX GINTSTS=0x%08lX\n",
               usb1_irq_count, gahbcfg, gintmsk, gintsts);
        printf("[USB_DEV] GRXFSIZ=0x%04lX GNPTXFSIZ=0x%08lX DSTS=0x%08lX DCTL=0x%08lX\n",
               grxfsiz, gnptxfsiz, dsts, dctl);
      }
      /*
       * In standalone mode USBX tasks must run frequently for EP0 control
       * transfers during enumeration.
       */
      osDelay(1);
      continue;
    }

    /* Always try to receive data */
    rx_len = usb_device_receive(rx_buf, sizeof(rx_buf), 200);
    if (rx_len > 0)
    {
      printf("[USB_DEV] RX [%ld bytes]: %.*s\n", rx_len, (int)rx_len, (char*)rx_buf);

      /* Echo data back to host */
      if (usb_device_send(rx_buf, rx_len) == UX_SUCCESS) {
        printf("[USB_DEV] ✓ Echo sent [%ld bytes]\n", rx_len);
      }
    }

    /* Send test message periodically (only when connected) */
    if (connected && (tick_count++ % 10 == 0)) {
      counter++;
      int len = snprintf((char*)tx_buf, sizeof(tx_buf), "[DEV] Test message %ld\r\n", counter);
      if (usb_device_send(tx_buf, len) == UX_SUCCESS) {
        printf("[USB_DEV] ✓ TX: %s", (char*)tx_buf);
      }
    }

    osDelay(1);
  }
}

/* ── USB Host Test Task ────────────────────────────────────────────────── */
/**
  * @brief  USB Host test task - sends data and receives responses
  * @param  argument: Not used
  * @retval None
  */
void usb_host_test_task(void *argument)
{
  UINT ux_ret;
  uint8_t usbx_ready = 0;
  uint8_t rx_buf[256];
  uint8_t tx_buf[256];
  uint32_t rx_len = 0;
  static uint32_t counter = 0;
  static uint8_t was_connected = 0;

  (void)argument;

  printf("[RTOS] usb host task started\r\n");
  osDelay(1000); /* Let clocks/peripherals settle before USBX init. */

  printf("[RTOS] calling MX_USBX_Init()\r\n");
  ux_ret = MX_USBX_Init();
  if (ux_ret == UX_SUCCESS) {
    usbx_ready = 1;
    printf("[RTOS] USBX Initialized\r\n");
  } else {
    printf("[RTOS] USBX Init Failed: %u\r\n", ux_ret);
  }
  
  while (1)
  {
#if defined(UX_STANDALONE)
    if (usbx_ready)
    {
      ux_host_stack_tasks_run();
    }
    else
    {
      osDelay(1);
      continue;
    }
#endif

    uint8_t connected = usb_host_is_connected();
    
    /* Log connection state transitions */
    if (connected && !was_connected) {
      printf("[USB_HOST] ✓ Device connected\n");
      was_connected = 1;
    } else if (!connected && was_connected) {
      printf("[USB_HOST] ✗ Device disconnected\n");
      was_connected = 0;
    }

    if (!connected) {
      printf("[USB_HOST] Waiting for device...\n");
      osDelay(1000);
      continue;
    }

    /* Send test message to device */
    // counter++;
    // int len = snprintf((char*)tx_buf, sizeof(tx_buf), "[HOST] Test message %ld\r\n", counter);
    // if (usb_host_send(tx_buf, len) == UX_SUCCESS) {
    //   printf("[USB_HOST] ✓ TX: %s", (char*)tx_buf);
    // } else {
    //   printf("[USB_HOST] ✗ TX Error\n");
    // }

    /* Try to receive data from device */
    rx_len = usb_host_receive(rx_buf, sizeof(rx_buf));
    if (rx_len > 0)
    {
      printf("[USB_HOST] RX [%ld bytes]: %.*s\n", rx_len, (int)rx_len, (char*)rx_buf);
    }

    osDelay(2000);
  }
}

/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
  /* creation of defaultTask */
  // defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* USB Device test task (receives and echoes data) */
  usbDeviceTaskHandle = osThreadNew(usb_device_test_task, NULL, &usbDeviceTask_attributes);
  if (usbDeviceTaskHandle == NULL)
  {
    /* If this prints, the task never starts because creation failed. */
    printf("[RTOS] ERROR: usb_device_test_task creation failed\r\n");
  }
  /* USB Host test task (sends and receives test messages) */
  // usbHostTaskHandle = osThreadNew(usb_host_test_task, NULL, &usbHostTask_attributes);
  // if (usbHostTaskHandle == NULL)
  // {
  //   /* If this prints, the task never starts because creation failed. */
  //   printf("[RTOS] ERROR: usb_host_test_task creation failed\r\n");
  // }
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}
/* USER CODE BEGIN Header_StartDefaultTask */
/**
* @brief Function implementing the defaultTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN defaultTask */
  (void)argument;
  for(;;)
  {
    osDelay(1000);
  }
  
  /* USER CODE END defaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

