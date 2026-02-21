//
// updated by ...: Loreto Notarantonio
// Date .........: 13-09-2025 17.32.34
//


#pragma once
    #include <WiFi.h>
    #include <esp_wifi.h>



    // ---------------------------------
    // ssid definition networkd
    // ---------------------------------
    #include "wifiManager_ssid_credentials.h" // ssid definition networkd


    #define WIFI_NO_AUTO_RECONNECT 0
    #define WIFI_AUTO_RECONNECT    1


    // CallBack per gestire connessione e disconnessione
    // passiamo come parametro leventID (WiFiGeneric.h)
    // vedi funzione handleEvent()
    // mi è utile per attivare NTP o meno
    typedef void (*ConnectCallback)(arduino_event_id_t event);

    // ----------------------------------------------------
    // Classe WiFiManager_Class
    // ----------------------------------------------------
    class WiFiManager_Class {
        private:
            Network*            m_networks;
            int8_t              m_networkCount;
            bool                m_neverConnected=true;
            bool                m_autoReconnect=true;

                // --- teniamo conto del tempo di disconnessione. S
                // --- se supera m_maxWifiTimeout allora facciamo il restart dell'ESP32
            // uint32_t            m_disconnectionStartTime = 0;
            const uint32_t      m_maxWifiTimeout = 1*60*1000UL; // Timeout massimo senza connessione (5 minuti)
            // uint32_t            m_disconnectedTime = 1*60*1000UL; // tempo di disconnessione.... 1 minuto

                // --- Intervallo di scansione della rete (in ms), qui 50 minuti
            // const uint32_t      m_scanInterval = 1*60*1000;
            uint32_t            m_scanInterval = 10*60*1000UL;
            uint32_t            m_scanIntervalWhenConnected = 10*60*1000UL;
            uint32_t            m_scanIntervalWhenNotConnected = 1*60*1000UL;
            uint32_t            m_lastScanTime = 0;
            bool                m_scanning;
            bool                m_starting = true;
            uint8_t             m_eventCounter = 0;
            int8_t              m_rssiGap = 7;

            ConnectCallback     m_onConnectCallback = nullptr;
            // MillisTimer         m_disconnectedTimer;


            static WiFiManager_Class* s_instance; // Istanza statica per la gestione degli eventi

        public:
            // Costruttore
            WiFiManager_Class(void);

            // Inizializza il WiFi in modalità Station e si connette
            void init(Network* creds, int8_t count, bool autoReconnect=true);
            void start(void);
            void restart(void);
            void disconnect(void);
            // uint32_t disconnectedElapsed(void) const { return m_disconnectionStartTime; };

            // Funzione da chiamare nel loop principale per monitorare la connessione
            void update();
            void processScanResults(int n);
            bool isConnected(void) {return WiFi.status() == WL_CONNECTED; };

            void setScanInterval(uint16_t whenConnected=10*60, uint16_t whenNotConnected=1*60); // seconds
            void showCurrentConnection(void);

            // Imposta la funzione di callback da chiamare in caso di connessione
            void setConnectCallback(ConnectCallback callback) { m_onConnectCallback = callback; }
            void connectToBestNetwork();

            // # --- di comodo
            const char * ipaddress(void) const {return WiFi.localIP().toString().c_str();}
            const char * ssid(void)      const {return WiFi.SSID().c_str();}
            const char * bssid(void)     const {return WiFi.BSSIDstr().c_str();}
            int8_t       rssi(void)      const {return WiFi.RSSI();}



        private:
            // Scansiona le reti e si connette a quella migliore (RSSI più alto)
            // void checkOutOfService();
            void connectToSSID(int8_t networkIndex=-1);
            static void handleEvent(arduino_event_id_t event);
    };

