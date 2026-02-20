/**
  ******************************************************************************
  * @file    psram_shutdown.h
  * @brief   Complete PSRAM + XSPI1 shutdown for safe software reset.
  *
  *          Uses direct XSPI1 register access + HAL RCC/GPIO macros.
  *          Does NOT require HAL_XSPI_MODULE_ENABLED.
  ******************************************************************************
  */

#ifndef PSRAM_SHUTDOWN_H
#define PSRAM_SHUTDOWN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32n6xx_hal.h"

/**
  * @brief  Complete PSRAM + XSPI1 teardown:
  *         1. Ensures XSPI1 clock + peripheral are enabled
  *         2. Aborts memory-mapped mode
  *         3. Sends Global Reset (0xFF) to PSRAM chip
  *         4. Disables XSPI1 peripheral
  *         5. Force-resets XSPI1 via RCC (all registers -> power-on defaults)
  *         6. Deinits all XSPI1 GPIO pins (analog/floating)
  *         7. Disables XSPI1 clock
  */
void PSRAM_Shutdown(void);

/**
  * @brief  Disables interrupts, calls PSRAM_Shutdown(), then NVIC_SystemReset().
  *         Use this instead of calling NVIC_SystemReset() directly.
  */
void PSRAM_ShutdownAndReset(void);

#ifdef __cplusplus
}
#endif

#endif /* PSRAM_SHUTDOWN_H */
