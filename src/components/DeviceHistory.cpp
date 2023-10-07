#include "DeviceHistory.h"

std::deque<DeviceHistoryData> DeviceHistory::data =
    std::deque<DeviceHistoryData>();

int taskDelay1Min = 60000;
int taskDelay10Min = 10 * taskDelay1Min;
bool ranOnce = false;

void recordDeviceHistory(void *sensor) {
    while (true) {
        int taskDelay = taskDelay10Min;
        if (!ranOnce) {
            ranOnce = true;
            taskDelay = taskDelay1Min;
        }
        vTaskDelay(taskDelay);
        DeviceHistoryData dataPoint = {
            .co2ppm = Co2Sensor::data.ppm,
            .temperature = ClimateSensor::data.temparature,
            .humidity = ClimateSensor::data.humidity,
            .pressure = ClimateSensor::data.pressure,
            .fanSpeed = SecRCDevice::status.fanSpeed,
            .ventilationMode = SecRCDevice::status.mode};
        if (DeviceHistory::data.size() >= 144) {
            DeviceHistory::data.pop_front();
        }
        DeviceHistory::data.push_back(dataPoint);
    }
}

void DeviceHistory::init() {
    xTaskCreate(recordDeviceHistory, "Record Device History", 2048,
                (void *)NULL, 1, NULL);
}
