//
// updated by ...: Loreto Notarantonio
// Date .........: 13-09-2025 17.33.23
//


#include <WiFi.h>

// ---------------------------------
// lnLibrary headers files
// ---------------------------------
// #define  LOG_MODULE_LEVEL LOG_LEVEL_DEBUG
#include <lnLogger_Class.h>

// #include "wifiManager_ssid_credentials.h" // ssid definition networkd
#include "WiFiManager_Class.h"


/* ---
    - Al momento dell'init viene lanciato la scanning asyncron connectToBestNetwork()
    - nel loop() inseriamo la chiamata al metodo update() il quale verifica lo status dello scanning
    - se lo scanning è completato vengono processate le reti trovate processScanResults()
    - viene cercala la bestRSSI e si tenta la connessione
    - se va a buon fine allora dovrebbe scattare l'handleEvent() dove ho messo la callBack
---- */

// ----------------------------------------------------
// Classe WiFiManager_Class
// ----------------------------------------------------
WiFiManager_Class* WiFiManager_Class::s_instance = nullptr;

// Costruttore
WiFiManager_Class::WiFiManager_Class() {
    s_instance = this;
};

// #####################################################################
// Inizializza il WiFi in modalità Station e si connette
// #####################################################################
void WiFiManager_Class::init(Network* creds, int8_t count, bool autoReconnect) {
    m_networks     = creds;
    m_networkCount = count;
    m_autoReconnect = autoReconnect;
    start();
}




// #####################################################################
// Inizializza il WiFi in modalità Station e si connette
// #####################################################################
void WiFiManager_Class::start() {
    LOG_INFO("Inizializzazione WiFi...");
    WiFi.mode(WIFI_STA);

    // Registra la funzione di gestione degli eventi WiFi
    WiFi.onEvent(handleEvent);

    // fai partire lo scanning delle reti
    m_starting = true;
    connectToBestNetwork();
}


// #####################################################################
// # Inizializza il WiFi in modalità Station e si connette
// #####################################################################
void WiFiManager_Class::restart(void) {
    LOG_WARN("Restart() called");

    if (!m_starting) {
        m_eventCounter=0;
        disconnect();
        start();
    }

    if (m_eventCounter++ > 0) {
        LOG_WARN("WiFi already Restarting - m_eventCounter: %d", m_eventCounter);
        if (m_eventCounter > 20) {
            LOG_ERROR("Restarting  ESP32...");
            ESP.restart();
        }
    }

}



// #########################################
// # ....
// #########################################
void WiFiManager_Class::disconnect() {
    LOG_NOTIFY("Disconnecting wifi %s", WiFi.SSID());
    WiFi.scanDelete();
    m_scanning = false;
    if (WiFi.disconnect()) {
        LOG_INFO("[WIFI] Disconnected");
        delay(100);
    } else {
        LOG_ERROR("WiFi was not Connected!");
    }
    m_scanInterval = m_scanIntervalWhenNotConnected;
}


// #########################################
// # ---- valori in secondi
// #########################################
void WiFiManager_Class::setScanInterval(uint16_t whenConnected, uint16_t whenNotConnected) {
    m_scanIntervalWhenConnected = whenConnected*1000UL;
    m_scanIntervalWhenNotConnected = whenNotConnected*1000UL;
}

// #####################################################################
// Funzione da chiamare nel loop principale per monitorare la connessione
// Se non siamo connessi o è il momento di scansionare nuovamente, avvia la scansione
// #####################################################################
void WiFiManager_Class::update() {

    m_scanInterval = ( WiFi.status() == WL_CONNECTED) ? m_scanIntervalWhenConnected : m_scanIntervalWhenNotConnected;  // velocizziamo l'intervallo se disconnessi

    uint32_t scanElapsed = millis() - m_lastScanTime;

    if ( (WiFi.status() != WL_CONNECTED || scanElapsed > m_scanInterval ) && !m_scanning) {
        if ( scanElapsed > m_scanInterval) {
            if (WiFi.status() != WL_CONNECTED)   {LOG_ERROR("WiFi - connessione non attiva."); }
            if (scanElapsed > m_scanInterval)    {LOG_NOTIFY("WiFi - Intervallo di scansione periodica (%s) raggiunto", lnLog.msecToHMS(m_scanInterval, fMilliSecondsTrue)); }
            connectToBestNetwork();
            m_lastScanTime = millis();
        }
        else {
            if (scanElapsed % 60000UL < 100) {
                LOG_NOTIFY("scanElapsed/m_scanInterval is not expired: (%lu/%lu)", scanElapsed,  m_scanInterval);
            }
        }
    }

    // Se la scansione è in corso, controlla se è terminata
    if (m_scanning) {
        int scanResult = WiFi.scanComplete();
        if (scanResult >= 0) { // La scansione è completata
            m_scanning = false;
            processScanResults(scanResult);
        }
    }

}




// #####################################################################
// Avvia una scansione non bloccante
// #####################################################################
void WiFiManager_Class::connectToBestNetwork() {
    if (!m_scanning) {
        m_scanning = true;
        LOG_NOTIFY("Avvio scansione reti (asincrona)...");
        WiFi.scanNetworks(true); // Scansione non bloccante
    }
}




// #####################################################################
// Elabora i risultati della scansione e si connette alla rete migliore
// #####################################################################
void WiFiManager_Class::processScanResults(int networks) {
    int8_t bestRSSI = -127; // Valore RSSI minimo
    int8_t bestNetworkIndex = -1;

    if (networks == 0) {
        LOG_WARN("Nessuna rete trovata.");
        return;
    }


    // Cerca la rete migliore tra quelle configurate
    LOG_NOTIFY("reti trovate: %d", networks);
    for (int i = 0; i < networks; ++i) {
        LOG_INFO("  %d: SSID: %-12s BSSID: %s (%d dBm)", i + 1, WiFi.SSID(i).c_str(), WiFi.BSSIDstr(i).c_str(), WiFi.RSSI(i));
        for (int j = 0; j < m_networkCount; ++j) {
            if (strcmp(WiFi.SSID(i).c_str(), m_networks[j].ssid) == 0) {
                if (WiFi.RSSI(i) > bestRSSI) {
                    bestRSSI = WiFi.RSSI(i);
                    bestNetworkIndex = j;
                }
                break;
            }
        }
    }


    // --- non cambiamo se il gap di livello è inferiore a 10
    int8_t currRSSI = WiFi.RSSI();
    if ( (currRSSI == 0) || (bestRSSI - currRSSI >= m_rssiGap) ) {
        connectToSSID(bestNetworkIndex);
    } else {
        LOG_NOTIFY("WiFi Status");
        LOG_INFO("   currSSID: %-12s (%d dBm)", WiFi.SSID().c_str(), WiFi.RSSI());
        LOG_INFO("   newSSID:  %-12s (%d dBm)", WiFi.SSID(bestNetworkIndex), bestRSSI);  // il nome lo prelevo dalla mia struttura che è char*
        LOG_INFO("   RSSI gap is less than %ddBm. Mantaining current SSID", m_rssiGap);
    }
}





// #####################################################################
// #
// #####################################################################
void WiFiManager_Class::connectToSSID(int8_t networkIndex) {
    if (networkIndex == -1) {
        LOG_ERROR("Nessuna delle reti configurate...:");
        for (int i = 0; i < m_networkCount; i++) {
            LOG_INFO("  [%d] - %s", i, m_networks[i].ssid);
        }
        LOG_ERROR("  ...è stata trovata.");
    } else {
        const char *ssid     = m_networks[networkIndex].ssid;
        const char *password = m_networks[networkIndex].password;
        LOG_NOTIFY("best net: [%d] - %s", networkIndex, ssid);

        // Controlla se siamo già connessi alla rete migliore
        // if (WiFi.status() == WL_CONNECTED && String(WiFi.SSID()) == String(ssid)) {
        if (WiFi.status() == WL_CONNECTED && (strcmp(WiFi.SSID().c_str(), ssid) == 0) ) {
            LOG_NOTIFY("Già connesso alla rete migliore: %s - %s.", WiFi.SSID(), WiFi.BSSIDstr().c_str());
            LOG_NOTIFY("...non è necessario cambiare.");
        } else {
            LOG_INFO("Connessione a: %s", ssid);
            WiFi.begin(ssid, password);
        }
        m_scanning = false;
        m_starting = false;
        m_eventCounter=0;
    }

}



// #####################################################################
// #
// #####################################################################
void WiFiManager_Class::showCurrentConnection() {
#if LOG_MODULE_LEVEL >= LOG_LEVEL_SPEC
    char buffer[16];
    const char *ptr;

    if (WiFi.status() == WL_CONNECTED) {
        LOG_NOTIFY("Connected to:     %s", WiFi.SSID());
        LOG_NOTIFY("\tBSSID:          %s", WiFi.BSSIDstr().c_str());
        LOG_NOTIFY("\tRSSI:           %ld", WiFi.RSSI());
        LOG_NOTIFY("\tCHANNEL:        %ld", WiFi.channel());
        LOG_NOTIFY("\tIP:             %s",  WiFi.localIP().toString().c_str());
    }
    else {
        LOG_ERROR("WiFi is not connected!");
    }
    uint32_t scanElapsed = millis() - m_lastScanTime;

    ptr = lnLog.msecToHMS(buffer, 16, (m_scanInterval-scanElapsed), fMilliSecondsTrue, fstripHoursTrue);
    LOG_NOTIFY("\tNext Scan:      %s - %7lu", ptr,  (m_scanInterval-scanElapsed));

    ptr = lnLog.msecToHMS(buffer, 16, m_scanInterval, fMilliSecondsTrue, fstripHoursTrue);
    LOG_NOTIFY("\tScan interval:  %s - %7lu", ptr, m_scanInterval);

    ptr = lnLog.msecToHMS(buffer, 16, scanElapsed, fMilliSecondsTrue, fstripHoursTrue);
    LOG_NOTIFY("\tScan elapsed:   %s - %7lu", ptr, scanElapsed);

    LOG_NOTIFY("\tis scanning:    %d", m_scanning);
    LOG_NOTIFY("\tis starting:    %d", m_starting);
#endif
}




// #####################################################################
// # Funzione statica per la gestione degli eventi Wi-Fi
// #####################################################################
void WiFiManager_Class::handleEvent(arduino_event_id_t event) {
    if (s_instance) {
        switch (event) {

            case ARDUINO_EVENT_WIFI_STA_GOT_IP:
                LOG_INFO("Connesso! %s - %s - %s", WiFi.SSID(), WiFi.BSSIDstr().c_str(), WiFi.localIP().toString().c_str());
                s_instance->m_starting = false;
                break;

            case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
                LOG_ERROR("WiFi - Connessione persa.");
                if (s_instance->m_autoReconnect) {
                    s_instance->restart();
                }
                break;
            // Aggiungi altri eventi se necessario...
        }

        if (s_instance->m_onConnectCallback) {
            s_instance->m_onConnectCallback(event);
        }
    }
}



