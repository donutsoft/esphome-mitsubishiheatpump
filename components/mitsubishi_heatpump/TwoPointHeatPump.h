/**
 * TwoPointHeatPump.h
 * 
 * Author: Paul Murphy @donutsoft on GitHub
 * 
 * Last Updated: September 4th 2024
 * License: BSD
 *
 */

#ifndef TWOPOINTHEATPUMP_H
#define TWOPOINTHEATPUMP_H

#include "HeatPump.h"

struct twoPointHeatPumpSettings : heatpumpSettings {
    float temperature_low;
    float temperature_high;
};

enum HeatpumpMode {
    UNKNOWN,
    OFF,
    COOL,
    HEAT
};

class TwoPointHeatPump : public HeatPump {
public:
    TwoPointHeatPump(float temperature_low, float temperature_high, bool managed_mode) : 
        HeatPump(),
        managed_mode_(managed_mode),
        temperature_low_(nearestHalf(temperature_low)),
        temperature_high_(nearestHalf(temperature_high)) {};

    twoPointHeatPumpSettings getSettings();
    void setTemperatureLow(float setting);
    void setTemperatureHigh(float setting);
    void setModeSetting(const char* setting);
    void setPowerSetting(const char* setting);

    void setDesiredModeOverride(HeatpumpMode heatPumpMode);

    void update();

    void updateIfChangesPending();

    void sync();

private:
    // Retrieves the low/high temperature from heatpump, depending on whether
    // it was currently configured to HEAT or COOL. Returns true if a new
    // value was retrieved, or false otherwise.
    boolean readTemperatureSetpointsFromHeatPump();

    // Ensures the heatpump is configured correctly to HEAT/COOL if
    // managed mode is enabled. Returns true if it was already configured
    // correctly, or false if it will be configured.
    boolean ensureDesiredModeConfigured();

    float nearestHalf(float input);
    
    // Returns the correct mode (HEAT/COOL) if managed mode is enabled. If
    // managed mode is disabled, it will simply return GetCurrentMode().
    HeatpumpMode GetDesiredMode();

    // Returns the currently configured mode on the heat pump.
    HeatpumpMode GetCurrentMode();

    boolean changes_pending_ = false;
    HeatpumpMode desired_mode_override_ = HeatpumpMode::UNKNOWN;
    boolean managed_mode_ = false;
    float temperature_low_;
    float temperature_high_;
};

#endif