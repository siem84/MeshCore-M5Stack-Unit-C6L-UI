# C6L UI v1.0.1 — Preferences / SPIFFS fix

## English

MeshCore Companion v1.17.1; base development commit `ac7d88efe72201729cc8584c2b85e1f45350d0ec`; public filename base: `v1.17.1-dev-ac7d88ef`.

v1.0.1 includes all v1.0.0 features and changes only preferences persistence / SPIFFS handling. The UI appearance is unchanged.

Preferences could fail to save when SPIFFS ran out of space. The previous C6L layout had 128 KiB SPIFFS; v1.0.1 increases it to **256 KiB**, retaining two OTA slots, with `variants/m5stack_unit_c6l/partitions_4mb_256k_spiffs.csv`. Settings such as Path Hash Size now persist across restart/cold boot in physical C6L testing. Failed prefs saves are reported and do not clear dirty state.

Contacts, channels and device identity are not deliberately deleted or limited by the running firmware to recover space. Storage protection may reclaim raw advert cache; affected adverts need to be received again before export/sharing.

**The first upgrade from v1.0.0 requires CLEAN INSTALL with `-merged.bin` because the partition layout changed. CLEAN INSTALL erases device data, including identity, contacts, channels, preferences and BLE bonding. Back up needed data first.** After migration, later updates may use UPDATE `.bin` unless the partition layout changes again.

This is **C6L UI v1.0.1**, not a new MeshCore version. Both final CLEAN INSTALL and UPDATE images and the persistence fix have been tested on physical M5Stack Unit C6L.

[Release v1.0.1](https://github.com/siem84/MeshCore-M5Stack-Unit-C6L-UI/releases/tag/v1.0.1)

Firmware:

- `MeshCore-v1.17.1-dev-ac7d88ef-M5Stack-Unit-C6L-UI-v1.0.1.bin`
  SHA256: `518B72285AB2A864642D60E4176ACF5DE241FDBCDE6CBE3751C1E12C9F0DD559`
- `MeshCore-v1.17.1-dev-ac7d88ef-M5Stack-Unit-C6L-UI-v1.0.1-merged.bin`
  SHA256: `7AC529DBE5D66ED085D03606DE72FA0AF647CAD5F6A24E1C9EB294D20884EF7D`

## Polski

MeshCore Companion v1.17.1; base development commit `ac7d88efe72201729cc8584c2b85e1f45350d0ec`; public filename base: `v1.17.1-dev-ac7d88ef`.

v1.0.1 zawiera wszystkie funkcje v1.0.0 i zmienia wyłącznie trwały zapis preferences / obsługę SPIFFS. Wygląd UI pozostaje bez zmian.

Preferences mogły nie zostać zapisane z powodu braku miejsca w SPIFFS. Poprzedni layout C6L miał 128 KiB SPIFFS; v1.0.1 zwiększa je do **256 KiB**, zachowując dwa sloty OTA, przez plik `variants/m5stack_unit_c6l/partitions_4mb_256k_spiffs.csv`. Testy fizyczne C6L potwierdziły zachowanie ustawień takich jak Path Hash Size po restart/cold boot. Nieudany zapis prefs jest zgłaszany i nie czyści stanu dirty.

Działający firmware nie usuwa ani celowo nie ogranicza contacts, channels i device identity w celu odzyskania miejsca. Ochrona pamięci może odzyskiwać cache surowych reklam; takie reklamy trzeba ponownie odebrać przed eksportem/udostępnieniem.

**Pierwsze przejście z v1.0.0 wymaga CLEAN INSTALL przez `-merged.bin`, ponieważ zmienił się układ partycji. CLEAN INSTALL kasuje dane urządzenia, w tym identity, contacts, channels, preferences i BLE bonding. Wcześniej zabezpiecz potrzebne dane.** Po migracji późniejsze aktualizacje mogą używać UPDATE `.bin`, o ile układ partycji ponownie się nie zmieni.

To **C6L UI v1.0.1**, a nie nowa wersja MeshCore. Oba finalne obrazy CLEAN INSTALL i UPDATE oraz poprawka trwałości ustawień zostały przetestowane na fizycznym M5Stack Unit C6L.

[Release v1.0.1](https://github.com/siem84/MeshCore-M5Stack-Unit-C6L-UI/releases/tag/v1.0.1)

Firmware:

- `MeshCore-v1.17.1-dev-ac7d88ef-M5Stack-Unit-C6L-UI-v1.0.1.bin`
  SHA256: `518B72285AB2A864642D60E4176ACF5DE241FDBCDE6CBE3751C1E12C9F0DD559`
- `MeshCore-v1.17.1-dev-ac7d88ef-M5Stack-Unit-C6L-UI-v1.0.1-merged.bin`
  SHA256: `7AC529DBE5D66ED085D03606DE72FA0AF647CAD5F6A24E1C9EB294D20884EF7D`
