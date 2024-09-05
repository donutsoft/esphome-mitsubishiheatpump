/**
 * ZoneConsistencyController.cpp
 * 
 * Author: Paul Murphy @donutsoft on GitHub
 * 
 * Last Updated: September 4th 2024
 * License: BSD
 *
 */

#include "ZoneConsistencyController.h"
#include "esphome.h"

using esphome::esp_log_printf_;

void ZoneConsistencyController::update() {
    // This will be called every "update_interval" milliseconds.
    assignDominantSetting();
}

void ZoneConsistencyController::zoneUpdate(
    const std::string& device_name,
    const std::string& state,
    float temperature_low,
    float temperature_high,
    float current_temperature) {
    if (remote_temperature_data_.count(device_name) > 0) {
        delete remote_temperature_data_[device_name];
    }

    if (state == "heat_cool") {
        remote_temperature_data_[device_name] = new RemoteTemperatureData(
            temperature_low,
            temperature_high,
            current_temperature);
    } else {
        remote_temperature_data_.erase(device_name);
    }

    assignDominantSetting();
}

int ZoneConsistencyController::calculateDelta(RemoteTemperatureData* remoteTemperatureData) {
    float delta = 0;
    if (remoteTemperatureData->temperature_current_ < remoteTemperatureData->temperature_low_) {
        delta = remoteTemperatureData->temperature_current_ - remoteTemperatureData->temperature_low_;
    } else if (remoteTemperatureData->temperature_current_ > remoteTemperatureData->temperature_high_) {
        delta = remoteTemperatureData->temperature_current_ - remoteTemperatureData->temperature_high_;
    }

    return delta;
}

void ZoneConsistencyController::assignDominantSetting() {
    if (hp_ == nullptr) {
        ESP_LOGD("ZoneConsistencyController", "HP not set yet, wont update.");
        return;
    }

    twoPointHeatPumpSettings currentSettings = hp_->getSettings();
    if (strcmp(currentSettings.mode, "DUAL_POINT") != 0) {
        ESP_LOGD("ZoneConsistencyController", "HP not set to dual point, wont update");
        return;
    }


    float maxDelta = 0;
    HeatpumpMode mode = HeatpumpMode::UNKNOWN;


    for (auto const& kvp : remote_temperature_data_) {
        if (abs(calculateDelta(kvp.second)) > abs(maxDelta)) {
            maxDelta = calculateDelta(kvp.second);
        }
    }

    if (maxDelta < -0.1) {
        ESP_LOGD("ZoneConsistencyController", "Max Delta=%f, assigning heat.", maxDelta);

        if (hp_->getRoomTemperature() <= currentSettings.temperature_low) {
            mode = HeatpumpMode::HEAT;
        } else {
            mode = HeatpumpMode::OFF;
        }
    } else if (maxDelta > 0.1) {
        ESP_LOGD("ZoneConsistencyController", "Max Delta=%f, assigning cool.", maxDelta);

        if (hp_->getRoomTemperature() >= currentSettings.temperature_high) {
            mode = HeatpumpMode::COOL;
        } else {
            mode = HeatpumpMode::OFF;
        }
    } else {
        // Stick to whatever mode we had previously, it's likely to be used again
        // once a heatpump falls further away from its setpoint.
        mode = previous_mode_;
    }

    previous_mode_ = mode;
    this->hp_->setDesiredModeOverride(mode);
}

void ZoneConsistencyController::setHeatpumpController(TwoPointHeatPump* hp) {
    hp_ = hp;
}