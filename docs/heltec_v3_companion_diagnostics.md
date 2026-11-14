# Heltec V3 Companion Diagnostics

The diagnostic UI is available in the custom build environment:

```bash
pio run -e Heltec_v3_companion_radio_ble_custom
```

The original BLE Companion environment remains available:

```bash
pio run -e Heltec_v3_companion_radio_ble
```

If PlatformIO is installed in its default virtual environment and `pio` is not
on `PATH`, use `~/.platformio/penv/bin/pio` in the commands below.

## Firmware files and upload

The application image is generated at:

```text
.pio/build/Heltec_v3_companion_radio_ble_custom/firmware.bin
```

To generate a combined image containing the bootloader, partition table, and
application:

```bash
pio run -e Heltec_v3_companion_radio_ble_custom -t mergebin
```

The combined image is
`.pio/build/Heltec_v3_companion_radio_ble_custom/firmware-merged.bin`, for flash
offset `0x0`. The standalone application image uses offset `0x10000`.

To upload through PlatformIO with the board connected:

```bash
pio run -e Heltec_v3_companion_radio_ble_custom -t upload
```

## Pages

- `STATUS`: battery voltage, estimated percentage, uptime, and BLE state.
- `RADIO`: last received RSSI/SNR, configured frequency, and configured TX power.
- `NETWORK`: raw radio RX/TX counters and known contacts.

The diagnostic pages are read-only. Radio settings continue to be changed by
the mobile application and no RF defaults are changed by this customization.

## Navigation

Use the Heltec V3 `USR` button:

- A short press advances to the next page.
- A double press returns to the previous page.
- When the display is off, a short press wakes it without changing the page.

The diagnostic pages follow the repeater discovery page, in the order `STATUS`,
`RADIO`, `NETWORK`, before the hibernation page. Their headers show `01/03`,
`02/03`, and `03/03`. Values refresh every five seconds while the display is on;
the existing 15-second display timeout applies.

## Metric semantics

- Battery voltage comes from the Heltec V3 board ADC implementation.
- Battery percentage is an estimate using the existing 3000-4200 mV mapping.
- Uptime is tracked from the diagnostic task start and handles `millis()` rollover.
- RSSI and SNR are shown only after at least one raw radio packet is received.
- RX counts successful raw radio reads, and TX counts completed raw radio sends.
- Contacts excludes the reserved anonymous contact slots.
- Frequency and TX power are the values held by the Companion preferences.
- BLE `CONNECTED` means the BLE interface has an authenticated client connection.

## Build validation

Both environments compiled successfully with PlatformIO Core 6.2.0 and the
Espressif32 6.11.0 platform, including combined-image generation:

```bash
pio run -e Heltec_v3_companion_radio_ble_custom -e Heltec_v3_companion_radio_ble -t mergebin
```

| Environment | Static RAM | Application flash |
| --- | ---: | ---: |
| Original BLE Companion | 194,828 bytes (59.5%) | 1,666,529 bytes (49.9%) |
| Custom diagnostic UI | 194,852 bytes (59.5%) | 1,668,801 bytes (49.9%) |

Hardware validation remains pending: page layout and button navigation, battery
readings, BLE state transitions, and radio metrics during packet reception and
transmission.
