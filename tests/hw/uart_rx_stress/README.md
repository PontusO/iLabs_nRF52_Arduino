# uart_rx_stress

Hardware bench test for the core `Uart` RX path (double-buffered EasyDMA +
idle-flush). Not part of the CI example build; it needs a board and a jumper.

## Wiring

Connectivity 840: jumper **D2 (Serial1 TX)** to **D3 (Serial1 RX)**.
Open the USB CDC serial port for the report.

## What it does

- A low-priority task streams `%SCMNOTIFYEV:"ADRASTEA-IReady" #NNNNNN\r\n`
  lines out of Serial1 at 115200. A quarter of the lines are split in two
  with a 0.7..3.2 ms pause in the middle (modem-style TX stall); every 16th
  line is followed by 1..4 ms of silence so the idle-flush path runs constantly.
- A normal-priority task (same priority as the modem RX task in the field
  firmware) reads Serial1 and checks each line byte-for-byte against what the
  sequence number says it must be.
- A high-priority task hogs the CPU for 3 ms every 17 ms and then masks
  interrupts for 500 us, so the FreeRTOS timer service task (which drives the
  idle-flush) misses ticks exactly the way LMIC / BLE / QSPI work makes it miss
  them on a real unit.

## Reading the result

Every 5 s a status line is printed; after 60 s a verdict:

    verdict: PASS (14876 lines checked, 0 bad, 0 lost, 0 hw overruns)

`bad` = a line arrived with wrong content (dropped/doubled/corrupt byte), and
the first six offenders are printed with non-printables escaped.
`lost` = sequence numbers that never arrived. `hw_overruns` is the UARTE
ERRORSRC overrun count via `Serial1.getRxOverruns()`.

Keys: `s` stall on/off, `c` critical-section stalls on/off, `g` mid-line gaps
on/off, `r` reset counters, `?` help.

## Build

    arduino-cli compile \
      --fqbn "ilabs:iLabs_nRF52_Arduino:connectivity_840:softdevice=s140v6,debug=l0,debug_output=serial,external_mem_size=6MB" \
      --build-property "compiler.cpp.extra_flags=-DSERIAL_BUFFER_SIZE=2048" \
      --build-property "compiler.c.extra_flags=-DSERIAL_BUFFER_SIZE=2048" \
      --output-dir /tmp/uart_rx_stress tests/hw/uart_rx_stress

`-DSERIAL_BUFFER_SIZE=2048` matches what the Palorax production build passes.

## Flash

Same as the production sketch: app hex plus a regenerated bootloader-settings
page in one pyOCD call, sector erase only (never chip erase).

    slottool.py bootsettings uart_rx_stress.ino.hex -o bs.hex
    pyocd flash --target nrf52840 --erase=sector uart_rx_stress.ino.hex bs.hex
