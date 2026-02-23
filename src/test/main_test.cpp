//
// updated by ...: Loreto Notarantonio
// Date .........: 20-02-2026 17.54.06
//



#include <Arduino.h>


// --- Project
#define  __I_AM_MAIN_CPP__
#include "WiFiManager.h"
#include "lnLogger_Class.h"

// --- CREDENTIALS
#include <esp32_ssid_credentials.h>

//
WiFiManagerNB wifiManager;

void setup() {
    Serial.begin(115200);

    // - prima dell'init()
    for (int8_t i = 0; i < loretoNetworksCount; i++) {
        wifiManager.addSSID(loretoNetworks[i].ssid, loretoNetworks[i].password);
    }

    wifiManager.init(
        30000,   // scan ogni 30s se connesso
        10000,   // scan ogni 10s se non connesso
        60000,  // timeout max 5 minuti (5*60*1000)
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