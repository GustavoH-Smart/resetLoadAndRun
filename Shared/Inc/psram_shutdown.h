/**
  ******************************************************************************
  * @file    psram_shutdown.h
  * @brief   PSRAM shutdown and safe reset functions (shared between FSBL & Appli)
  *
  *          Uses direct XSPI1 register access (CMSIS device headers only),
  *          so it works without HAL_XSPI_MODULE_ENABLED.
  ******************************************************************************
  */

#ifndef PSRAM_SHUTDOWN_H
#define PSRAM_SHUTDOWN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32n6xx_hal.h"

/**
  * @brief  Fully shuts down the PSRAM on XSPI1:
  *         1. Aborts memory-mapped mode (returns XSPI1 to indirect mode)
  *         2. Sends Global Reset command (0xFF) to PSRAM
  *         3. Waits tRST >= 2 us for PSRAM internal reset
  *         4. Disables the XSPI1 peripheral
  *
  *         Safe to call from both FSBL and Appli contexts.
  */
void PSRAM_Shutdown(void);

/**
  * @brief  Convenience: calls PSRAM_Shutdown() then NVIC_SystemReset().
  *         Use this instead of calling NVIC_SystemReset() directly.
  */
void PSRAM_ShutdownAndReset(void);

#ifdef __cplusplus
}
#endif

#endif /* PSRAM_SHUTDOWN_H */
