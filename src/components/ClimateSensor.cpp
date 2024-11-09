#include "ClimateSensor.h"

ClimateData ClimateSensor::data;
SensirionI2cSht4x ClimateSensor::sensor;

void readClimateSensor(void *sensor) {
    while (true) {
        vTaskDelay(10000);
        ((SensirionI2cSht4x *)sensor)->measureHighPrecision(ClimateSensor::data.temperature, ClimateSensor::data.humidity);
        ClimateSensor::data.pressure = 0;
    }
}

void ClimateSensor::init() {
    Wire.begin(1,2);
    sensor.begin(Wire, SHT41_I2C_ADDR_44);
    sensor.softReset();
    xTaskCreate(readClimateSensor, "Read Climate Sensor", 2048, &sensor, 1, NULL);
}
