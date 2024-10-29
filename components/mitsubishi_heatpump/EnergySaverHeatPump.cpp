/**
 * EnergySaverHeatPump.cpp
 * 
 * Author: Paul Murphy @donutsoft on GitHub
 * 
 * Last Updated: October 24th 2024
 * License: BSD
 *
 */

#include "EnergySaverHeatPump.h"
#include "esphome.h"
#include <cmath>

using esphome::esp_log_printf_;

heatpumpSettings EnergySaverHeatPump::getSettings() {
    heatpumpSettings settings = HeatPump::getSettings();
    if (settings.power != NULL && desired_temperature_ > 0.0 && energy_saver_enabled_) {
        // Heatpump not fully initialized yet.
        settings.temperature = desired_temperature_;
    }
    return settings;
}

void EnergySaverHeatPump::sync() {
    HeatPump::sync();

    if (desired_temperature_ > 0.0 && energy_saver_enabled_) {
        float temperature_delta = getTemperatureDelta();
        HeatPump::setTemperature(temperature_delta);

        if (temperature_delta != last_delta_) {
            ESP_LOGD("EnergySaverHeatPump", "New target temperature %2.f (%.2f)", temperature_delta, (temperature_delta * 1.8) + 32); 
            last_delta_ = temperature_delta;
            changes_pending_ = true;
        }
    }
}

float EnergySaverHeatPump::getTemperatureDelta() {
    float room_temperature = getRoomTemperature();
    if (abs(room_temperature - desired_temperature_) > 1.5) {
        if (desired_temperature_ < room_temperature) {
            return room_temperature - 1.5;
        } else {
            return room_temperature + 1.5;

        }
    } else {
        return desired_temperature_;
    }
}

void EnergySaverHeatPump::setTemperature(float setting) {
    desired_temperature_ = setting;
    if (energy_saver_enabled_) {
        HeatPump::setTemperature(getTemperatureDelta());
    } else {
        HeatPump::setTemperature(setting);
    }
}

void EnergySaverHeatPump::setEnergySavingMode(bool enabled) {
    ESP_LOGD("EnergySaverHeatPump", "Set energy saving mode %d", enabled);
    if (energy_saver_enabled_ != enabled && desired_temperature_ > 0.0) {
        energy_saver_enabled_ = enabled;
        setTemperature(desired_temperature_);
    } else {
        energy_saver_enabled_ = enabled;
    }
}

bool EnergySaverHeatPump::getEnergySavingMode() {
    return energy_saver_enabled_;
}