# ESP32 WiFiManagerNB (Non-Blocking)

Libreria per la gestione della connessione WiFi su **ESP32** progettata per essere completamente **non-blocking**. Supporta la memorizzazione di più credenziali (Multi-SSID), il roaming intelligente tra Access Point basato sulla potenza del segnale (RSSI) e l'identificazione univoca tramite BSSID.

## 🚀 Caratteristiche principali

- **Multi-SSID**: Gestisce una lista di reti conosciute e si connette alla migliore disponibile.
- **Non-Blocking**: Tutte le operazioni di scansione e connessione avvengono in modo asincrono, senza bloccare il `loop()` principale.
- **Smart Roaming**: Passa automaticamente a un Access Point con segnale migliore se la differenza di RSSI supera una soglia configurabile (`rssiGap`).
- **BSSID Aware**: Distingue tra diversi AP con lo stesso SSID (mesh/ripetitori) per garantire la connessione al nodo più forte.
- **Preservazione Flash**: Disabilita `WiFi.persistent(false)` per evitare scritture inutili sulla memoria flash del chip ad ogni connessione.
- **Logging Integrato**: Utilizza la libreria `lnLogger_esp32` per un monitoraggio dettagliato degli eventi di rete.

---

## 🛠 Configurazione PlatformIO

Per utilizzare questa libreria nel tuo progetto PlatformIO, assicurati di avere le dipendenze corrette nel file `platformio.ini`:

```ini
lib_deps =
    https://github.com/Loreton/lnLogger_esp32.git#v1.1.5

build_flags =
    -I /percorso/alle/tue/credenziali  # Per includere ssid.h se esterno
```

---

## 💻 Esempio d'uso

```cpp
#include <Arduino.h>
#include "WiFiManager.h"
#include "lnLogger_Class.h"

WiFiManagerNB wifiManager;

void setup() {
    Serial.begin(115200);
    lnLog.init(128, 20); // Inizializzazione logger

    // 1. Aggiungi le reti conosciute
    wifiManager.addSSID("MiaRete_Casa", "password123");
    wifiManager.addSSID("Ufficio_WiFi", "segreto456");

    // 2. Inizializza il manager
    wifiManager.init(
        60,   // scanIntervalWhenConnected (secondi): controlla se esiste un AP migliore ogni 60s
        30,   // scanIntervalWhenNotConnected (secondi): scansiona ogni 30s se disconnesso
        300,  // maxWifiTimeout (secondi): forza una nuova scansione dopo 5 minuti di offline
        8     // rssiGap (dB): cambia AP solo se il nuovo segnale è migliore di almeno 8dB
    );
}

void loop() {
    // 3. Mantieni aggiornato lo stato del WiFi (necessario per logica asincrona)
    wifiManager.update();

    if (wifiManager.isConnected()) {
        // Il tuo codice...
    }
}
```

---

## 📖 Documentazione API

### `void init(uint16_t scanConnected, uint16_t scanDisconnected, uint16_t timeout, int8_t rssiGap)`
Inizializza il modulo e imposta le temporizzazioni.
- `scanConnected`: Intervallo tra le scansioni quando il dispositivo è già connesso (per roaming).
- `scanDisconnected`: Intervallo tra le scansioni quando il dispositivo è in cerca di rete.
- `timeout`: Tempo massimo dopo il quale viene resettata la logica di connessione.
- `rssiGap`: Differenza minima di segnale per forzare il passaggio a un altro AP.

### `void addSSID(const char* ssid, const char* password)`
Aggiunge una coppia di credenziali alla lista interna.

### `void update()`
Gestisce la macchina a stati interna. Deve essere chiamata ad ogni iterazione del `loop()`.

### `bool isConnected()`
Ritorna `true` se il dispositivo è connesso a un AP e ha ottenuto un indirizzo IP.

### `const char* getConnectedSSID()`
Ritorna il nome della rete (SSID) a cui si è attualmente connessi.

---

## 📂 Struttura del Progetto

- `src/WiFiManager.h`: Definizione della classe e strutture dati.
- `src/WiFiManager.cpp`: Implementazione della logica di scansione, roaming ed eventi WiFi.
- `test/wifiManager_test.cpp`: Esempio completo di implementazione.

## 📝 Note Tecniche
Il sistema utilizza gli eventi nativi dell'SDK Espressif (`WiFiEvent_t`) per gestire in modo reattivo la perdita di connessione o l'ottenimento dell'IP, riducendo al minimo il polling manuale.

---
**Autore:** Loreto Notarantonio
**Data Ultimo Aggiornamento:** 20-02-2026