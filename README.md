# NodeMCU ESP8266 - Daikin ARC/BRC Controller
The project is about automating a Daikin AC. Using a NodeMCU ESP8266, you can control your AC with any MQTT compatible Home automation hub. I have use Home assistant.

## Wiring Diagram

To make the Wiring Diagram, I have use a awsome website [circuito.io](https://www.circuito.io/app).

![Wiring Diagram](Wiring%20Diagram.png)

## Parts

* NodeMCU ESP8266 - Qty: 1
* LED - Basic Green 5mm - Qty: 1
* 100 Ohm Resistor - Qty: 1
* Infrared (IR) LED 950nm - Qty: 1
* Transistor - NPN BC337 - Qty: 1
* 220 Ohm Resistor - Qty: 1
* Mini Pushbutton Switch - Qty: 1
* 10K Ohm Resistor - Qty: 1

## Home assistant configuration
```
climate:
  - platform: mqtt
    name: Living Room AC
    modes:
      - "off"
      - "fan_only"
      - "cool"
      - "dry"
      - "heat"
    swing_modes:
      - "On"
      - "Off"
    fan_modes:
      - "Min"
      - "Low"
      - "Medium"
      - "High"
      - "Auto"
    mode_command_topic: "DaikinACController/Mode"
    temperature_command_topic: "DaikinACController/Temperature"
    fan_mode_command_topic: "DaikinACController/FanSpeed"
    swing_mode_command_topic: "DaikinACController/Swing"
    min_temp: 18
    max_temp: 30
    retain: false
    precision: 1.0
```
## Compatibility
### Compatible with AC:
* FTK09NMVJU
* FTX09NMVJU
* FTK12NMVJU
* FTX12NMVJU (Tested with)
* FTK18NMVJU
* FTX18NMVJU
* FTK24NMVJU
* FTX24NMVJU
* AND A LOT MORE

### Compatible with AC remote:

#### If the remote used for your Daikin AC remote Start With ARC or BRC, it should be compatible.

* ARC480A1  * ARC480A11 * ARC480A21 * ARC480A31
* ARC480A2  * ARC480A12 * ARC480A22 * ARC480A32
* ARC480A3  * ARC480A13 * ARC480A23 * ARC480A33
* ARC480A4  * ARC480A14 * ARC480A24 * ARC480A34
* ARC480A5  * ARC480A15 * ARC480A25 * ARC480A35
* ARC480A6  * ARC480A16 * ARC480A26 * ARC480A36
* ARC480A7  * ARC480A17 * ARC480A27 * ARC480A37
* ARC480A8  * ARC480A18 * ARC480A28
* ARC480A9  * ARC480A19 * ARC480A29
* ARC480A10 * ARC480A20 * ARC480A30
* BRC073a4 * BRC073a5 * BRC073a7 * BRC1E62
* BRC4C155 * BRC4C158 * BRC4C152 * BRC1C62
* BRC7F634F * BRC7F635F
* AND A LOT MORE

## Big thanks to

* [tzapu](https://github.com/tzapu) For is Amazing librarie [WiFiManager](https://github.com/tzapu/WiFiManager)
* [knolleary](https://github.com/knolleary) For is Amazing librarie [PubSubClient](https://github.com/knolleary/pubsubclient)
* [danny-source](https://github.com/danny-source) For is Amazing librarie [Arduino_DY_IRDaikin ARC/BRC](https://github.com/danny-source/Arduino_DY_IRDaikin)
* [bblanchon](https://github.com/bblanchon) For is Amazing librarie [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
