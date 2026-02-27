//
// updated by ...: Loreto Notarantonio
// Date .........: 20-02-2026 17.16.03
//


// #include "lnLogger_LEVELS.h"
// #define LOG_MODULE_LEVEL LOG_LEVEL_DEBUG
#include "lnLogger_Class.h"

#include "WiFiManager.h"


WiFiManagerNB* WiFiManagerNB::s_instance = nullptr;

WiFiManagerNB::WiFiManagerNB() {
    s_instance = this;
}



// #######################################################################################################
// # WiFi.persistent(false):
// #    Inserito in init(). Senza questo, ogni volta che chiami WiFi.begin(),
// #    l'ESP32 scrive le credenziali nella memoria Flash (NVS). La Flash ha cicli di scrittura limitati;
// #    disabilitandolo preservi il chip.
// #######################################################################################################
// void WiFiManagerNB::init(uint32_t scanIntervalWhenConnected, uint32_t scanIntervalWhenNotConnected, uint32_t maxWifiTimeout, int rssiGap) {
//     m_scanIntervalWhenConnected = scanIntervalWhenConnected;
//     m_scanIntervalWhenNotConnected = scanIntervalWhenNotConnected;
//     m_maxWifiTimeout = maxWifiTimeout;
//     m_rssiGap = rssiGap;

void WiFiManagerNB::init(uint16_t scanIntervalWhenConnected, uint16_t scanIntervalWhenNotConnected, uint16_t maxWifiTimeout, int8_t rssiGap) {
    m_scanIntervalWhenConnected = scanIntervalWhenConnected*1000;
    m_scanIntervalWhenNotConnected = scanIntervalWhenNotConnected*1000;
    m_maxWifiTimeout = maxWifiTimeout*1000;
    m_rssiGap = rssiGap;

    WiFi.mode(WIFI_STA);
    WiFi.persistent(false); // [Punto 5] Evita usura Flash
    WiFi.disconnect(true);

    WiFi.onEvent(WiFiEventHandler);

    m_lastConnectedTime = millis(); // [Punto D] Inizializzazione corretta
    startScan();
    // Serial.printf("LOG_MODULE_LEVEL: %d", LOG_MODULE_LEVEL);
    // Serial.printf("LOG_LEVEL_INFO: %d", LOG_LEVEL_INFO);
}

void WiFiManagerNB::addSSID(const char* ssid, const char* password) {
    WifiCredential cred;
    strncpy(cred.ssid, ssid, MAX_SSID_LENGTH - 1);
    strncpy(cred.password, password, MAX_PASS_LENGTH - 1);
    m_credentials.push_back(cred);
}



void WiFiManagerNB::update() {
    uint32_t now = millis();
    wl_status_t status = WiFi.status();

    // Se non siamo connessi, verifichiamo il timeout totale (Punto D)
    if (status != WL_CONNECTED) {
        if (now - m_lastConnectedTime > m_maxWifiTimeout) {
            lnLOG_WARNING("WiFi: Max timeout reached. Forcing new scan.");
            m_lastConnectedTime = now;
            startScan();
            return;
        }
    } else {
        m_lastConnectedTime = now;
    }

    // --- LOGICA TIMEOUT CONNESSIONE ---
    // Se abbiamo appena lanciato un WiFi.begin(), aspettiamo 10s prima di scansionare ancora
    if (status != WL_CONNECTED && m_connectionStartTime > 0) {
        if (now - m_connectionStartTime < 10000) {
            return; // Troppo presto, attendi che il tentativo finisca
        }
    }

    // Timer scansione periodica
    uint32_t interval = (status == WL_CONNECTED) ? m_scanIntervalWhenConnected : m_scanIntervalWhenNotConnected;
    if (now - m_lastScanTime >= interval) {
        startScan();
    }

    if (WiFi.scanComplete() >= 0) {
        handleScanResult();
        WiFi.scanDelete();
    }
}



// ##################################################################################################################
// # Il timer m_lastScanTime viene aggiornato solo quando la scansione viene effettivamente lanciata.
// ##################################################################################################################
void WiFiManagerNB::startScan() {
    // [Punto A] Avvia la scansione solo se non ce n'è una in corso
    if (WiFi.scanComplete() == -2) {
        // Serial.println("WiFi: Starting async scan...");
        lnLOG_NOTIFY("WiFi: Starting async scan...");
        WiFi.scanNetworks(true);
        m_lastScanTime = millis(); // Aggiorna il timer qui
    }
}



void WiFiManagerNB::handleScanResult() {
    int n = WiFi.scanComplete();
    if (n <= 0) return;

    // DEBUG: Mostra cosa abbiamo trovato
    printScanResults();

    int bestRSSI = -1000;
    int bestIdx = -1;
    const char* bestPassword = nullptr;

    for (int i = 0; i < n; ++i) {
        for (auto &cred : m_credentials) {
            if (strcmp(WiFi.SSID(i).c_str(), cred.ssid) == 0) {
                if (WiFi.RSSI(i) > bestRSSI) {
                    bestRSSI = WiFi.RSSI(i);
                    bestIdx = i;
                    bestPassword = cred.password;
                }
            }
        }
    }

    if (bestIdx == -1) return;

    if (WiFi.status() == WL_CONNECTED) {
        int currentRSSI = WiFi.RSSI();

        // --- FIX SICUREZZA ---
        uint8_t* currentBSSID = WiFi.BSSID();
        uint8_t* targetBSSID = WiFi.BSSID(bestIdx);

        // Se uno dei due è nullo, non possiamo confrontarli in sicurezza
        if (currentBSSID != nullptr && targetBSSID != nullptr) {
            if (memcmp(targetBSSID, currentBSSID, 6) == 0) {
                // Siamo già sull'AP migliore di questa rete
                return;
            }
        }

        // Controllo Gap
        if ((bestRSSI - currentRSSI) < m_rssiGap) {
            return;
        }

        // Serial.printf("WiFi: Switching to better AP (%s) RSSI: %d (Gap: %d)\n",
        //               WiFi.SSID(bestIdx).c_str(), bestRSSI, bestRSSI - currentRSSI);
        lnLOG_INFO("WiFi: Switching to better AP (%s) RSSI: %d (Gap: %d)",
                      WiFi.SSID(bestIdx).c_str(), bestRSSI, bestRSSI - currentRSSI);
    }

    // Aggiorna e connetti
    strncpy(m_currentSSID, WiFi.SSID(bestIdx).c_str(), MAX_SSID_LEN - 1);
    WiFi.begin(m_currentSSID, bestPassword);
}




void WiFiManagerNB::WiFiEventHandler(WiFiEvent_t event, WiFiEventInfo_t info) {

    if (s_instance != nullptr) {
        // Inoltra l'evento alla funzione di istanza (se vuoi gestirli lì)
        // s_instance->onWiFiEvent(event);

        // Oppure gestisci direttamente qui i log comuni:
        const char* eventName = "UNKNOWN";
        switch (event) {
            case ARDUINO_EVENT_WIFI_READY:           eventName = "WIFI_READY"; break;
            case ARDUINO_EVENT_WIFI_SCAN_DONE:       eventName = "SCAN_DONE"; break;
            case ARDUINO_EVENT_WIFI_STA_START:       eventName = "STA_START"; break;
            case ARDUINO_EVENT_WIFI_STA_STOP:        eventName = "STA_STOP"; break;
            case ARDUINO_EVENT_WIFI_STA_CONNECTED:   eventName = "STA_CONNECTED"; break;
            case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:eventName = "STA_DISCONNECTED"; break;
            case ARDUINO_EVENT_WIFI_STA_GOT_IP:      eventName = "STA_GOT_IP"; break;
            case ARDUINO_EVENT_WIFI_STA_LOST_IP:     eventName = "STA_LOST_IP"; break;
            default: break;
        }

        // Serial.printf("WiFi Event: %s (%d)\n", eventName, (int)event);
        lnLOG_NOTIFY("WiFi Event: %s (%d)", eventName, (int)event);
        if (event == ARDUINO_EVENT_WIFI_STA_GOT_IP) {
            // Serial.print("WiFi: Got IP ");
            // Serial.println(WiFi.localIP());
            lnLOG_INFO("WiFi: Got IP %s", WiFi.localIP().toString().c_str());
        }
    }
}


// Questa versione confronta ogni rete trovata con quelle nella tua lista m_credentials. S
// e c'è un match, aggiunge un indicatore visivo.
void WiFiManagerNB::printScanResults() {
    int n = WiFi.scanComplete();
    if (n < 0) return;

    // Serial.println("\n--- WiFi Scan Results ---");
    lnLOG_DEBUG("--- WiFi Scan Results ---");
    for (int i = 0; i < n; ++i) {
        bool isSaved = false;
        for (auto &cred : m_credentials) {
            if (WiFi.SSID(i) == cred.ssid) {
                isSaved = true;
                break;
            }
        }

        // Serial.printf("%s %-20s RSSI: %d dBm %s\n",
        lnLOG_DEBUG("%s %-20s RSSI: %d dBm %s",
            isSaved ? "[*]" : "[ ]",      // Asterisco se la rete è salvata
            WiFi.SSID(i).c_str(),
            WiFi.RSSI(i),
            (WiFi.status() == WL_CONNECTED && WiFi.SSID(i) == WiFi.SSID()) ? "<-- ACTIVE" : ""
        );
    }
    // Serial.println("--------------------------\n");
    lnLOG_INFO("--------------------------");
}

bool WiFiManagerNB::isConnected() { return WiFi.status() == WL_CONNECTED; }
const char* WiFiManagerNB::getConnectedSSID() { return m_currentSSID; }