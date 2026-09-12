# C6L UI v1.0.1

## English

All v1.0.0 UI features are retained; v1.0.1 is a preferences / SPIFFS fix. The persistence fix and both final firmware images have been tested on physical M5Stack Unit C6L.

- Increased C6L SPIFFS from 128 KiB to 256 KiB; retained two OTA slots.
- Fixed the capacity issue preventing Path Hash Size and other preferences from surviving reboot when the filesystem was near full.
- Improved prefs save error handling; dirty state is retained on failure.
- Preserved contacts, channels and device identity without reducing contact limits. Raw advert cache may be reclaimed under storage pressure.
- Removed temporary HASH BOOT, HASH SAVE and FS INFO diagnostics.
- A one-time CLEAN INSTALL (full flash erase and -merged.bin) is required to apply the new partition layout. Back up needed data: CLEAN INSTALL erases device data.
- Future updates can use normal UPDATE .bin after migration unless the partition table changes again.

## Polski

Wszystkie funkcje UI v1.0.0 pozostają zachowane; v1.0.1 to poprawka preferences / SPIFFS. Poprawka trwałości i oba finalne obrazy firmware zostały przetestowane na fizycznym M5Stack Unit C6L.

- Zwiększono C6L SPIFFS ze 128 KiB do 256 KiB; zachowano dwa sloty OTA.
- Usunięto problem pojemności uniemożliwiający zachowanie Path Hash Size i innych preferencji po restarcie przy prawie pełnym filesystemie.
- Poprawiono obsługę błędów zapisu prefs; po niepowodzeniu stan dirty pozostaje zachowany.
- Zachowano kontakty, kanały i tożsamość urządzenia bez zmniejszania limitu kontaktów. Przy braku miejsca można odzyskiwać cache surowych reklam.
- Usunięto tymczasową diagnostykę HASH BOOT, HASH SAVE i FS INFO.
- Nowy layout wymaga jednorazowego CLEAN INSTALL (pełne wymazanie flash i -merged.bin). Zabezpiecz potrzebne dane: CLEAN INSTALL kasuje dane urządzenia.
- Po migracji kolejne aktualizacje mogą używać zwykłego UPDATE .bin, chyba że tablica partycji zmieni się ponownie.
