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
    TwoPointHeatPump(float temperature_low, float temperature_high) : 
        HeatPump(),
        temperature_low_(nearestHalf(temperature_low)),
        temperature_high_(nearestHalf(temperature_high)) {
        setRoomTempChangedCallback([this](float current_temperature) {
                this->room_temperature_update(current_temperature);
            });
    };

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
    boolean syncTemperatureSetpointsFromHeatPump();
    float nearestHalf(float input);
    void room_temperature_update(float current_temperature);
    HeatpumpMode GetDesiredMode();
    HeatpumpMode GetCurrentMode();

    float temperature_low_;
    float temperature_high_;

    boolean managed_mode_ = false;
    HeatpumpMode desired_mode_override_ = HeatpumpMode::UNKNOWN;

    boolean changes_pending_ = false;
};

#endif