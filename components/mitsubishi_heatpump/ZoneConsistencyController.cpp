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
    float delta = 0;
    if (current_temperature < temperature_low) {
        delta = current_temperature - temperature_low;
    } else if (current_temperature > temperature_high) {
        delta = current_temperature - temperature_high;
    } else {
        delta = 0;
    }

    ESP_LOGD(
            "ZoneConsistencyController", "Found zone update for %s. state=%s delta=%.2f",
            device_name.c_str(),
            state.c_str(),
            delta);


    if (state == "heat_cool") {
        remote_temperature_deltas_[device_name] = 
            std::pair<std::chrono::time_point<std::chrono::steady_clock>, float>(
                std::chrono::steady_clock::now(),
                delta);
    } else {
        remote_temperature_deltas_.erase(device_name);
    }

    assignDominantSetting();
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


    for (auto const& kvp : remote_temperature_deltas_) {
        auto value = kvp.second;
        if (abs(value.second) > abs(maxDelta)) {
            maxDelta = value.second;
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
        ESP_LOGD("ZoneConsistencyController", "Max Delta=%f, assigning to local control.", maxDelta);
        mode = HeatpumpMode::UNKNOWN;
    }

    this->hp_->setDesiredModeOverride(mode);
}

void ZoneConsistencyController::setHeatpumpController(TwoPointHeatPump* hp) {
    hp_ = hp;
}