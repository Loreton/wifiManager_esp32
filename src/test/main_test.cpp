//
// updated by ...: Loreto Notarantonio
// Date .........: 20-02-2026 17.54.06
//



#include <Arduino.h>


// --- Project
#define  __I_AM_MAIN_CPP__
#include "WiFiManager.h"
// #include "lnLogger_Class.h"

// --- CREDENTIALS
#include <esp32_ssid_credentials.h>
// #include <lnEsp32Orto_Bot.h>
// #include <ssid_casetta.h>

// const char* ssid = casettaSSID;
// const char* password = casettaPassword;
// #define BOTtoken lnEsp32Orto_token
// #define BOTchatid lnEsp32Orto_chatid
//
WiFiManagerNB wifiManager;

void setup() {
    Serial.begin(115200);

    for (int8_t i; i<loretoNetworksCount; i++) {
        // wifiManager.addSSID("SSID2", "PASSWORD2");
        wifiManager.addSSID(loretoNetworks[i].ssid, loretoNetworks[i].password);
        // const char *ssid =    loretoNetworks[i].ssid;
        // const char *password = loretoNetworks[i].password;
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