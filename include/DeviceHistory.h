#include "Co2Sensor.h"
#include "ClimateSensor.h"
#include "SecRCDevice.h"
#include <deque>

struct DeviceHistoryData {
    int co2ppm;
    double temperature;
    double humidity;
    double pressure;
    int fanSpeed;
    int ventilationMode;
}; 

class DeviceHistory {
   public:
    static void init();
    static std::deque<DeviceHistoryData> data;
};