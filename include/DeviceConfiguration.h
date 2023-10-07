#pragma once
#include <Preferences.h>
#include <WiFi.h>

struct WiFiConfiguration {
    String ssid;
    String password;
    String accessKey;
};

class DeviceConfiguration {
   private:
    static bool alreadyInited;

   public:
    static bool doubleResetDetected;
    static Preferences preferences;
    static void init();
    static WiFiConfiguration getWiFiConfiguration();
    static void saveWiFiConfiguration(WiFiConfiguration wiFiConfiguration);
    static void reset();
};