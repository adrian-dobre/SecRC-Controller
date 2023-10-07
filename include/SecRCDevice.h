#ifndef SEC_RC_DEVICE_H
#define SEC_RC_DEVICE_H
#include "SecRC.h"

class SecRCDevice {
   private:
    static SecRC *secRC;

   public:
    static void init();
    static SecRCStatus status;
    static void changeVentilationMode(SecRCVentilationMode ventilationMode);
    static void changeFanSpeed(SecRCFanSpeed fanSpeed);
    static void resetChangeFilterReminder();
    static void toggleOnOff();
};
#endif