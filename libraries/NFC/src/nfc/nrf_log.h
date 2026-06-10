/* Minimal nrf_log.h shim
 *
 * NFC sources (hal_nfc_t2t_logger.h, nfc_ndef_parser_logger.h) include this
 * header for Nordic SDK's logging facility. We're not pulling in the full
 * logger backend; stub everything to no-ops. The library's logging is
 * already gated behind compile-time flags (e.g. HAL_NFC_DEBUG_PIN_ENABLE),
 * so disabling NRF_LOG_PRINTF here just makes the gates explicit.
 */
#ifndef NRF_LOG_H__
#define NRF_LOG_H__

#define NRF_LOG_PRINTF(...)         ((void)0)
#define NRF_LOG_INFO(...)           ((void)0)
#define NRF_LOG_DEBUG(...)          ((void)0)
#define NRF_LOG_WARNING(...)        ((void)0)
#define NRF_LOG_ERROR(...)          ((void)0)
#define NRF_LOG_HEXDUMP_INFO(...)   ((void)0)
#define NRF_LOG_HEXDUMP_DEBUG(...)  ((void)0)

#endif /* NRF_LOG_H__ */
