/**
  ******************************************************************************
  * @file    psram_shutdown.c
  * @brief   Complete PSRAM + XSPI1 shutdown for safe software reset.
  *
  *          Performs a full teardown:
  *            - Global Reset command to PSRAM chip
  *            - XSPI1 peripheral disable + RCC force-reset
  *            - GPIO pins deinit (back to analog/floating)
  *            - XSPI1 clock disable
  *
  *          This file does NOT require HAL_XSPI_MODULE_ENABLED.
  *          Uses CMSIS register definitions + HAL RCC/GPIO macros only.
  ******************************************************************************
  */

#include "psram_shutdown.h"

/* -----------------------------------------------------------------------
 * XSPI1 CCR register value for the Global Reset command:
 *   IMODE  = 100b (8 lines)   -> bits [2:0] = 4
 *   IDTR   = 0   (no DTR)     -> bit  3     = 0
 *   ISIZE  = 00  (8-bit)      -> bits [5:4] = 0
 *   ADMODE = 000 (none)       -> bits [10:8]= 0
 *   ABMODE = 000 (none)       -> bits [18:16]=0
 *   DMODE  = 000 (none)       -> bits [26:24]=0
 * ----------------------------------------------------------------------- */
#define PSRAM_GRESET_CCR  (4U << XSPI_CCR_IMODE_Pos)

/* XSPI1 GPIO pins (from HAL_XSPI_MspInit / HAL_XSPI_MspDeInit) */
#define XSPI1_GPIOP_PINS  (GPIO_PIN_0  | GPIO_PIN_1  | GPIO_PIN_2  | GPIO_PIN_3  | \
                            GPIO_PIN_4  | GPIO_PIN_5  | GPIO_PIN_6  | GPIO_PIN_7  | \
                            GPIO_PIN_8  | GPIO_PIN_9  | GPIO_PIN_10 | GPIO_PIN_11 | \
                            GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15)

#define XSPI1_GPIOO_PINS  (GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4)

void PSRAM_Shutdown(void)
{
  /* ===== PHASE 1: Ensure XSPI1 is alive and can talk to PSRAM ========= */

  __HAL_RCC_XSPI1_CLK_ENABLE();
  __HAL_RCC_GPIOP_CLK_ENABLE();
  __HAL_RCC_GPIOO_CLK_ENABLE();

  /* Re-enable XSPI1 if it was disabled (e.g. by a previous shutdown) */
  SET_BIT(XSPI1->CR, XSPI_CR_EN);

  /* ===== PHASE 2: Abort memory-mapped mode ============================ */

  SET_BIT(XSPI1->CR, XSPI_CR_ABORT);
  while (READ_BIT(XSPI1->CR, XSPI_CR_ABORT)) {}   /* auto-clears */

  CLEAR_BIT(XSPI1->CR, XSPI_CR_FMODE);            /* Indirect Write */
  while (READ_BIT(XSPI1->SR, XSPI_SR_BUSY)) {}

  /* ===== PHASE 3: Send Global Reset (0xFF) to PSRAM chip ============== */

  WRITE_REG(XSPI1->CCR, PSRAM_GRESET_CCR);
  WRITE_REG(XSPI1->TCR, 0U);
  WRITE_REG(XSPI1->DLR, 0U);
  WRITE_REG(XSPI1->IR,  0xFFU);                    /* triggers command */

  while (!READ_BIT(XSPI1->SR, XSPI_SR_TCF)) {}
  WRITE_REG(XSPI1->FCR, XSPI_FCR_CTCF);

  /* tRST >= 2 us  (generous margin) */
  for (volatile uint32_t d = 0; d < 2000U; d++) {}

  /* ===== PHASE 4: Tear down XSPI1 peripheral completely =============== */

  /* Disable the peripheral */
  CLEAR_BIT(XSPI1->CR, XSPI_CR_EN);

  /* Force-reset via RCC -> all XSPI1 registers back to power-on defaults */
  __HAL_RCC_XSPI1_FORCE_RESET();
  __HAL_RCC_XSPI1_RELEASE_RESET();

  /* ===== PHASE 5: Deinit GPIO pins (back to analog / floating) ======== */
  /*       This removes any electrical drive to the PSRAM chip.           */

  HAL_GPIO_DeInit(GPIOP, XSPI1_GPIOP_PINS);        /* PP0..PP15  data   */
  HAL_GPIO_DeInit(GPIOO, XSPI1_GPIOO_PINS);        /* PO0 NCS, PO2-4   */

  /* ===== PHASE 6: Kill XSPI1 clock ==================================== */

  __HAL_RCC_XSPI1_CLK_DISABLE();
}

void PSRAM_ShutdownAndReset(void)
{
  __disable_irq();
  PSRAM_Shutdown();
  NVIC_SystemReset();
}
