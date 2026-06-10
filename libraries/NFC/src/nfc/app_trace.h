/* Minimal app_trace.h shim
 *
 * Nordic SDK NDEF parser sources include this for app_trace_log/dump
 * macros. We stub them to no-ops; the NFC stack works fine without
 * trace output and pulling in the full app_trace backend would drag
 * in UART/RTT/NRF_LOG dependencies we don't need.
 */
#ifndef APP_TRACE_H__
#define APP_TRACE_H__

#define app_trace_init()           ((void)0)
#define app_trace_log(...)         ((void)0)
#define app_trace_dump(...)        ((void)0)

#endif /* APP_TRACE_H__ */
