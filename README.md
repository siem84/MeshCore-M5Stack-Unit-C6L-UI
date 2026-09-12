English | [Polski](README_PL.md)

# MeshCore C6L UI

MeshCore C6L UI is a community/unofficial adaptation of MeshCore Companion for **M5Stack Unit C6L**. It brings up and integrates the built-in OLED, RGB LED, user button and local UI, BLE pairing display, clock with selectable timezone, RF Monitor, and board-specific RF initialization.

## OLED UI gallery

| LOGO | STATUS | RADIO |
| --- | --- | --- |
| ![LOGO](Image%20-%20MeshCore%20M5Stack%20Unit%20C6L%20UI/001%20MeshCore%20M5Stack%20Unit%20C6L%20UI.jpg) | ![STATUS](Image%20-%20MeshCore%20M5Stack%20Unit%20C6L%20UI/002%20MeshCore%20M5Stack%20Unit%20C6L%20UI.jpg) | ![RADIO](Image%20-%20MeshCore%20M5Stack%20Unit%20C6L%20UI/003%20MeshCore%20M5Stack%20Unit%20C6L%20UI.jpg) |
| Startup logo and C6L UI identification. | BLE state, LoRa frequency and RX/TX counters. | LoRa radio parameters. |

| SIGNAL | RF MON | DEVICE |
| --- | --- | --- |
| ![SIGNAL](Image%20-%20MeshCore%20M5Stack%20Unit%20C6L%20UI/004%20MeshCore%20M5Stack%20Unit%20C6L%20UI.jpg) | ![RF MON](Image%20-%20MeshCore%20M5Stack%20Unit%20C6L%20UI/005%20MeshCore%20M5Stack%20Unit%20C6L%20UI.jpg) | ![DEVICE](Image%20-%20MeshCore%20M5Stack%20Unit%20C6L%20UI/006%20MeshCore%20M5Stack%20Unit%20C6L%20UI.jpg) |
| Last packet RSSI, SNR and RX count. | Noise Floor, RSSI, SNR and RX count. | Device identification, uptime and BLE state. |

| CLOCK | TIMEZONE UTC | TIMEZONE EUROPE |
| --- | --- | --- |
| ![CLOCK](Image%20-%20MeshCore%20M5Stack%20Unit%20C6L%20UI/007%20MeshCore%20M5Stack%20Unit%20C6L%20UI.jpg) | ![TIMEZONE UTC](Image%20-%20MeshCore%20M5Stack%20Unit%20C6L%20UI/008%20MeshCore%20M5Stack%20Unit%20C6L%20UI.jpg) | ![TIMEZONE EUROPE](Image%20-%20MeshCore%20M5Stack%20Unit%20C6L%20UI/009%20MeshCore%20M5Stack%20Unit%20C6L%20UI.jpg) |
| Local time and date. | UTC timezone profile. | EUROPE timezone profile with automatic DST. |

## v1.0.0 — Initial C6L UI release

- **OLED 64×48:** support for the built-in SSD1306, with shared SPI operation alongside the SX1262 and a complete local device interface.
- **RGB LED:** support for the built-in WS2812C on GPIO2, indicating startup, BLE connection state and LoRa activity.
- **User button:** SYS_KEY1 through the PI4IOE5V6408 expander, supporting page navigation, CLOCK/TIMEZONE controls and OLED wake-up.
- **BLE pairing:** an event-driven PAIRING overlay showing the actual six-digit passkey. BLE callbacks pass state to the main loop; they do not draw on the OLED or access its SPI bus.
- **Clock / Timezone:** MeshCore/Companion time synchronization, local time display, button-based timezone selection and persistent profile storage.
- **OLED auto-off:** information pages turn off after inactivity; MeshCore continues operating.
- **RF Monitor:** MeshCore Noise Floor, last packet RSSI and SNR, and received-packet count, without an additional radio measurement.
- **Startup buzzer:** audible startup indication.

Seven normal pages: LOGO / STATUS / RADIO / SIGNAL / RF MON / DEVICE / CLOCK.

C6L-specific RF initialization is part of v1.0.0; details below.

[Release v1.0.0](https://github.com/siem84/MeshCore-M5Stack-Unit-C6L-UI/releases/tag/v1.0.0)

## v1.0.1 — Preferences / SPIFFS fix

v1.0.1 includes all v1.0.0 features and changes only preferences persistence / SPIFFS handling. The UI appearance is unchanged.

Preferences could fail to save when SPIFFS ran out of space. The previous C6L layout had 128 KiB SPIFFS; v1.0.1 increases it to **256 KiB**, retaining two OTA slots, with `variants/m5stack_unit_c6l/partitions_4mb_256k_spiffs.csv`. Settings such as Path Hash Size now persist across restart/cold boot in physical C6L testing. Failed prefs saves are reported and do not clear dirty state.

Contacts, channels and device identity are not deliberately deleted or limited by the running firmware to recover space. Storage protection may reclaim raw advert cache; affected adverts need to be received again before export/sharing.

**The first upgrade from v1.0.0 requires CLEAN INSTALL with `-merged.bin` because the partition layout changed. CLEAN INSTALL erases device data, including identity, contacts, channels, preferences and BLE bonding. Back up needed data first.** After migration, later updates may use UPDATE `.bin` unless the partition layout changes again.

This is **C6L UI v1.0.1**, not a new MeshCore version. Both final CLEAN INSTALL and UPDATE images and the persistence fix have been tested on physical M5Stack Unit C6L.

[Release v1.0.1](https://github.com/siem84/MeshCore-M5Stack-Unit-C6L-UI/releases/tag/v1.0.1)

## Technical documentation

MeshCore Companion v1.17.1; base development commit `ac7d88efe72201729cc8584c2b85e1f45350d0ec`; public filename base: `v1.17.1-dev-ac7d88ef`.

This repository is a C6L source overlay for that base commit, not a complete standalone MeshCore tree. Copy the provided source files into the matching base tree, preserving paths.

### RF / LoRa initialization

Unit C6L routes board-specific RF control signals through the PI4IOE5V6408:

| Expander pin | Signal | Initialization |
| --- | --- | --- |
| P5 | SX_LNA_EN | HIGH to enable the required LNA path |
| P6 | SX_ANT_SW | HIGH to set the antenna switch |
| P7 | SX_NRST | LOW for about 100 ms, then HIGH to reset SX1262 |

The implementation configures these pins as outputs, disables their high-impedance state, performs the hardware reset, then sets the antenna switch and LNA enable HIGH. Read-modify-write operations preserve SYS_KEY1/P0 and the other expander bits. Both SPI chip-select lines are inactive before the first shared-bus transfer.

This supplies the RF control states required for correct LoRa receive-path operation on Unit C6L. It does not change the user's frequency, bandwidth, spreading factor, coding rate or TX power settings.

### RGB LED indications

| Color | Meaning |
| --- | --- |
| GREEN | Startup / boot indication |
| WHITE | BLE disconnected |
| BLUE | BLE connected |
| RED | LoRa RX, approximately 150 ms |
| YELLOW | LoRa TX, approximately 150 ms |

RX/TX indications temporarily override the BLE color. The startup indication is not a full hardware self-test.

### OLED UI and controls

There are exactly **seven normal pages**:

```text
LOGO -> STATUS -> RADIO -> SIGNAL -> RF MON -> DEVICE -> CLOCK -> LOGO
```

PAIRING and TIMEZONE are special overlays/modes, not additional pages in this cycle.

| Page | Display |
| --- | --- |
| LOGO | Startup and C6L UI identification: MESH / CORE / C6L UI. Shown for the initial three-second boot interval. |
| STATUS | MeshCore identification, current BLE state, LoRa frequency and RX/TX packet counters. |
| RADIO | Frequency, bandwidth, spreading factor, TX power and coding rate. |
| SIGNAL | Last received packet RSSI and SNR, plus RX count. RSSI/SNR show `---` before the first received packet. |
| RF MON | Noise Floor, last packet RSSI/SNR and received-packet count. A diagnostic view, not a laboratory receiver-sensitivity measurement. |
| DEVICE | C6L device identification, uptime and current BLE state. |
| CLOCK | Local time as HH:MM and date as DD.MM.YY, using MeshCore/Companion-synchronized device time. |

### Button controls

| Context | SYS_KEY1 action | Result |
| --- | --- | --- |
| Normal UI | Short press | Next page |
| OLED off | First press | Wake the current page only; the entire gesture is consumed |
| CLOCK | Short press | Return to LOGO |
| CLOCK | Hold about 3 seconds | Enter TIMEZONE |
| TIMEZONE | Short press | Select the next timezone profile |
| TIMEZONE | Hold about 3 seconds | Save the selected profile and return to CLOCK |
| TIMEZONE | About 30 seconds idle | Cancel the unsaved selection and return to CLOCK |

After waking the OLED, release the button; the next press works normally. PAIRING takes priority and does not allow normal page navigation.

### OLED auto-off

LOGO, STATUS, RADIO, SIGNAL, RF MON and DEVICE turn off after **60 seconds without button activity**. CLOCK stays on without this timeout. TIMEZONE and an active PAIRING overlay also keep the display on, subject to their own mode timeouts.

The first press after auto-off wakes the current screen without changing pages.

**Turning off the OLED does not put the device to sleep or stop MeshCore.** BLE, LoRa, RGB LED and the other firmware functions continue operating.

### BLE pairing screen

When BLE reports a passkey through `onPassKeyNotify(pass_key)`, the main loop displays:

```text
PAIRING
xxxxxx
```

The displayed value is the passkey supplied by that event, formatted as six digits including leading zeros. Stored bonding count does not decide whether it appears.

PAIRING wakes the OLED and has priority over the normal UI. After authentication completes, fails, or BLE disconnects, the previous page and previous OLED on/off state are restored. A 30-second display timeout also closes the overlay without cancelling BLE authentication.

An event during the initial three-second LOGO interval is deferred until that interval ends, and displayed only if pairing is still active. If pairing begins during TIMEZONE, the unsaved timezone selection is cancelled; the UI returns to CLOCK after pairing.

### CLOCK / TIMEZONE

Synchronize the device time through MeshCore/Companion over BLE, then select the local timezone on the device: **CLOCK → hold about 3 seconds → TIMEZONE → short presses to choose → hold about 3 seconds to save**.

| Profile | Local time rules |
| --- | --- |
| UTC | UTC, no daylight saving time |
| EUROPE | Central European CET/CEST with automatic DST using POSIX TZ rules |
| US EAST | Eastern EST/EDT with automatic DST |
| US WEST | Pacific PST/PDT with automatic DST |
| INDIA | UTC+05:30, no daylight saving time |

EUROPE represents Central European time, not every timezone in Europe. The selected profile is stored in preferences and survives restart. Device time remains a UTC/Unix timestamp; only its display is converted to local time.

The phone synchronizes time; it does **not** automatically supply the timezone profile. No recompilation is required to select a profile. If valid device time is unavailable, CLOCK shows `NO TIME` / `SYNC BLE`.

### RF Monitor

Example values:

```text
RF MON
NF -88
R -112
S +6.5
RX 248
```

NF is the existing MeshCore Noise Floor value, without correction for the LNA or a new measurement. R is last packet RSSI in dBm, S is SNR in dB, and RX is the received-packet count. Before the first RX, R and S show `---`. An initial NF of zero is displayed as `NF 0`.

The page refreshes approximately every 750 ms while visible. Large RX counts use readable k/M abbreviations if needed. This display does not initiate a channel scan or change the radio's receive mode. Noise Floor is diagnostic telemetry, not a laboratory measurement of receiver sensitivity.

### Supported hardware

**M5Stack Unit C6L only:**

- ESP32-C6 + SX1262
- SSD1306 OLED, 64×48
- WS2812C RGB LED
- SYS_KEY1 user button
- PI4IOE5V6408 I/O expander

## Installation

**Migration from v1.0.0 / 128 KiB SPIFFS requires a full flash erase and CLEAN INSTALL. UPDATE does not change the partition table. CLEAN INSTALL deletes device data; back up needed data first.**

Use only images for **M5Stack Unit C6L**. The two files serve different purposes.

### UPDATE

For an existing installation already using the v1.0.1 256 KiB SPIFFS layout. Select the ordinary, non-merged file:

```text
MeshCore-v1.17.1-dev-ac7d88ef-M5Stack-Unit-C6L-UI-v1.0.1.bin
```

Size: **1,537,552 bytes**. SHA-256:

```text
518B72285AB2A864642D60E4176ACF5DE241FDBCDE6CBE3751C1E12C9F0DD559
```

Do not select full erase for a normal update intended to retain device data.

### CLEAN INSTALL

For a full installation, including an erased device or a device with a different flash layout:

```text
MeshCore-v1.17.1-dev-ac7d88ef-M5Stack-Unit-C6L-UI-v1.0.1-merged.bin
```

Size: **1,603,088 bytes**. SHA-256:

```text
7AC529DBE5D66ED085D03606DE72FA0AF647CAD5F6A24E1C9EB294D20884EF7D
```

The merged image includes the bootloader, partition table, initial OTA data and application.

> **Data loss:** CLEAN INSTALL with a full erase removes existing device data, including identity, contacts, preferences and BLE bonding. Back up anything you need before continuing.

### Recommended method: MeshCore Web Flasher

1. Connect Unit C6L by USB and open [MeshCore Web Flasher](https://flasher.meshcore.io/).
2. Choose **Custom Firmware** and select the appropriate file from `firmware/`.
3. For UPDATE, select the non-merged file and leave erase disabled.
4. For CLEAN INSTALL, select the file ending in `-merged.bin`. Keep this suffix: the flasher recognizes it, warns about erasing flash, and selects the clean-install path.
5. Select the correct device port, review the erase choice, and follow the flasher instructions. Keep the device connected until flashing finishes.
6. Connect through the MeshCore app, synchronize time, and choose your local timezone on CLOCK.

The merged-file detection and erase behavior are implemented in the [Web Flasher source](https://github.com/meshcore-dev/flasher.meshcore.io/blob/main/flasher.js). When changing between installation types, reload the flasher and reselect the file so that a previous erase choice is not carried over.

## Testing

C6L UI v1.0.0 and both final publication firmware images (UPDATE and CLEAN INSTALL), with the exact SHA-256 hashes retained in Release v1.0.0, have been tested on a physical M5Stack Unit C6L.

The following basic functions were checked:

- Boot/startup.
- OLED and the seven-page UI.
- OLED auto-off and wake-up.
- SYS_KEY1 button.
- RGB LED.
- BLE connection.
- BLE pairing and display of the six-digit passkey.
- LoRa RX/TX.
- Correct initialization of the C6L RF path.
- CLOCK.
- TIMEZONE selection and persistence after restart.
- UPDATE using the ordinary non-merged `.bin`.
- CLEAN INSTALL using `-merged.bin`.

This is the first public release of C6L UI v1.0.0, and unknown issues may remain. Issue reports are welcome; include the hardware, firmware filename and steps to reproduce the problem.

C6L UI v1.0.1: preferences persistence across restart/cold boot and both final UPDATE and CLEAN INSTALL images have been physically tested on M5Stack Unit C6L.

## Credits

- [MeshCore](https://github.com/meshcore-dev/MeshCore) — original project and contributors.
- M5Stack Unit C6L adaptation, OLED UI and C6L-specific additions — **siem84**.
- Public maintainer of this adaptation — **siem84**.

This is a community/unofficial adaptation and is not an official MeshCore release.

## License

The base MeshCore code is licensed under the **MIT License**. [LICENSE](LICENSE) is an unchanged copy of the base repository's `license.txt`, including its original copyright and permission notice.

Third-party framework and library components retain their own licenses. Attribution of the C6L adaptation does not transfer authorship of the original MeshCore project.
