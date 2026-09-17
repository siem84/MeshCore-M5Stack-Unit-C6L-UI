[English](README.md) | Polski

# MeshCore C6L UI

![MeshCore M5Stack Unit C6L UI](Image%20-%20MeshCore%20M5Stack%20Unit%20C6L%20UI/MeshCore_Unit_C6L_UI_banner_by_siem84.png)

MeshCore C6L UI to społecznościowa, nieoficjalna adaptacja MeshCore Companion dla **M5Stack Unit C6L**. Uruchamia i integruje wbudowany OLED, RGB LED, przycisk użytkownika i lokalny interfejs, ekran parowania BLE, zegar z wyborem strefy czasowej, RF Monitor oraz inicjalizację toru RF właściwą dla tej płytki.

## Galeria OLED UI

| LOGO | STATUS | RADIO |
| --- | --- | --- |
| ![LOGO](Image%20-%20MeshCore%20M5Stack%20Unit%20C6L%20UI/001%20MeshCore%20M5Stack%20Unit%20C6L%20UI.jpg) | ![STATUS](Image%20-%20MeshCore%20M5Stack%20Unit%20C6L%20UI/002%20MeshCore%20M5Stack%20Unit%20C6L%20UI.jpg) | ![RADIO](Image%20-%20MeshCore%20M5Stack%20Unit%20C6L%20UI/003%20MeshCore%20M5Stack%20Unit%20C6L%20UI.jpg) |
| Logo startowe i identyfikacja C6L UI. | Stan BLE, częstotliwość LoRa i liczniki RX/TX. | Parametry radia LoRa. |

| SIGNAL | RF MON | DEVICE |
| --- | --- | --- |
| ![SIGNAL](Image%20-%20MeshCore%20M5Stack%20Unit%20C6L%20UI/004%20MeshCore%20M5Stack%20Unit%20C6L%20UI.jpg) | ![RF MON](Image%20-%20MeshCore%20M5Stack%20Unit%20C6L%20UI/005%20MeshCore%20M5Stack%20Unit%20C6L%20UI.jpg) | ![DEVICE](Image%20-%20MeshCore%20M5Stack%20Unit%20C6L%20UI/006%20MeshCore%20M5Stack%20Unit%20C6L%20UI.jpg) |
| RSSI i SNR ostatniego pakietu oraz licznik RX. | Noise Floor, RSSI, SNR i licznik RX. | Identyfikacja urządzenia, uptime i stan BLE. |

| CLOCK | TIMEZONE UTC | TIMEZONE EUROPE |
| --- | --- | --- |
| ![CLOCK](Image%20-%20MeshCore%20M5Stack%20Unit%20C6L%20UI/007%20MeshCore%20M5Stack%20Unit%20C6L%20UI.jpg) | ![TIMEZONE UTC](Image%20-%20MeshCore%20M5Stack%20Unit%20C6L%20UI/008%20MeshCore%20M5Stack%20Unit%20C6L%20UI.jpg) | ![TIMEZONE EUROPE](Image%20-%20MeshCore%20M5Stack%20Unit%20C6L%20UI/009%20MeshCore%20M5Stack%20Unit%20C6L%20UI.jpg) |
| Lokalna godzina i data. | Profil strefy czasowej UTC. | Profil EUROPE z automatyczną zmianą czasu. |

## v1.0.0 — Initial C6L UI release

- **OLED 64×48:** obsługa wbudowanego SSD1306, współdzielącego SPI z SX1262, oraz kompletny lokalny interfejs urządzenia.
- **RGB LED:** obsługa wbudowanej WS2812C na GPIO2, wskazującej start, stan połączenia BLE i aktywność LoRa.
- **Przycisk użytkownika:** SYS_KEY1 przez ekspander PI4IOE5V6408; nawigacja po ekranach, obsługa CLOCK/TIMEZONE i wybudzanie OLED.
- **Parowanie BLE:** overlay PAIRING wywoływany zdarzeniem, pokazujący rzeczywisty sześciocyfrowy passkey. Callbacki BLE przekazują stan do głównej pętli; nie rysują OLED ani nie korzystają z jego SPI.
- **CLOCK / TIMEZONE:** synchronizacja czasu przez MeshCore/Companion, lokalne wyświetlanie czasu, wybór strefy przyciskiem i trwały zapis profilu.
- **Auto-off OLED:** wygaszanie stron informacyjnych po bezczynności, przy zachowaniu działania MeshCore.
- **RF Monitor:** Noise Floor MeshCore, RSSI i SNR ostatniego pakietu oraz licznik odbioru, bez dodatkowego pomiaru radia.
- **Buzzer startowy:** dźwiękowa sygnalizacja uruchomienia.

Siedem normalnych stron: LOGO / STATUS / RADIO / SIGNAL / RF MON / DEVICE / CLOCK.

Inicjalizacja RF właściwa dla C6L jest częścią v1.0.0; szczegóły poniżej.

[Release v1.0.0](https://github.com/siem84/MeshCore-M5Stack-Unit-C6L-UI/releases/tag/v1.0.0)

## v1.0.1 — Preferences / SPIFFS fix

v1.0.1 zawiera wszystkie funkcje v1.0.0 i zmienia wyłącznie trwały zapis preferences / obsługę SPIFFS. Wygląd UI pozostaje bez zmian.

Preferences mogły nie zostać zapisane z powodu braku miejsca w SPIFFS. Poprzedni layout C6L miał 128 KiB SPIFFS; v1.0.1 zwiększa je do **256 KiB**, zachowując dwa sloty OTA, przez plik `variants/m5stack_unit_c6l/partitions_4mb_256k_spiffs.csv`. Testy fizyczne C6L potwierdziły zachowanie ustawień takich jak Path Hash Size po restart/cold boot. Nieudany zapis prefs jest zgłaszany i nie czyści stanu dirty.

Działający firmware nie usuwa ani celowo nie ogranicza contacts, channels i device identity w celu odzyskania miejsca. Ochrona pamięci może odzyskiwać cache surowych reklam; takie reklamy trzeba ponownie odebrać przed eksportem/udostępnieniem.

**Pierwsze przejście z v1.0.0 wymaga CLEAN INSTALL przez `-merged.bin`, ponieważ zmienił się układ partycji. CLEAN INSTALL kasuje dane urządzenia, w tym identity, contacts, channels, preferences i BLE bonding. Wcześniej zabezpiecz potrzebne dane.** Po migracji późniejsze aktualizacje mogą używać UPDATE `.bin`, o ile układ partycji ponownie się nie zmieni.

To **C6L UI v1.0.1**, a nie nowa wersja MeshCore. Oba finalne obrazy CLEAN INSTALL i UPDATE oraz poprawka trwałości ustawień zostały przetestowane na fizycznym M5Stack Unit C6L.

[Release v1.0.1](https://github.com/siem84/MeshCore-M5Stack-Unit-C6L-UI/releases/tag/v1.0.1)

## Dokumentacja techniczna

MeshCore Companion v1.17.1; base development commit `ac7d88efe72201729cc8584c2b85e1f45350d0ec`; public filename base: `v1.17.1-dev-ac7d88ef`.

Repozytorium jest nakładką źródeł C6L na ten bazowy commit, a nie kompletnym drzewem MeshCore. Skopiuj dostarczone źródła do zgodnej wersji bazowej, zachowując ścieżki.

### Inicjalizacja RF / LoRa

Unit C6L prowadzi specyficzne dla płytki sygnały sterowania RF przez PI4IOE5V6408:

| Pin ekspandera | Sygnał | Inicjalizacja |
| --- | --- | --- |
| P5 | SX_LNA_EN | HIGH włącza wymagany tor LNA |
| P6 | SX_ANT_SW | HIGH ustawia przełącznik antenowy |
| P7 | SX_NRST | LOW przez około 100 ms, następnie HIGH resetuje SX1262 |

Implementacja ustawia te piny jako wyjścia, wyłącza stan wysokiej impedancji, wykonuje reset sprzętowy, a następnie ustawia przełącznik antenowy i LNA enable na HIGH. Operacje read-modify-write zachowują SYS_KEY1/P0 i pozostałe bity ekspandera. Obie linie CS SPI są nieaktywne przed pierwszym transferem po wspólnej magistrali.

Zapewnia to stany sterujące RF wymagane do prawidłowej pracy toru odbiorczego LoRa w Unit C6L. Nie zmienia ustawień użytkownika dotyczących częstotliwości, bandwidth, spreading factor, coding rate ani mocy TX.

### Sygnalizacja RGB LED

| Kolor | Znaczenie |
| --- | --- |
| GREEN | Sygnalizacja startu / boot |
| WHITE | BLE rozłączone |
| BLUE | BLE połączone |
| RED | LoRa RX, około 150 ms |
| YELLOW | LoRa TX, około 150 ms |

Sygnalizacja RX/TX chwilowo zastępuje kolor stanu BLE. Sygnalizacja startowa nie jest pełnym autotestem sprzętu.

### OLED UI i sterowanie

Normalny cykl zawiera dokładnie **siedem ekranów**:

```text
LOGO -> STATUS -> RADIO -> SIGNAL -> RF MON -> DEVICE -> CLOCK -> LOGO
```

PAIRING i TIMEZONE są specjalnymi overlayami/trybami, a nie dodatkowymi stronami tego cyklu.

| Ekran | Zawartość |
| --- | --- |
| LOGO | Start i identyfikacja C6L UI: MESH / CORE / C6L UI. Pokazywany przez początkowy trzysekundowy okres startowy. |
| STATUS | Identyfikacja MeshCore, aktualny stan BLE, częstotliwość LoRa i liczniki pakietów RX/TX. |
| RADIO | Częstotliwość, bandwidth, spreading factor, moc TX i coding rate. |
| SIGNAL | RSSI i SNR ostatniego odebranego pakietu oraz licznik RX. Przed pierwszym odbiorem RSSI/SNR pokazują `---`. |
| RF MON | Noise Floor, RSSI/SNR ostatniego pakietu i licznik odbioru. Ekran diagnostyczny, nie laboratoryjny pomiar czułości odbiornika. |
| DEVICE | Identyfikacja urządzenia C6L, uptime i aktualny stan BLE. |
| CLOCK | Lokalna godzina HH:MM i data DD.MM.YY na podstawie czasu urządzenia synchronizowanego przez MeshCore/Companion. |

### Sterowanie przyciskiem

| Kontekst | Działanie SYS_KEY1 | Wynik |
| --- | --- | --- |
| Normalny UI | Krótkie naciśnięcie | Następny ekran |
| OLED wygaszony | Pierwsze naciśnięcie | Tylko wybudzenie aktualnej strony; cały gest jest konsumowany |
| CLOCK | Krótkie naciśnięcie | Powrót do LOGO |
| CLOCK | Przytrzymanie około 3 sekund | Wejście do TIMEZONE |
| TIMEZONE | Krótkie naciśnięcie | Wybór następnego profilu strefy |
| TIMEZONE | Przytrzymanie około 3 sekund | Zapis profilu i powrót do CLOCK |
| TIMEZONE | Około 30 sekund bezczynności | Anulowanie niezapisanego wyboru i powrót do CLOCK |

Po wybudzeniu OLED puść przycisk; następne naciśnięcie działa normalnie. PAIRING ma priorytet i blokuje zwykłe przełączanie stron.

### Auto-off OLED

LOGO, STATUS, RADIO, SIGNAL, RF MON i DEVICE wygaszają się po **60 sekundach bez aktywności przycisku**. CLOCK pozostaje włączony bez tego limitu. TIMEZONE i aktywny overlay PAIRING również utrzymują ekran włączony, z uwzględnieniem własnych timeoutów tych trybów.

Pierwsze naciśnięcie po wygaszeniu wybudza aktualny ekran bez przełączania strony.

**Wygaszenie OLED nie usypia urządzenia ani nie zatrzymuje MeshCore.** BLE, LoRa, RGB LED i pozostałe funkcje firmware nadal działają.

### Ekran parowania BLE

Gdy BLE zgłosi passkey przez `onPassKeyNotify(pass_key)`, główna pętla pokazuje:

```text
PAIRING
xxxxxx
```

Wyświetlana wartość pochodzi z tego zdarzenia i ma sześć cyfr, z zachowaniem zer wiodących. Liczba zapisanych bondingów nie decyduje o jej pokazaniu.

PAIRING wybudza OLED i ma priorytet nad normalnym UI. Po zakończeniu uwierzytelniania, jego niepowodzeniu lub rozłączeniu BLE przywracane są poprzednia strona i poprzedni stan włączenia/wygaszenia OLED. Timeout ekranu po 30 sekundach także zamyka overlay, bez anulowania uwierzytelniania BLE.

Zdarzenie podczas początkowych trzech sekund LOGO jest odroczone do końca tego okresu; kod pojawia się tylko wtedy, gdy parowanie nadal jest aktywne. Jeżeli parowanie zacznie się podczas TIMEZONE, niezapisany wybór strefy jest anulowany; po parowaniu UI wraca do CLOCK.

### CLOCK / TIMEZONE

Zsynchronizuj czas urządzenia przez MeshCore/Companion po BLE, a następnie wybierz strefę lokalnie: **CLOCK → przytrzymanie około 3 sekund → TIMEZONE → krótkie naciśnięcia wybierają profil → przytrzymanie około 3 sekund zapisuje**.

| Profil | Reguły czasu lokalnego |
| --- | --- |
| UTC | UTC, bez zmiany czasu |
| EUROPE | Czas środkowoeuropejski CET/CEST z automatyczną zmianą czasu według reguł POSIX TZ |
| US EAST | Czas wschodni EST/EDT z automatyczną zmianą czasu |
| US WEST | Czas pacyficzny PST/PDT z automatyczną zmianą czasu |
| INDIA | UTC+05:30, bez zmiany czasu |

EUROPE oznacza czas środkowoeuropejski, nie wszystkie strefy Europy. Wybrany profil zapisuje się w preferencjach i pozostaje ustawiony po restarcie. Czas urządzenia nadal jest timestampem UTC/Unix; przeliczane jest wyłącznie jego wyświetlanie.

Telefon synchronizuje czas; **nie** przesyła automatycznie profilu strefy czasowej. Wybór profilu nie wymaga rekompilacji. Gdy urządzenie nie ma poprawnego czasu, CLOCK pokazuje `NO TIME` / `SYNC BLE`.

### RF Monitor

Przykładowe wartości:

```text
RF MON
NF -88
R -112
S +6.5
RX 248
```

NF to istniejąca wartość Noise Floor MeshCore, bez korekcji dla LNA i bez nowego pomiaru. R oznacza RSSI ostatniego pakietu w dBm, S to SNR w dB, a RX to liczba odebranych pakietów. Przed pierwszym RX wartości R i S mają postać `---`. Początkowe NF równe zero jest pokazywane jako `NF 0`.

Widoczna strona odświeża się co około 750 ms. Duże liczniki RX korzystają w razie potrzeby z czytelnych skrótów k/M. Ekran nie uruchamia skanowania kanału i nie zmienia trybu odbioru radia. Noise Floor jest telemetrią diagnostyczną, a nie laboratoryjnym pomiarem czułości odbiornika.

### Obsługiwany sprzęt

**Wyłącznie M5Stack Unit C6L:**

- ESP32-C6 + SX1262
- OLED SSD1306, 64×48
- RGB LED WS2812C
- Przycisk użytkownika SYS_KEY1
- Ekspander I/O PI4IOE5V6408

## Instalacja

**Migracja z v1.0.0 / SPIFFS 128 KiB wymaga pełnego wymazania flash i CLEAN INSTALL. UPDATE nie zmienia tablicy partycji. CLEAN INSTALL kasuje dane urządzenia; wcześniej zabezpiecz potrzebne dane.**

Używaj wyłącznie obrazów dla **M5Stack Unit C6L**. Dwa pliki mają różne przeznaczenie.

### UPDATE

Do aktualizacji instalacji używającej już layoutu v1.0.1 ze SPIFFS 256 KiB. Wybierz zwykły plik non-merged:

```text
MeshCore-v1.17.1-dev-ac7d88ef-M5Stack-Unit-C6L-UI-v1.0.1.bin
```

Rozmiar: **1 537 552 bajtów**. SHA-256:

```text
518B72285AB2A864642D60E4176ACF5DE241FDBCDE6CBE3751C1E12C9F0DD559
```

Przy zwykłej aktualizacji z zachowaniem danych urządzenia nie wybieraj pełnego kasowania.

### CLEAN INSTALL

Do pełnej instalacji, także po wymazaniu urządzenia lub przy innym układzie flash:

```text
MeshCore-v1.17.1-dev-ac7d88ef-M5Stack-Unit-C6L-UI-v1.0.1-merged.bin
```

Rozmiar: **1 603 088 bajtów**. SHA-256:

```text
7AC529DBE5D66ED085D03606DE72FA0AF647CAD5F6A24E1C9EB294D20884EF7D
```

Obraz merged zawiera bootloader, tablicę partycji, początkowe dane OTA i aplikację.

> **Utrata danych:** CLEAN INSTALL z pełnym kasowaniem usuwa dotychczasowe dane urządzenia, w tym identity, kontakty, preferencje i bonding BLE. Przed kontynuowaniem zabezpiecz potrzebne dane.

### Zalecana metoda: MeshCore Web Flasher

1. Podłącz Unit C6L przez USB i otwórz [MeshCore Web Flasher](https://flasher.meshcore.io/).
2. Wybierz **Custom Firmware** i odpowiedni plik z `firmware/`.
3. Dla UPDATE wybierz plik non-merged i pozostaw kasowanie wyłączone.
4. Dla CLEAN INSTALL wybierz plik z końcówką `-merged.bin`. Zachowaj tę końcówkę: flasher rozpoznaje ją, ostrzega o kasowaniu flash i wybiera ścieżkę czystej instalacji.
5. Wybierz właściwy port urządzenia, sprawdź opcję kasowania i postępuj zgodnie z instrukcjami flashera. Nie odłączaj urządzenia do zakończenia zapisu.
6. Połącz się przez aplikację MeshCore, zsynchronizuj czas i wybierz lokalną strefę na CLOCK.

Rozpoznawanie pliku merged i obsługa kasowania znajdują się w [kodzie Web Flashera](https://github.com/meshcore-dev/flasher.meshcore.io/blob/main/flasher.js). Przy zmianie typu instalacji odśwież flasher i ponownie wybierz plik, aby nie przenieść wcześniejszego wyboru kasowania.

## Testy

C6L UI v1.0.0 oraz oba finalne publikacyjne obrazy firmware (UPDATE i CLEAN INSTALL), o dokładnych hashach SHA-256 zachowanych w Release v1.0.0, zostały przetestowane na fizycznym M5Stack Unit C6L.

Sprawdzono następujące podstawowe funkcje:

- Boot/start urządzenia.
- OLED i siedmioekranowy UI.
- Auto-off i wybudzanie OLED.
- Przycisk SYS_KEY1.
- RGB LED.
- Połączenie BLE.
- Parowanie BLE i wyświetlanie sześciocyfrowego passkey.
- LoRa RX/TX.
- Poprawną inicjalizację toru RF C6L.
- CLOCK.
- Wybór TIMEZONE i zachowanie ustawienia po restarcie.
- UPDATE przy użyciu zwykłego pliku non-merged `.bin`.
- CLEAN INSTALL przy użyciu `-merged.bin`.

To pierwsze publiczne wydanie C6L UI v1.0.0 i mogą wystąpić nieznane problemy. Zgłoszenia problemów są mile widziane; podaj sprzęt, nazwę firmware i kroki odtworzenia problemu.

C6L UI v1.0.1: trwałość preferences po restart/cold boot oraz oba finalne obrazy UPDATE i CLEAN INSTALL zostały przetestowane na fizycznym M5Stack Unit C6L.

## Autorzy i podziękowania

- [MeshCore](https://github.com/meshcore-dev/MeshCore) — oryginalny projekt i jego współtwórcy.
- Adaptacja M5Stack Unit C6L, OLED UI i dodatki specyficzne dla C6L — **siem84**.
- Publiczny maintainer tej adaptacji — **siem84**.

Jest to społecznościowa, nieoficjalna adaptacja, a nie oficjalne wydanie MeshCore.

## Licencja

Bazowy kod MeshCore jest objęty **licencją MIT**. [LICENSE](LICENSE) jest niezmienioną kopią pliku `license.txt` z repozytorium bazowego, z zachowaniem oryginalnej informacji copyright i warunków licencji.

Składniki frameworka i bibliotek zewnętrznych zachowują własne licencje. Przypisanie autorstwa adaptacji C6L nie oznacza przypisania autorstwa oryginalnego projektu MeshCore.
