//
// updated by ...: Loreto Notarantonio
// Date .........: 13-09-2025 17.32.34
//


#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <vector>

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
        String getConnectedSSID();

    private:
        static WiFiManagerNB* s_instance;
        static void WiFiEventHandler(WiFiEvent_t event, WiFiEventInfo_t info);

        struct WifiCredential {
            String ssid;
            String password;
        };

        std::vector<WifiCredential> m_credentials;

        uint32_t m_scanIntervalWhenConnected;
        uint32_t m_scanIntervalWhenNotConnected;
        uint32_t m_maxWifiTimeout;

        uint32_t m_lastScanTime = 0;
        uint32_t m_lastConnectedTime = 0;

        int m_rssiGap;
        String m_currentSSID;

        void startScan();
        void handleScanResult();
        void connectToBestNetwork();
        void onWiFiEvent(WiFiEvent_t event);
};

