#include "Co2Sensor.h"

Co2Data Co2Sensor::data;
SensirionI2CScd4x Co2Sensor::sensor;

void readCo2Sensor(void *sensor) {
    while (true) {
        vTaskDelay(10000);
        ((SensirionI2CScd4x *)sensor)->readMeasurement(Co2Sensor::data.ppm, Co2Sensor::data.temperature, Co2Sensor::data.humidity);
    }
}

void Co2Sensor::init() {
    Wire.begin(1,2);
    sensor.begin(Wire);
    sensor.stopPeriodicMeasurement();
    sensor.startPeriodicMeasurement();
    xTaskCreate(readCo2Sensor, "Read CO2 Sensor", 2048, &sensor, 1, NULL);
}