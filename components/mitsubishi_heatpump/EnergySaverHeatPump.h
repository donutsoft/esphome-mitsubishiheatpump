/**
 * EnergySaverHeatPump.h
 * 
 * Author: Paul Murphy @donutsoft on GitHub
 * 
 * Last Updated: October 4th 2024
 * License: BSD
 *
 */

#include <string>

#ifndef ENERGYSAVERHEATPUMP_H
#define ENERGYSAVERHEATPUMP_H

#include "HeatPump.h"

class EnergySaverHeatPump : public HeatPump {
public:
    EnergySaverHeatPump() : 
        HeatPump() {};

    void setTemperature(float setting);
    void sync();

    float getTemperatureDelta();

    void setEnergySavingMode(bool);
    bool getEnergySavingMode();

    heatpumpSettings getSettings();
protected:
    boolean changes_pending_ = false;
private:
    float desired_temperature_ = 0.0;
    float last_delta_ = 0.0;
    bool energy_saver_enabled_ = false;
    
};

#endif