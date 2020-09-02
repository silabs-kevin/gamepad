/*************************************************************************
    > File Name: joystick.c
    > Author: Kevin
    > Created Time: 2020-09-02
    > Description:
 ************************************************************************/

/* Includes *********************************************************** */
#include <string.h>
#include "inc/hal/joystick.h"

#include "em_cmu.h"
#include "em_gpio.h"
#include "em_prs.h"
#include "em_ldma.h"
#include "em_letimer.h"
#include "sl_bt_api.h"
#include "logging/logging.h"
/* Defines  *********************************************************** */
#define ADC_RESULT_SIZE 4

#define ADC_BUFFER_SIZE (ADC_RESULT_SIZE * JSTK_NUM * 2)

static uint8_t adc_buf[ADC_BUFFER_SIZE] = { 0 };
/* Global Variables *************************************************** */

/* Const Variables *************************************************** */
static const gpio_t axis_x_gpio = {
  .port = gpioPortA,
  .pin = 5,
  .pos = adcPosSelAPORT2XCH9
};

static const gpio_t axis_y_gpio = {
  .port = gpioPortA,
  .pin = 5,
};

static const gpio_t axis_z_gpio = {
  .port = gpioPortA,
  .pin = 5,
};
static const gpio_t axis_rz_gpio = {
  .port = gpioPortA,
  .pin = 5,
};

/* Static Variables *************************************************** */
static joystick_t jstk = { 0 };

/* Static Functions Declaractions ************************************* */

LDMA_TransferCfg_t trans;
LDMA_Descriptor_t descr;

/**************************************************************************//**
 * @brief LDMA Handler
 *****************************************************************************/
void LDMA_IRQHandler(void)
{
  // Clear interrupt flag
  LDMA_IntClear((1 << LDMA_CHANNEL) << _LDMA_IFC_DONE_SHIFT);

  sl_bt_external_signal(JOYSTICK_UPDATE_EVT);
}

static void _jstk_stop(void)
{
  if (jstk.timer.state != enabled) {
    return;
  }

  LETIMER_Enable(LETIMER0, 0);
  jstk.timer.state = init_ed;
}

static void _jstk_start(void)
{
  if (jstk.timer.state == init_ed) {
    LETIMER_Enable(LETIMER0, 1);
  }
}

static void _jstk_init(uint16_t freq,
                       uint8_t enable)
{
  if (jstk.timer.state == enabled) {
    _jstk_stop();
  }

  /* ADC Init */
  if (jstk.adc_state == nil) {
    // Declare init structs
    ADC_Init_TypeDef init = ADC_INIT_DEFAULT;
    ADC_InitScan_TypeDef initScan = ADC_INITSCAN_DEFAULT;

    // Enable ADC clock
    CMU_ClockEnable(cmuClock_ADC0, true);
    CMU_ClockEnable(cmuClock_HFPER, true);

    // Select AUXHFRCO for ADC ASYNC mode so it can run in EM2
    CMU->ADCCTRL = CMU_ADCCTRL_ADC0CLKSEL_AUXHFRCO;

    // Set AUXHFRCO frequency and use it to setup the ADC
    CMU_AUXHFRCOFreqSet(cmuAUXHFRCOFreq_4M0Hz);
    init.timebase = ADC_TimebaseCalc(CMU_AUXHFRCOBandGet());
    init.prescale = ADC_PrescaleCalc(ADC_FREQ, CMU_AUXHFRCOBandGet());

    // Let the ADC enable its clock on demand in EM2
    init.em2ClockConfig = adcEm2ClockOnDemand;

    for (uint8_t i = 0; i < ADC_CHANNEL_NUM; i++) {
      if (jstk.axis[i].gpio) {
        // Add external ADC input to scan. See README for corresponding EXP header pin.
        // *Note that internal channels are unavailable in ADC scan mode
        uint32_t ret = ADC_ScanSingleEndedInputAdd(&initScan,
                                                   adcScanInputGroup0,
                                                   adcPosSelAPORT2XCH9);
        ASSERT(ret);
      }
    }

    // Basic ADC scan configuration
    initScan.diff       = false;      // single-ended
    initScan.reference  = adcRef2V5;  // 2.5V reference
    initScan.resolution = adcRes12Bit; // 12-bit resolution
    initScan.acqTime    = adcAcqTime4;// set acquisition time to meet minimum requirements

    // DMA is available in EM2 for processing SCANFIFO DVL request
    initScan.scanDmaEm2Wu = 1;

    // Enable PRS trigger and select channel 0
    initScan.prsEnable = true;
    initScan.prsSel = (ADC_PRSSEL_TypeDef) PRS_CHANNEL;

    // Initialize ADC
    ADC_Init(ADC0, &init);
    ADC_InitScan(ADC0, &initScan);

    // Set scan data valid level (DVL)
    ADC0->SCANCTRLX = (ADC0->SCANCTRLX & ~_ADC_SCANCTRLX_DVL_MASK) | (((ADC_CHANNEL_NUM - 1) << _ADC_SCANCTRLX_DVL_SHIFT) & _ADC_SCANCTRLX_DVL_MASK);

    // Clear the SCAN FIFO
    ADC0->SCANFIFOCLEAR = ADC_SCANFIFOCLEAR_SCANFIFOCLEAR;
  }

  if (jstk.timer.state == nil) {
    // Start LFRCO and wait until it is stable
    CMU_OscillatorEnable(cmuOsc_LFRCO, true, true);

    // Enable clock to the interface of the low energy modules
    CMU_ClockEnable(cmuClock_HFLE, true);

    // Route the LFRCO clock to LFA (TIMER0)
    CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFRCO);

    // Enable clock for LETIMER0
    CMU_ClockEnable(cmuClock_LETIMER0, true);

    LETIMER_Init_TypeDef letimerInit = LETIMER_INIT_DEFAULT;

    letimerInit.enable = false;

    // Reload COMP0 on underflow, pulse output, and run in repeat mode
    letimerInit.comp0Top  = true;
    letimerInit.ufoa0     = letimerUFOAPulse;
    letimerInit.repMode   = letimerRepeatFree;

    // Initialize LETIMER
    LETIMER_Init(LETIMER0, &letimerInit);

    // Need REP0 != 0 to pulse on underflow
    LETIMER_RepeatSet(LETIMER0, 0, 1);

    // Use LETIMER0 as async PRS to trigger ADC in EM2
    CMU_ClockEnable(cmuClock_PRS, true);

    PRS_SourceAsyncSignalSet(PRS_CHANNEL, PRS_CH_CTRL_SOURCESEL_LETIMER0,
                             PRS_CH_CTRL_SIGSEL_LETIMER0CH0);

    // Enable LDMA clock
    CMU_ClockEnable(cmuClock_LDMA, true);

    // Basic LDMA configuration
    LDMA_Init_t ldmaInit = LDMA_INIT_DEFAULT;

    LDMA_Init(&ldmaInit);

    // Transfer triggers on ADC Scan conversion complete
    trans = (LDMA_TransferCfg_t)LDMA_TRANSFER_CFG_PERIPHERAL(ldmaPeripheralSignal_ADC0_SCAN);

    descr = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_LINKREL_P2M_BYTE(
      &(ADC0->SCANDATA),  // source
      adc_buf,          // destination
      ADC_BUFFER_SIZE,    // data transfer size
      0);                 // link relative offset (links to self)

    descr.xfer.blockSize = ADC_CHANNEL_NUM - 1;  // transfers ADC_CHANNEL_NUM number of units per arbitration cycle
    descr.xfer.ignoreSrec = true; // ignores single requests to reduce energy usage
    descr.xfer.size = ldmaCtrlSizeWord; // transfers words instead of bytes

    // Initialize LDMA transfer
    LDMA_StartTransfer(LDMA_CHANNEL, &trans, &descr);

    // Clear pending and enable interrupts for channel
    NVIC_ClearPendingIRQ(LDMA_IRQn);
    NVIC_EnableIRQ(LDMA_IRQn);
  }

  // calculate the topValue
  uint32_t topValue = CMU_ClockFreqGet(cmuClock_LETIMER0) / (jstk.timer.freq == 0 ? LETIMERDESIRED : jstk.timer.freq);

  // Compare on wake-up interval count
  LETIMER_CompareSet(LETIMER0, 0, topValue);

  if (enable) {
    _jstk_start();
  }
}

/**************************************************************************//**
 * @brief LDMA initialization
 *****************************************************************************/

/**************************************************************************//**
 * @brief ADC initialization
 *****************************************************************************/
void jstk_init(void)
{
  memset(&jstk, 0, sizeof(joystick_t));
  jstk.axis[0].gpio = &axis_x_gpio;
  jstk.axis[1].gpio = NULL;
  jstk.axis[2].gpio = NULL;
  jstk.axis[3].gpio = NULL;
  jstk.init = _jstk_init;
  jstk.start = _jstk_start;
  jstk.stop = _jstk_stop;
}
