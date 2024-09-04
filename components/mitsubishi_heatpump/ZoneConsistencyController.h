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
    std::map<std::string, std::pair<std::chrono::time_point<std::chrono::steady_clock>, float>> remote_temperature_deltas_;
    TwoPointHeatPump* hp_ = nullptr;
};

#endif