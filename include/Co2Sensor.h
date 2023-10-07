#ifndef CO2_SENSOR_H
#define CO2_SENSOR_H
#include "MHZ19.h"

struct Co2Data {
    int ppm;
};

class Co2Sensor {
   private:
    static MHZ19 sensor;

   public:
    static void init();
    static Co2Data data;
};
#endif