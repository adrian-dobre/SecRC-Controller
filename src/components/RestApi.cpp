#include "RestApi.h"

#include "ClimateSensor.h"
#include "Co2Sensor.h"
#include "DeviceConfiguration.h"
#include "DeviceHistory.h"
#include "SecRCDevice.h"
#include "WebServer.h"

double round2(double value) { return (int)(value * 100 + 0.5) / 100.0; }

void RestApi::init(bool apMode) {
    ClimateSensor::init();
    Co2Sensor::init();
    SecRCDevice::init();
    DeviceHistory::init();
    WebServer::init(80);
    WiFiConfiguration wiFiConfiguration =
        DeviceConfiguration::getWiFiConfiguration();
    if (apMode) {
        Serial.println(
            "Server started in AP mode. Adding configuration portal.");
        WebServer::on(
            "/", HTTP_GET, [wiFiConfiguration](AsyncWebServerRequest *request) {
                request->send(SPIFFS, "/files/index.html", "text/html", false,
                              [wiFiConfiguration](const String &var) {
                                  if (var == "ssid") {
                                      return wiFiConfiguration.ssid;
                                  }
                                  if (var == "password") {
                                      return wiFiConfiguration.password;
                                  }
                                  if (var == "access-key") {
                                      return wiFiConfiguration.accessKey;
                                  }
                                  return String();
                              });
            });

        WebServer::on("/sec-rc/config", HTTP_PUT,
                      [](AsyncWebServerRequest *request, JsonVariant &json) {
                          if (!json.isNull()) {
                              String ssid = json["ssid"] | "";
                              String password = json["password"] | "";
                              String accessKey = json["access-key"] | "";
                              if (!ssid.isEmpty() && !password.isEmpty() &&
                                  !accessKey.isEmpty()) {
                                  WiFiConfiguration configuration;
                                  configuration.ssid = ssid;
                                  configuration.password = password;
                                  configuration.accessKey = accessKey;
                                  DeviceConfiguration::saveWiFiConfiguration(
                                      configuration);
                                  WebServer::OK(request);
                                  return ESP.restart();
                              }
                          }
                          WebServer::badRequest(request);
                      });

        WebServer::on("/sec-rc/reset", HTTP_POST,
                      [](AsyncWebServerRequest *request) {
                          DeviceConfiguration::reset();
                          WebServer::OK(request);
                          ESP.restart();
                      });

        WebServer::on("/sec-rc/restart", HTTP_POST,
                      [](AsyncWebServerRequest *request) {
                          WebServer::OK(request);
                          ESP.restart();
                      });
    } else {
        Serial.println("Server started in Station mode. Setting up RestAPI");
        WebServer::setAccessKey(wiFiConfiguration.accessKey);
        WebServer::onAuthorized(
            "/sec-rc/stats", HTTP_GET, [](AsyncWebServerRequest *request) {
                ClimateData climateData = ClimateSensor::data;
                StaticJsonDocument<256> doc;

                doc["climate"]["temperature"] = round2(climateData.temparature);
                doc["climate"]["pressure"] = round2(climateData.pressure);
                doc["climate"]["humidity"] = round2(climateData.humidity);
                doc["climate"]["co2"] = round2(Co2Sensor::data.ppm);
                doc["ventilation"]["mode"] = SecRCDevice::status.mode;
                doc["ventilation"]["fanSpeed"] = SecRCDevice::status.fanSpeed;
                doc["ventilation"]["filterChangeRequired"] =
                    SecRCDevice::status.filterChangeRequired;

                String buffer;
                serializeJson(doc, buffer);
                request->send(200, "application/json", buffer);
            });

        WebServer::onAuthorized(
            "/sec-rc/history", HTTP_GET, [](AsyncWebServerRequest *request) {
                std::deque<DeviceHistoryData> history = DeviceHistory::data;
                DynamicJsonDocument doc = DynamicJsonDocument(27456);

                JsonArray temperature =
                    doc["climate"].createNestedArray("temperature");
                JsonArray pressure =
                    doc["climate"].createNestedArray("pressure");
                JsonArray humidity =
                    doc["climate"].createNestedArray("humidity");
                JsonArray co2 = doc["climate"].createNestedArray("co2");
                JsonArray ventilationMode =
                    doc["ventilation"].createNestedArray("mode");
                JsonArray fanSpeed =
                    doc["ventilation"].createNestedArray("fanSpeed");

                for (DeviceHistoryData item : history) {
                    temperature.add(round2(item.temperature));
                    pressure.add(round2(item.pressure));
                    humidity.add(round2(item.humidity));
                    co2.add(round2(item.co2ppm));
                    ventilationMode.add(item.ventilationMode);
                    fanSpeed.add(item.fanSpeed);
                }

                String buffer;
                serializeJson(doc, buffer);
                request->send(200, "application/json", buffer);
            });

        WebServer::onAuthorized(
            "/sec-rc/ventilation/mode", HTTP_PUT,
            [](AsyncWebServerRequest *request, JsonVariant &json) {
                if (!json.isNull()) {
                    int mode = json["mode"];
                    if (mode != 0) {
                        SecRCDevice::changeVentilationMode(
                            (SecRCVentilationMode)mode);
                        return WebServer::OK(request);
                    }
                }
                WebServer::badRequest(request);
            });

        WebServer::onAuthorized(
            "/sec-rc/ventilation/fan-speed", HTTP_PUT,
            [](AsyncWebServerRequest *request, JsonVariant &json) {
                if (!json.isNull()) {
                    int fanSpeed = json["fanSpeed"];
                    if (fanSpeed != 0) {
                        SecRCDevice::changeFanSpeed((SecRCFanSpeed)fanSpeed);
                        return WebServer::OK(request);
                    }
                }
                WebServer::badRequest(request);
            });

        WebServer::onAuthorized("/sec-rc/ventilation/filter-reset", HTTP_POST,
                                [](AsyncWebServerRequest *request) {
                                    SecRCDevice::resetChangeFilterReminder();
                                    WebServer::OK(request);
                                });

        WebServer::onAuthorized("/sec-rc/ventilation/on-off/toggle", HTTP_POST,
                                [](AsyncWebServerRequest *request) {
                                    SecRCDevice::toggleOnOff();
                                    WebServer::OK(request);
                                });
    }
}
