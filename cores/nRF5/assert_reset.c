/* Replaces newlib's __assert_func, which calls abort() and leaves the
 * CPU spinning forever with interrupts effectively dead. On an
 * unattended device that is a brick: a corrupt LittleFS tripping
 * "block < lfs->cfg->block_count" at mount used to hang here until
 * power cycle, and re-hang on every subsequent boot.
 *
 * Instead: give the application one chance to record what happened
 * (weak hook, e.g. into a .noinit struct a boot-health ladder reads
 * after reset), then reset. Never hang.
 */
#include <nrf.h>

__attribute__((weak))
void ilabs_assert_hook(const char *file, int line, const char *func, const char *expr)
{
  (void) file; (void) line; (void) func; (void) expr;
}

__attribute__((noreturn))
void __assert_func(const char *file, int line, const char *func, const char *expr)
{
  ilabs_assert_hook(file, line, func, expr);
  NVIC_SystemReset();
  for (;;) { }
}
