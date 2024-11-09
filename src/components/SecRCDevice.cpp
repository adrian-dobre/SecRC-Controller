#include "SecRCDevice.h"

#include <queue>
#include <utility>

SecRC *SecRCDevice::secRC;
SecRCStatus SecRCDevice::status;
using namespace std;

int secRCStatusReads = 0;

void updateSecRCStatus(SecRC *secRC) {
    SecRCDevice::status =
        secRC->getStatus(secRCStatusReads == 0 || secRCStatusReads % 180 == 0);
    secRCStatusReads++;
}

void updateSecRCStatus(void *secRC) {
    while (true) {
        vTaskDelay(10000);
        updateSecRCStatus((SecRC *)secRC);
    }
}
enum CommandType {
    Mode = 1,
    FanSpeed = 2,
    ResetFilterReminder = 3,
    ToggleOnOff = 4
};
queue<pair<CommandType, int>> commandsQueue = queue<pair<CommandType, int>>();

void processSecRCCommands(void *secRCPanel) {
    while (true) {
        if (!commandsQueue.empty()) {
            pair<CommandType, int> commandToSend = commandsQueue.front();
            commandsQueue.pop();
            SecRC *secRC = (SecRC *)secRCPanel;
            switch (commandToSend.first) {
                case CommandType::Mode:
                    secRC->changeVentilationMode(
                        (SecRCVentilationMode)commandToSend.second);
                    updateSecRCStatus(secRC);
                    break;
                case CommandType::FanSpeed:
                    secRC->changeFanSpeed((SecRCFanSpeed)commandToSend.second);
                    updateSecRCStatus(secRC);
                    break;
                case CommandType::ResetFilterReminder:
                    secRC->resetFilterChangeStatus();
                    updateSecRCStatus(secRC);
                    break;
                case CommandType::ToggleOnOff:
                    secRC->toggleOnOff();
                    break;
                default:
                    break;
            }
        }
        vTaskDelay(500);
    }
}

void SecRCDevice::init() {
    SecRCIOConfig config = {.enablePin = 48,
                            .fanSpeed1LedPin = 4,
                            .fanSpeed2LedPin = 5,
                            .fanSpeed3LedPin = 6,
                            .fanSpeed4LedPin = 7,
                            .hrvModeLedPin = 13,
                            .bypassModeLedPin = 14,
                            .filterResetLedPin = 21,
                            .powerButtonPin = 10,
                            .fanSpeedButtonPin = 11,
                            .hrvModeButtonPin = 8,
                            .bypassModeButtonPin = 9,
                            .filterResetButtonPin = 12};
    secRC = new SecRC(config);
    xTaskCreate(updateSecRCStatus, "Update SecRC status", 2048, secRC, 1, NULL);
    xTaskCreate(processSecRCCommands, "Process SecRC commands", 2048, secRC, 1,
                NULL);

}

void SecRCDevice::changeFanSpeed(SecRCFanSpeed fanSpeed) {
    commandsQueue.push(pair<CommandType, int>(CommandType::FanSpeed, fanSpeed));
}

void SecRCDevice::changeVentilationMode(SecRCVentilationMode mode) {
    commandsQueue.push(pair<CommandType, int>(CommandType::Mode, mode));
}

void SecRCDevice::resetChangeFilterReminder() {
    commandsQueue.push(
        pair<CommandType, int>(CommandType::ResetFilterReminder, 0));
}

void SecRCDevice::toggleOnOff() {
    commandsQueue.push(pair<CommandType, int>(CommandType::ToggleOnOff, 0));
}