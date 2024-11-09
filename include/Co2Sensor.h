#ifndef CO2_SENSOR_H
#define CO2_SENSOR_H
#include <SensirionI2CScd4x.h>

struct Co2Data {
    uint16_t ppm;
    float temperature;
    float humidity;
};

class Co2Sensor {
   private:
    static SensirionI2CScd4x sensor;

   public:
    static void init();
    static Co2Data data;
};
#endif