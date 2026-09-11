[English](README.md) | Polski

# MeshCore C6L UI

MeshCore C6L UI to społecznościowa, nieoficjalna adaptacja MeshCore Companion dla **M5Stack Unit C6L**. Uruchamia i integruje wbudowany OLED, RGB LED, przycisk użytkownika i lokalny interfejs, ekran parowania BLE, zegar z wyborem strefy czasowej, RF Monitor oraz inicjalizację toru RF właściwą dla tej płytki.

Pakiet zawiera dziesięć plików źródłowych adaptacji C6L ze strukturą ścieżek względem MeshCore oraz dwa obrazy firmware w `firmware/`. Jest to zestaw plików nakładanych na podaną niżej wersję bazową, a nie kompletne, samodzielne drzewo źródeł MeshCore.

## Wersja bazowa

| Składnik | Wersja |
| --- | --- |
| MeshCore Companion | v1.17.1 |
| Bazowy commit development | `ac7d88ef` |
| C6L UI | v1.0.0 |

Bazą jest kod development **po** tagu `companion-v1.17.1`, z commita `ac7d88efe72201729cc8584c2b85e1f45350d0ec` (`companion-v1.17.1-66-gac7d88ef`). Dlatego nazwy firmware zawierają `v1.17.1-dev-ac7d88ef`; nie oznaczają niezmodyfikowanego, oficjalnego wydania v1.17.1.

## Funkcje / Zmiany

- **OLED 64×48:** obsługa wbudowanego SSD1306, współdzielącego SPI z SX1262, oraz kompletny lokalny interfejs urządzenia.
- **RGB LED:** obsługa wbudowanej WS2812C na GPIO2, wskazującej start, stan połączenia BLE i aktywność LoRa.
- **Przycisk użytkownika:** SYS_KEY1 przez ekspander PI4IOE5V6408; nawigacja po ekranach, obsługa CLOCK/TIMEZONE i wybudzanie OLED.
- **Parowanie BLE:** overlay PAIRING wywoływany zdarzeniem, pokazujący rzeczywisty sześciocyfrowy passkey. Callbacki BLE przekazują stan do głównej pętli; nie rysują OLED ani nie korzystają z jego SPI.
- **CLOCK / TIMEZONE:** synchronizacja czasu przez MeshCore/Companion, lokalne wyświetlanie czasu, wybór strefy przyciskiem i trwały zapis profilu.
- **Auto-off OLED:** wygaszanie stron informacyjnych po bezczynności, przy zachowaniu działania MeshCore.
- **RF Monitor:** Noise Floor MeshCore, RSSI i SNR ostatniego pakietu oraz licznik odbioru, bez dodatkowego pomiaru radia.
- **Buzzer startowy:** dźwiękowa sygnalizacja uruchomienia.

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

## OLED UI i sterowanie

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

## Instalacja

Używaj wyłącznie obrazów dla **M5Stack Unit C6L**. Dwa pliki mają różne przeznaczenie.

### UPDATE

Do aktualizacji istniejącej instalacji z poprawnym układem flash MeshCore. Wybierz zwykły plik non-merged:

```text
MeshCore-v1.17.1-dev-ac7d88ef-M5Stack-Unit-C6L-UI-v1.0.0.bin
```

Rozmiar: **1 534 576 bajtów**. SHA-256:

```text
08E910E777B7F3B521F7576951C84603F35E7EBD8A501D7F9FDF28357F6F33ED
```

Przy zwykłej aktualizacji z zachowaniem danych urządzenia nie wybieraj pełnego kasowania.

### CLEAN INSTALL

Do pełnej instalacji, także po wymazaniu urządzenia lub przy innym układzie flash:

```text
MeshCore-v1.17.1-dev-ac7d88ef-M5Stack-Unit-C6L-UI-v1.0.0-merged.bin
```

Rozmiar: **1 600 112 bajtów**. SHA-256:

```text
7EC5F690CF4E0A1E7E4E951282BA4ACC620E92BA272DA68C7D50D8C9366B58CD
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

## Obsługiwany sprzęt

**Wyłącznie M5Stack Unit C6L:**

- ESP32-C6 + SX1262
- OLED SSD1306, 64×48
- RGB LED WS2812C
- Przycisk użytkownika SYS_KEY1
- Ekspander I/O PI4IOE5V6408

## Status / Testy

C6L UI v1.0.0 oraz oba finalne publikacyjne obrazy firmware (UPDATE i CLEAN INSTALL), o dokładnych hashach SHA-256 podanych w sekcji Instalacja, zostały przetestowane na fizycznym M5Stack Unit C6L.

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

## Autorzy i podziękowania

- [MeshCore](https://github.com/meshcore-dev/MeshCore) — oryginalny projekt i jego współtwórcy.
- Adaptacja M5Stack Unit C6L, OLED UI i dodatki specyficzne dla C6L — **siem84**.
- Publiczny maintainer tej adaptacji — **siem84**.

Jest to społecznościowa, nieoficjalna adaptacja, a nie oficjalne wydanie MeshCore.

## Licencja

Bazowy kod MeshCore jest objęty **licencją MIT**. [LICENSE](LICENSE) jest niezmienioną kopią pliku `license.txt` z repozytorium bazowego, z zachowaniem oryginalnej informacji copyright i warunków licencji.

Składniki frameworka i bibliotek zewnętrznych zachowują własne licencje. Przypisanie autorstwa adaptacji C6L nie oznacza przypisania autorstwa oryginalnego projektu MeshCore.
