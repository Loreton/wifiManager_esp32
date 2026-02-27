//
// updated by ...: Loreto Notarantonio
// Date .........: 20-02-2026 17.54.06
//



#include <Arduino.h>

// #define LOG_MODULE_LEVEL LOG_MODULE_INFO
#include "lnLogger_Class.h"

// --- Project
#define  __I_AM_MAIN_CPP__
#include "WiFiManager.h"


// --- CREDENTIALS
#include <esp32_ssid_credentials.h>

//
WiFiManagerNB wifiManager;

void setup() {
    Serial.begin(115200);
    lnLog.init(128, 20);  // line_buffer_len, filename_buffer_len

    // - prima dell'init()
    for (int8_t i = 0; i < loretoNetworksCount; i++) {
        wifiManager.addSSID(loretoNetworks[i].ssid, loretoNetworks[i].password);
    }

    wifiManager.init(
        60,   // scan ogni 60s se connesso
        30,   // scan ogni 30s se non connesso
        5*60,  // timeout max 5 minuti (5*60)
        8        // rssi gap
    );
}

void loop() {
    wifiManager.update();

    // if (wifiManager.isConnected()) {
    //     Serial.println("Connected to: " + wifiManager.getConnectedSSID());
    // }
    delay(100);
}