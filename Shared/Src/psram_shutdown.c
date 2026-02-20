/**
  ******************************************************************************
  * @file    psram_shutdown.c
  * @brief   PSRAM shutdown via direct XSPI1 register access.
  *
  *          This file does NOT require HAL_XSPI_MODULE_ENABLED.
  *          It uses only CMSIS register definitions (XSPI_TypeDef, XSPI1)
  *          that are always available from the device header (stm32n657xx.h).
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

void PSRAM_Shutdown(void)
{
  /* ------ 1. Abort ongoing XSPI1 operation (exits memory-mapped mode) -- */
  SET_BIT(XSPI1->CR, XSPI_CR_ABORT);

  /* ABORT bit auto-clears when the abort completes */
  while (READ_BIT(XSPI1->CR, XSPI_CR_ABORT)) {}

  /* ------ 2. Set functional mode to Indirect Write (FMODE = 00) -------- */
  CLEAR_BIT(XSPI1->CR, XSPI_CR_FMODE);

  /* ------ 3. Wait for peripheral to be free ----------------------------- */
  while (READ_BIT(XSPI1->SR, XSPI_SR_BUSY)) {}

  /* ------ 4. Configure command: 8-line instruction, no addr/data -------- */
  WRITE_REG(XSPI1->CCR, PSRAM_GRESET_CCR);
  WRITE_REG(XSPI1->TCR, 0U);   /* 0 dummy cycles, DQS disabled */
  WRITE_REG(XSPI1->DLR, 0U);   /* no data bytes */

  /* ------ 5. Write instruction: 0xFF = Global Reset --------------------- */
  /*         Writing IR triggers the command when ADMODE=0 and DMODE=0.     */
  WRITE_REG(XSPI1->IR, 0xFFU);

  /* ------ 6. Wait for Transfer Complete --------------------------------- */
  while (!READ_BIT(XSPI1->SR, XSPI_SR_TCF)) {}

  /* ------ 7. Clear Transfer Complete Flag ------------------------------- */
  WRITE_REG(XSPI1->FCR, XSPI_FCR_CTCF);

  /* ------ 8. Wait tRST >= 2 us (at 400 MHz ~800 cycles; margin added) -- */
  for (volatile uint32_t d = 0; d < 1000U; d++) {}

  /* ------ 9. Disable the XSPI1 peripheral ------------------------------ */
  CLEAR_BIT(XSPI1->CR, XSPI_CR_EN);
}

void PSRAM_ShutdownAndReset(void)
{
  PSRAM_Shutdown();
  NVIC_SystemReset();
}
