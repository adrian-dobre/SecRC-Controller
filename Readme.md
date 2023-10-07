# SEC-RC Controller
## What's this about?

This is an ESP32 based Weather Station that can measure:
- CO2 levels (ppm)
- atmospheric pressure (bar)
- relative humidity
- temperature

combined with an interface ([based on my SecRC library](https://github.com/adrian-dobre/SecRC)) to control [Seventilation's Sevi160-RC](https://ventilatie-recuperare.ro/produs/recuperator-caldura-sevi-160-rc-wireless-480/) Heat-Recovery Ventilation system via the [Sec-RC Control Panel](https://www.dezentrale-lueftung.com/en/shop/decentralised-ventilation-central-controller-sec-rc/).

![Sevi-160-RC](./resources/demo/sevi-160-rc.jpg?raw=true)
![Sevi-160-RC](./resources/demo/sevi-160-rc.jpg?raw=true)

It uses the following sensors:
- MHZ-19C CO2 sensor
- BME 280 temperature, humidity and pressure sensor

![PCB](./resources/demo/sec-controller-pcb.jpg?raw=true)
![Sec-Controller-Front](./resources/demo/sec-controller-front.jpg?raw=true)
![Sec-Controller-Back](./resources/demo/sec-controller-back.jpg?raw=true)

## Why do it?

Well, due to a few of reasons:
1. As of right now, there is no "smart" (connected) solution offered by the manufacturer for the RC variant of the Sevi-160 Heat-Recovery Ventilation System
2. It makes sense to combine the "controller" with a Weather Station - this can allow creating automations without having to purchase separate sensors
3. I like controlling my devices from my phone.

## What does it do?

It exposes a configuration page, when booting in WiFi AP mode (similar to ESP WiFi managers), where you configure WiFi SSID, Password and an Access Key used to authenticate RestAPI calls.
![Configuration](./resources/demo/controller-config.png?raw=true)

Once in WiFi Station mode, it exposes a Rest API that allows controlling the HRV and seeing HRV status and climate data. You can check out the [available APIs here](./src/components/RestApi.cpp).
```
curl --location --request GET 'http://<host>:<port>/sec-rc/stats' \
--header 'Authorization: Bearer <AccessKey>'

Response:
{
    "climate": {
        "temperature": 29.01,
        "pressure": 1009.95,
        "humidity": 34.26,
        "co2": 912
    },
    "ventilation": {
        "mode": 2,
        "fanSpeed": 4,
        "filterChangeRequired": false
    }
}
```

You can control the device via Rest API calls or use [the mobile application](https://github.com/adrian-dobre/SecRC-Controller-Mobile)

![Mobile-App-Interface](./resources/demo/mobile-app-interface.jpg?raw=true)

## What parts do I need?

1. [ESP32 DevKit V1](https://www.banggood.com/Geekcreit-ESP32-WiFi+bluetooth-Development-Board-Ultra-Low-Power-Consumption-Dual-Cores-Pins-Unsoldered-p-1214159.html?cur_warehouse=CN) (or others, just make sure the pins are correctly mapped)
2. [MHZ-19(C) CO2 sensor](https://www.banggood.com/MH-Z19-MH-Z19C-IR-Infrared-CO2-Sensor-Module-Carbon-Dioxide-Gas-Sensor-NDIR-for-CO2-Monitor-400-5000ppm-UART-PWM-Output-MH-Z19C-p-1788261.html?cur_warehouse=CN)
3. [BME280 sensor](https://www.banggood.com/CJMCU-280E-BME280-High-Precision-Atmospheric-Pressure-Sensor-Module-Board-p-1897807.html?rmmds=myorder&cur_warehouse=CN)

You can wire everything according to this schematic:

![Schematic](./resources/schematic/Schematic_SecRCController.svg?raw=true)

I also built a custom PCB, you can check out [Gerber files here](./resources/gerber/Gerber_SecRCController.zip).