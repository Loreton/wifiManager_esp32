//
// updated by ...: Loreto Notarantonio
// Date .........: 13-09-2025 17.32.34
//


#pragma once

// Ho sostituito String con array di char e aggiunto la gestione del BSSID per il roaming reale tra AP con lo stesso SSID.

#include <Arduino.h>
#include <WiFi.h>
#include <vector>

// #ifndef MAX_SSID_LENGTH
    #define MAX_SSID_LENGTH 33 // MAX_SSID_LEN è già definita altrove
    #define MAX_PASS_LENGTH 64
// #endif




class WiFiManagerNB {
    public:
        WiFiManagerNB();
        void init(uint32_t scanIntervalWhenConnected,
                  uint32_t scanIntervalWhenNotConnected,
                  uint32_t maxWifiTimeout,
                  int rssiGap);

        void update();
        void addSSID(const char* ssid, const char* password);
        bool isConnected();
        const char* getConnectedSSID();
        void printScanResults(); // Funzione di debug

    private:
        static WiFiManagerNB* s_instance;
        static void WiFiEventHandler(WiFiEvent_t event, WiFiEventInfo_t info);

        struct WifiCredential {
            char ssid[MAX_SSID_LENGTH];
            char password[MAX_PASS_LENGTH];
        };

        std::vector<WifiCredential> m_credentials;

        uint32_t m_scanIntervalWhenConnected;
        uint32_t m_scanIntervalWhenNotConnected;
        uint32_t m_maxWifiTimeout;
        uint32_t m_lastScanTime = 0;
        uint32_t m_lastConnectedTime = 0;
        uint32_t m_connectionStartTime = 0; // Ora la usiamo correttamente

        int m_rssiGap;
        char m_currentSSID[MAX_SSID_LENGTH] = {0};
        uint8_t m_currentBSSID[6] = {0}; // Per il roaming vero

        void startScan();
        void handleScanResult();
        void onWiFiEvent(WiFiEvent_t event);
};