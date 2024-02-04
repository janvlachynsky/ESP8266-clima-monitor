// #pragma once
// #include <list>
// #include <memory>
// #include "U8g2lib.h"

// #include "sensor.h"

// class IDisplayDrawer
// {
// public:
//     virtual ~IDisplayDrawer(){};
//     virtual void draw(uint32_t x, uint32_t y) = 0;
// };

// template <typename Display = U8G2>
// class TemperatureDisplay : public IDisplayDrawer
// {
// public:
//     TemperatureDisplay(Display &display, std::list < std::unique_ptr<ISensor<TempSensorReadings>>> sensors)
//         : _sensors(sensors), _display(display)
//     {
//     }
//     void draw(uint32_t x, uint32_t y) override
//     {
//         _display.clearBuffer();

//         for (auto &sensor : _sensors)
//         {
//             sensor->read();
//             TempSensorReadings data = sensor->toDisplay();
//             std::map<String, String> dataMap = sensor->toMap();
//             // Really bad, refactor this:
//             if (data.name == "IN") {
//                 _display.setFont(u8g2_font_ncenB08_tr);
//                 _display.drawStr(0, 0, dataMap["name"]);
//                 _display.setFont(u8g2_font_ncenB14_tr);
//                 _display.drawStr(10, 0, dataMap["temp"]);
//                 _display.drawStr(30, 0, dataMap["humi"]);

//             }
//         }

//         _display.clearBuffer();
//         // TemperatureDisplay insideTempDisp = TemperatureDisplay(oledDisplay, dht_in);
//         // TemperatureDisplay outsideTempDisp = TemperatureDisplay(oledDisplay, dht_out);
//         // insideTempDisp.draw(0,20);
//         // outsideTempDisp.draw(0,40);
//         // // _display.setFont(u8g2_font_ncenB14_tr);

//         // _display.drawStr(0, 20, dht_in.toString().c_str());
//         // // _display.setFont(u8g2_font_ncenB14_tr);
//         // _display.drawStr(0, 40, dht_out.toString().c_str());
//         // _display.sendBuffer();

//         // _display.setFont(u8g2_font_ncenB10_tr);
//         // _display.drawStr(x, y, _source.toString().c_str());
//     }
//     std::list < std::unique_ptr<ISensor<TempSensorReadings>>> _sensors;
//     Display &_display;
// };
