

bool startAP() {
    WiFi.mode(WIFI_AP);    // turn on ESP8266 RF
    delay(100);

    Serial.print("Setting soft-AP ... ");
    WiFi.softAPConfig(local_IP, gateway, subnet);
       if(WiFi.softAP(ssid)) {
        Serial.println("Ready");
        WiFi.begin(ssid);
        WiFi.mode(WIFI_AP);    // turn on ESP8266 RF
        return true;
    }
    Serial.println("Failed!");
    return false;
    
}   // startAP()

