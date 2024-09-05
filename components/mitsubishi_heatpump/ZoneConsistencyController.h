/**
 * ZoneConsistencyController.h
 * 
 * Author: Paul Murphy @donutsoft on GitHub
 * 
 * Last Updated: September 4th 2024
 * License: BSD
 * 
 */

#ifndef ZONECONSISTENCYCONTROLLER_H
#define ZONECONSISTENCYCONTROLLER_H

#include <chrono>
#include <map>
#include <string>
#include "TwoPointHeatPump.h"

class RemoteTemperatureData {
public:
    RemoteTemperatureData(
        float temperature_low,
        float temperature_high,
        float temperature_current) :
            creation_time_(std::chrono::steady_clock::now()),
            temperature_low_(temperature_low),
            temperature_high_(temperature_high),
            temperature_current_(temperature_current) {};

    const std::chrono::time_point<std::chrono::steady_clock> creation_time_;
    const float temperature_low_;
    const float temperature_high_;
    const float temperature_current_;
};

class ZoneConsistencyController {
public:
    void zoneUpdate(const std::string& device_name,
                    const std::string& state,
                    float temperature_low,
                    float temperature_high,
                    float temperature_current);

    void assignDominantSetting();

    void setHeatpumpController(TwoPointHeatPump* hp);

    void update();

private:
    std::map<std::string, RemoteTemperatureData*> remote_temperature_data_;
    TwoPointHeatPump* hp_ = nullptr;

    int calculateDelta(RemoteTemperatureData* remoteTemperatureData);
};

#endif