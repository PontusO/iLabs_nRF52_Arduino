/* Minimal nrf_drv_clock.h shim
 *
 * hal_nfc_t2t.c was sourced from Nordic SDK 17.x and uses the legacy
 * nrf_drv_clock_* HFCLK API to request the crystal oscillator before
 * activating NFCT (HFCLK must be running while NFCT emulates a tag).
 *
 * Rather than importing the full legacy nrf_drv_clock driver (which has
 * its own dependency chain), we satisfy the surface area NFC actually
 * uses by wrapping the SoftDevice HFCLK APIs that this BSP always has
 * available (SOFTDEVICE_PRESENT is set for every board in boards.txt).
 *
 * Behavioural difference: the legacy API was asynchronous (callback fired
 * from a clock-event ISR). Here we busy-poll sd_clock_hfclk_is_running()
 * — HFXO startup is ~256 us so blocking is cheap, and it sidesteps the
 * need for an event handler chain.
 */
#ifndef NRF_DRV_CLOCK_H__
#define NRF_DRV_CLOCK_H__

#include <stdint.h>
#include "nrf.h"
#include "nrf_soc.h"
#include "nrf_sdm.h"
#include "sdk_errors.h"

typedef enum
{
    NRF_DRV_CLOCK_EVT_HFCLK_STARTED,
    NRF_DRV_CLOCK_EVT_LFCLK_STARTED,
    NRF_DRV_CLOCK_EVT_CAL_DONE,
    NRF_DRV_CLOCK_EVT_CAL_ABORTED,
} nrf_drv_clock_evt_type_t;

typedef void (*nrf_drv_clock_event_handler_t)(nrf_drv_clock_evt_type_t event);

typedef struct nrf_drv_clock_handler_item_s
{
    struct nrf_drv_clock_handler_item_s * p_next;
    nrf_drv_clock_event_handler_t         event_handler;
} nrf_drv_clock_handler_item_t;

static inline ret_code_t nrf_drv_clock_init(void)
{
    /* SoftDevice owns clock management; nothing to do. */
    return 0; /* NRF_SUCCESS */
}

static inline void nrf_drv_clock_hfclk_request(nrf_drv_clock_handler_item_t * p_handler_item)
{
    uint8_t sd_enabled = 0;
    (void) sd_softdevice_is_enabled(&sd_enabled);

    if (sd_enabled)
    {
        /* SoftDevice path: cooperate with the stack's clock arbitration. */
        (void) sd_clock_hfclk_request();
        uint32_t running = 0;
        do {
            (void) sd_clock_hfclk_is_running(&running);
        } while (!running);
    }
    else
    {
        /* Bare-metal fallback: a sketch that uses NFC but not BLE (i.e.
         * never calls Bluefruit.begin()) leaves the SoftDevice disabled,
         * making sd_clock_hfclk_* return NRF_ERROR_INVALID_STATE. Drive
         * HFCLK directly via the CLOCK peripheral instead. */
        NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
        NRF_CLOCK->TASKS_HFCLKSTART    = 1;
        while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0) { /* spin */ }
    }

    if (p_handler_item != NULL && p_handler_item->event_handler != NULL)
    {
        p_handler_item->event_handler(NRF_DRV_CLOCK_EVT_HFCLK_STARTED);
    }
}

static inline void nrf_drv_clock_hfclk_release(void)
{
    uint8_t sd_enabled = 0;
    (void) sd_softdevice_is_enabled(&sd_enabled);

    if (sd_enabled)
    {
        (void) sd_clock_hfclk_release();
    }
    else
    {
        NRF_CLOCK->TASKS_HFCLKSTOP = 1;
    }
}

#endif /* NRF_DRV_CLOCK_H__ */
