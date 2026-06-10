/* Minimal nordic_common.h shim
 *
 * NFC sources sourced from Nordic SDK 17.x include this header. The BSP
 * doesn't ship the full SDK utility layer, so we provide just the macros
 * actually used: UNUSED_VARIABLE, UNUSED_PARAMETER, ARRAY_SIZE, MIN, MAX.
 */
#ifndef NORDIC_COMMON_H__
#define NORDIC_COMMON_H__

#ifndef UNUSED_VARIABLE
#define UNUSED_VARIABLE(X)  ((void)(X))
#endif

#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER(X) UNUSED_VARIABLE(X)
#endif

#ifndef UNUSED_RETURN_VALUE
#define UNUSED_RETURN_VALUE(X) UNUSED_VARIABLE(X)
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(X) (sizeof(X) / sizeof((X)[0]))
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

/* Byte extraction macros — hal_nfc_t2t.c packs the 4-byte serial-number
 * tag header into nfc_internal[] one byte at a time using LSB_32/MSB_32. */
#ifndef LSB_16
#define LSB_16(a) ((uint8_t)((a) & 0x00FF))
#endif
#ifndef MSB_16
#define MSB_16(a) ((uint8_t)(((a) & 0xFF00) >> 8))
#endif
#ifndef LSB_32
#define LSB_32(a) ((uint8_t)((a) & 0x000000FFu))
#endif
#ifndef MSB_32
#define MSB_32(a) ((uint8_t)(((a) & 0xFF000000u) >> 24))
#endif

/* NFCT_ERRORSTATUS fallback bits: present on nRF52832 silicon but removed
 * from nRF52840. hal_nfc_t2t.c ORs them into a clear-all mask; defining
 * them as zero on 52840 keeps the mask valid (only FRAMEDELAYTIMEOUT
 * actually needs clearing on this chip). The MDK header was included via
 * nrf.h before nordic_common.h, so #ifndef cleanly defers to the real
 * definitions when they exist.
 */
#ifndef NFCT_ERRORSTATUS_NFCFIELDTOOWEAK_Msk
#define NFCT_ERRORSTATUS_NFCFIELDTOOWEAK_Msk    0u
#endif
#ifndef NFCT_ERRORSTATUS_NFCFIELDTOOSTRONG_Msk
#define NFCT_ERRORSTATUS_NFCFIELDTOOSTRONG_Msk  0u
#endif

#endif /* NORDIC_COMMON_H__ */
