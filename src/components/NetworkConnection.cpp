#include "NetworkConnection.h"
void (*NetworkConnection::onConnectedCallback)(bool apMode);
void NetworkConnection::onConnected(void (*onConnectedCallback)(bool apMode)) {
    NetworkConnection::onConnectedCallback = onConnectedCallback;
}

void NetworkConnection::init() {
    DeviceConfiguration::init();
    WiFiConfiguration configuration =
        DeviceConfiguration::getWiFiConfiguration();

    bool hasCredentials = !configuration.ssid.isEmpty() &&
                          !configuration.password.isEmpty() &&
                          !configuration.accessKey.isEmpty();
    if (hasCredentials && !DeviceConfiguration::doubleResetDetected) {
        startStation(configuration.ssid, configuration.password);
    } else {
        startAP(hasCredentials);
    }
}

void accessPointTimeout(void* param) {
    vTaskDelay(120000);
    if (WiFi.getMode() == WIFI_AP) {
        Serial.println("AP mode timeout. Resetting...");
        ESP.restart();
    }
}

bool isAddressReachable(String address) {
    bool isReachable = false;
    HTTPClient http;
    http.setTimeout(5000);
    http.setConnectTimeout(5000);
    http.begin(address);
    int httpCode = http.GET();
    if (httpCode <= 0) {
        Serial.printf("%s is unreachable.", address.c_str());
    } else {
        isReachable = true;
        Serial.printf("GET %s replied with code %d.\n", address.c_str(), httpCode);
    }
    http.end();
    return isReachable;
}

bool isGatewayReachable() {
    String checkConnectivityAddress = "http://";
    checkConnectivityAddress.concat(WiFi.gatewayIP().toString());
    return isAddressReachable(checkConnectivityAddress);
}

bool isWebServerReachable() {
    String checkConnectivityAddress = "http://";
    checkConnectivityAddress.concat(WiFi.localIP().toString());
    return isAddressReachable(checkConnectivityAddress);
}
bool isFirstNetworkCheck = true;
void monitorNetworkConnection(void* param) {
    while (true) {
        if (isFirstNetworkCheck) {
            vTaskDelay(120000);
            isFirstNetworkCheck = false;
        } else {
            vTaskDelay(30000);
        }
        auto mode = WiFi.getMode();
        auto status = WiFi.status();
        if (mode == WIFI_MODE_STA && status == WL_CONNECTED) {
            if (!isGatewayReachable() || !isWebServerReachable()) {
                Serial.println("Restarting connection...");
                WiFi.disconnect();
            }
        } else {
            Serial.printf("Network mode: %d, status: %d. Skipping connection check.\n", mode, status);
        }
    }
}

void NetworkConnection::startAP(bool hasCredentials) {
    Serial.println("Starting in AP mode.");
    WiFi.mode(WIFI_AP);
    WiFi.softAP("SecRC", "secrccontroller");
    Serial.print("IP: ");
    Serial.println(WiFi.softAPIP());

    if (hasCredentials) {
        xTaskCreate(accessPointTimeout, "Access Point Timeout", 1024, NULL, 1,
                    NULL);
    }
    if (onConnectedCallback) {
        onConnectedCallback(true);
    }
}

void NetworkConnection::startStation(String ssid, String password) {
    Serial.print("Starting in station mode.");
    int stationTimeout = millis() + 60000;
    WiFi.disconnect();
    WiFi.setAutoReconnect(true);
    WiFi.setHostname("sec-rc");
    WiFi.mode(WIFI_STA);
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
    WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info){
        Serial.println("\nWiFi Connected!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
    }, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);

    WiFi.begin(ssid.c_str(), password.c_str());
    while (WiFi.status() != WL_CONNECTED && millis() < stationTimeout) {
        Serial.print(".");
        delay(500);
    }
    if (millis() > stationTimeout) {
        Serial.println("Station mode connect timeout.");
        startAP(true);
    } else {
        setupAutomaticReconnect();
        if (onConnectedCallback) {
            onConnectedCallback(false);
        }
    }
}

void NetworkConnection::setupAutomaticReconnect() {
    xTaskCreate(monitorNetworkConnection, "Monitor Network Connection", 4096, NULL, 1, NULL);
    WiFi.setAutoReconnect(false);
    auto connecting = std::make_shared<bool>(false);
    WiFi.onEvent([connecting](WiFiEvent_t event, WiFiEventInfo_t info){
        if  (!*connecting){
            Serial.print("WiFi Disconnected. Attempting reconnect.");
            *connecting = true;
        } else {
            Serial.print(".");
        }
        WiFi.disconnect();
        WiFi.begin();
    }, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    WiFi.onEvent([connecting](WiFiEvent_t event, WiFiEventInfo_t info){
        *connecting = false;
    }, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
}

