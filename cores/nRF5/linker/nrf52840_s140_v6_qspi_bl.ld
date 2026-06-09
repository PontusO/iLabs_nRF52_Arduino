/* Linker script for nRF52840 boards using the iLabs/QSPI-variant Adafruit
 * nRF52 bootloader at 0x000F0000 (shifted 16 KB down from the standard
 * 0x000F4000 to make room for the QSPI driver + DFU rollback backup code).
 *
 * The app data region (0xE6000 .. 0xF0000) is owned by InternalFileSystem;
 * FLASH stops at 0xE6000 so the linker cannot place app code into the LFS
 * partition, and LFS cannot grow into the bootloader.
 *
 * Used by:
 *   - iLabs Connectivity 840   (-DARDUINO_CONNECTIVITY_840)
 *   - iLabs Challenger 840 BLE (-DARDUINO_CHALLENGER_840_BLE)
 *
 * Keep in lockstep with LFS_FLASH_ADDR / LFS_FLASH_TOTAL_SIZE in
 * libraries/InternalFileSytem/src/InternalFileSystem.cpp -- the static_assert
 * there will fail at build time if these drift.
 */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  FLASH (rx)     : ORIGIN = 0x26000, LENGTH = 0xE6000 - 0x26000

  /* SRAM required by Softdevice depend on
   * - Attribute Table Size (Number of Services and Characteristics)
   * - Vendor UUID count
   * - Max ATT MTU
   * - Concurrent connection peripheral + central + secure links
   * - Event Len, HVN queue, Write CMD queue
   */
  RAM (rwx) :  ORIGIN = 0x20006000, LENGTH = 0x20040000 - 0x20006000
}

SECTIONS
{
  . = ALIGN(4);
  .svc_data :
  {
    PROVIDE(__start_svc_data = .);
    KEEP(*(.svc_data))
    PROVIDE(__stop_svc_data = .);
  } > RAM

  .fs_data :
  {
    PROVIDE(__start_fs_data = .);
    KEEP(*(.fs_data))
    PROVIDE(__stop_fs_data = .);
  } > RAM
} INSERT AFTER .data;

INCLUDE "nrf52_common.ld"
