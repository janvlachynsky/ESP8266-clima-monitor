#pragma once

#include <map>

#include "DHT.h"

// Requires DataStruct for represent all data of sensor
template <typename DataStruct>
class ISensor
{
public:
    virtual ~ISensor() {}
    virtual void read() = 0;
    virtual String toString() = 0;
    virtual DataStruct toDisplay() = 0;
    virtual std::map<String, String> toMap() = 0;
};

struct TempSensorReadings {
    String name;
    float temp, humi;

};

// Temperature and humidity ISensor class implementation
template<typename SensorFamily = DHT>
class TempSensor : public ISensor<TempSensorReadings>
{
public:
    TempSensorReadings readings;

    SensorFamily *_sensor;
    TempSensor(const String name, uint8_t pin, uint8_t type)
    : readings({name, 0, 0})
    , _pin(pin)
    , _type(type)
    {
        _sensor = new SensorFamily(_pin, _type);
        pinMode(_pin, INPUT);
        _sensor->begin();
    }

    ~TempSensor(){ delete _sensor;}

    void read() override
    {
        readings.temp = _sensor->readTemperature();
        readings.humi = _sensor->readHumidity();
    }

    String toString() override {
        if (isnan(readings.temp) || isnan(readings.humi))
        {
            Serial.println("Failed to read from DHT sensor!");
            return readings.name + ": N/A";
        }
        String str = readings.name + ": ";
        str += String(readings.temp, 1) + "°C, ";
        str += String(readings.humi, 1) + "%";
        return str;
    }

    // Returns sensor data to draw on display
    TempSensorReadings toDisplay() override {
        return readings;
    }
    std::map<String, String> toMap(){
        std::map<String, String> map;

        map["name"] = readings.name + ": ";
        map["temp"] = String(readings.temp, 1);
        map["humi"] = String(readings.humi, 1) + "%";
        return map;
    }

private:
    uint8_t _pin; // Analog pin to read sensor
    uint8_t _type; // DHT type
};
