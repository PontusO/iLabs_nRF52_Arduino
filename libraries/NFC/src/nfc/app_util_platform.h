/* Minimal app_util_platform.h shim
 *
 * The NFC library's hal_nfc_t2t.c was sourced from Nordic SDK 17.x and
 * expects this header. The BSP doesn't ship the full SDK 17 utility layer,
 * so we provide just the macros NFC actually uses.
 *
 * The only symbol referenced by hal_nfc_t2t.c is APP_IRQ_PRIORITY_LOW.
 * With SoftDevice S140 present, NVIC priorities 0 and 1 are reserved for
 * the stack; application-available priorities are 2-3 and 5-7. Nordic SDK
 * defines APP_IRQ_PRIORITY_LOW = 6 in that context.
 */
#ifndef APP_UTIL_PLATFORM_H__
#define APP_UTIL_PLATFORM_H__

#define APP_IRQ_PRIORITY_HIGHEST    2
#define APP_IRQ_PRIORITY_HIGH       3
#define APP_IRQ_PRIORITY_MID        5
#define APP_IRQ_PRIORITY_LOW_MID    6
#define APP_IRQ_PRIORITY_LOW        6
#define APP_IRQ_PRIORITY_LOWEST     7
#define APP_IRQ_PRIORITY_THREAD     15

#endif /* APP_UTIL_PLATFORM_H__ */
