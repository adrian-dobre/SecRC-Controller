#include "DeviceConfiguration.h"

bool DeviceConfiguration::alreadyInited = false;
bool DeviceConfiguration::doubleResetDetected = false;
Preferences DeviceConfiguration::preferences;

void doubleResetTimeout(void* param) {
    while (millis() < 20000) {
        continue;
    }
    DeviceConfiguration::preferences.begin("device");
    DeviceConfiguration::preferences.putBool("reset", false);
    DeviceConfiguration::preferences.end();
    vTaskDelete(NULL);
}

void DeviceConfiguration::init() {
    if (!DeviceConfiguration::alreadyInited) {
        alreadyInited = true;
        doubleResetDetected = false;
        preferences.begin("device");
        bool deviceReset = preferences.getBool("reset", false);
        if (deviceReset) {
            doubleResetDetected = true;
            preferences.putBool("reset", false);
            preferences.end();
        } else {
            preferences.putBool("reset", true);
            preferences.end();
            xTaskCreate(doubleResetTimeout, "Double Reset Timeout", 2 * 1024,
                        NULL, 1, NULL);
        }
        Serial.print("Double Reset Detected: ");
        Serial.println(doubleResetDetected ? "yes" : "no");
    }
}

WiFiConfiguration DeviceConfiguration::getWiFiConfiguration() {
    WiFiConfiguration configuration;
    preferences.begin("network-config");
    configuration.ssid = preferences.getString("ssid", "");
    configuration.password = preferences.getString("password", "");
    configuration.accessKey = preferences.getString("accessKey", "");
    preferences.end();
    return configuration;
}

void DeviceConfiguration::saveWiFiConfiguration(
    WiFiConfiguration wiFiConfiguration) {
    preferences.begin("network-config");
    preferences.putString("ssid", wiFiConfiguration.ssid);
    preferences.putString("password", wiFiConfiguration.password);
    preferences.putString("accessKey", wiFiConfiguration.accessKey);
    preferences.end();
}

void DeviceConfiguration::reset() {
    const char* namespaces[] = {"network-config", "device"};
    for (int i = 0; i <= 2; i++) {
        preferences.begin(namespaces[i]);
        preferences.clear();
        preferences.end();
    }
}