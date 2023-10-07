#ifndef CLIMATE_SENSOR_H
#define CLIMATE_SENSOR_H
#include <BlueDot_BME280.h>
#include <Wire.h>

struct ClimateData {
    double temparature;
    double humidity;
    double pressure;
};

class ClimateSensor {
   private:
    static BlueDot_BME280 sensor;
    static unsigned long int lastUpdate;

   public:
    static ClimateData data;
    static void init();
};
#endif