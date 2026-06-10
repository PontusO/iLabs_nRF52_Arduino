/* Minimal nrf_delay.h shim
 *
 * NFC parser sources sourced from Nordic SDK 17.x include this header for
 * nrf_delay_us / nrf_delay_ms. The BSP's cores/nRF5/delay.h provides
 * delayMicroseconds() and delay() with identical semantics; wrap them.
 */
#ifndef NRF_DELAY_H__
#define NRF_DELAY_H__

#include "delay.h"

static inline void nrf_delay_us(uint32_t us) { delayMicroseconds(us); }
static inline void nrf_delay_ms(uint32_t ms) { delay(ms); }

#endif /* NRF_DELAY_H__ */
