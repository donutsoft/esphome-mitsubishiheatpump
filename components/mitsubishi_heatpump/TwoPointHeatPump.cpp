/**
 * TwoPointHeatPump.cpp
 * 
 * Author: Paul Murphy @donutsoft on GitHub
 * 
 * Last Updated: September 4th 2024
 * License: BSD
 *
 */


#include "TwoPointHeatPump.h"
#include "esphome.h"

using esphome::esp_log_printf_;

twoPointHeatPumpSettings TwoPointHeatPump::getSettings() {
    heatpumpSettings settings = HeatPump::getSettings();

    twoPointHeatPumpSettings result;
    result.power = managed_mode ? "ON" : settings.power;
    result.mode = managed_mode ? "DUAL_POINT" : settings.mode;
    result.temperature = settings.temperature;
    result.fan = settings.fan;
    result.vane = settings.vane;
    result.wideVane = settings.wideVane;
    result.iSee = settings.iSee;
    result.connected = settings.connected;

    result.temperature_low = temperature_low_;
    result.temperature_high = temperature_high_;

    return result;
}

boolean TwoPointHeatPump::syncTemperatureSetpointsFromHeatPump() { 
    boolean updated = false;
    heatpumpSettings settings = HeatPump::getSettings();
    if (settings.power == NULL) {
        // Heatpump not fully initialized yet.
        return updated;
    }

    if (strcmp(settings.power, "ON") == 0) {
        if (strcmp(settings.mode, "HEAT") == 0 && 
            settings.temperature != temperature_high_ &&
            settings.temperature != temperature_low_) {
            ESP_LOGD("TwoPointHeatPump", "Currently set to HEAT, extracting low temperature from unit");

            temperature_low_ = settings.temperature;
            updated = true;
        }
        else if (strcmp(settings.mode, "COOL") == 0 && 
            settings.temperature != temperature_low_ &&
            settings.temperature != temperature_high_) {
            ESP_LOGD("TwoPointHeatPump", "Currently set to COOL, extracting high temperature from unit");
            temperature_high_ = settings.temperature;
            updated = true;
        }
    }

    return updated;
}

HeatpumpMode TwoPointHeatPump::GetDesiredMode() {
    if (!managed_mode) {
        return GetCurrentMode();
    }

    if (desired_mode_override_ != HeatpumpMode::UNKNOWN) {
        return desired_mode_override_;
    }

    int roomTemperature = getRoomTemperature();
    if (roomTemperature <= temperature_low_) {
        return HeatpumpMode::HEAT;
    } else if (roomTemperature >= temperature_high_) {
        return HeatpumpMode::COOL;
    } else {
        int distanceFromHeatPoint = abs(temperature_low_ - roomTemperature);
        int distanceFromCoolPoint = abs(temperature_high_ - roomTemperature);

        if (distanceFromHeatPoint < distanceFromCoolPoint) {
            return HeatpumpMode::HEAT;
        } else {
            return HeatpumpMode::COOL;
        }
    }
}

void TwoPointHeatPump::setDesiredModeOverride(HeatpumpMode heatPumpMode) {
    if (desired_mode_override_ == heatPumpMode) {
        return;
    }

    if (managed_mode) {
        HeatpumpMode previousMode = GetDesiredMode();
        desired_mode_override_ = heatPumpMode;

        if (previousMode != GetDesiredMode()) {
            setModeSetting("DUAL_POINT");
            update();
        }
    }

}

void TwoPointHeatPump::update() {
    ESP_LOGD("TwoPointHeatPump", "Update called");
    changes_pending_ = true;
}

void TwoPointHeatPump::updateIfChangesPending() {
    if (changes_pending_) {
        changes_pending_ = false;
        HeatPump::update();
    }
}

void TwoPointHeatPump::sync() {
    if (!changes_pending_) {
        HeatPump::sync();

        if (syncTemperatureSetpointsFromHeatPump()) {
            // Reset desired mode as the temperature update likely happened from remote.
            desired_mode_override_ = HeatpumpMode::UNKNOWN;
        }
    }
}

HeatpumpMode TwoPointHeatPump::GetCurrentMode() {
    heatpumpSettings settings = HeatPump::getSettings();    
    if (strcmp(settings.power, "OFF") == 0) {
        return HeatpumpMode::OFF;
    } else if (strcmp(settings.mode, "HEAT") == 0) {
        return HeatpumpMode::HEAT;
    } else if (strcmp(settings.mode, "COOL") == 0) {
        return HeatpumpMode::COOL;
    } else {
        return HeatpumpMode::UNKNOWN;
    }

}

void TwoPointHeatPump::setPowerSetting(const char* setting) {
    if (strcmp(setting, "OFF") == 0) {
        managed_mode = false;
    }

    HeatPump::setPowerSetting(setting);
}

void TwoPointHeatPump::setModeSetting(const char* setting) {
    ESP_LOGD("TwoPointHeatPump", "SetModeSetting: %s", setting);
    // TODO: Add override for two point here.
    if (strcmp(setting, "DUAL_POINT") == 0) {
        managed_mode = true;

        HeatpumpMode desiredMode = GetDesiredMode();
        float temperature = 0;
        std::string powerSetting = "ON";
        if (desiredMode == HeatpumpMode::HEAT) {
            ESP_LOGD("TwoPointHeatPump", "Modifying setting to HEAT mode");
            temperature = temperature_low_;
            setting = "HEAT";
            powerSetting = "ON";
        } else if (desiredMode == HeatpumpMode::COOL) {
            ESP_LOGD("TwoPointHeatPump", "Modifying setting to COOL mode");
            temperature = temperature_high_;
            setting = "COOL";
            powerSetting = "ON";
        } else if (desiredMode == HeatpumpMode::OFF) {
            powerSetting = "OFF";
        } else {
            ESP_LOGD("TwoPointHeatPump", "Dont know what to modify setting to, returning");
            return;
        }
        
        if (temperature > 0) {
            setTemperature(temperature);
        }
        HeatPump::setPowerSetting(powerSetting.c_str());

        if (strcmp(setting, "DUAL_POINT") != 0) {
            HeatPump::setModeSetting(setting);
        }
    } else {
        managed_mode = false;

        HeatPump::setModeSetting(setting);
    }
}

String heatpumpModeToString(HeatpumpMode mode) {
    switch(mode) {
        case HeatpumpMode::HEAT:
            return "HEAT";
        case HeatpumpMode::COOL:
            return "COOL";
        case HeatpumpMode::UNKNOWN:
            return "UNKNOWN";
        case HeatpumpMode::OFF:
            return "OFF";        
    }
    return "NO_MATCH";
}

void TwoPointHeatPump::setTemperatureLow(float setting) {
    ESP_LOGD("TwoPointHeatPump", "setTemperatureLow: %.2f", setting);

    temperature_low_ = setting;
    if (GetCurrentMode() == HeatpumpMode::HEAT) {
        ESP_LOGD("TwoPointHeatPump", "setTempLow: GetCurrentMode is current mode %s, forwarding to heatpump: %.2f (room temp %.2f)", heatpumpModeToString(GetCurrentMode()), setting, getRoomTemperature());
        setTemperature(temperature_low_);
    }
}

void TwoPointHeatPump::setTemperatureHigh(float setting) {
    ESP_LOGD("TwoPointHeatPump", "setTemperatureHigh: %.2f", setting);
    temperature_high_ = setting;
    if (GetCurrentMode() == HeatpumpMode::COOL) {
        ESP_LOGD("TwoPointHeatPump", "setTempHigh: GetCurrentMode is current mode %s, forwarding to heatpump: %.2f (room temp %.2f)", heatpumpModeToString(GetCurrentMode()), setting, getRoomTemperature());
        setTemperature(temperature_high_);
    }
}

void TwoPointHeatPump::room_temperature_update(float current_temperature) {
    if (!managed_mode) {
        return;
    }

    HeatpumpMode currentMode = GetCurrentMode();
    HeatpumpMode desiredMode = GetDesiredMode();
    if (currentMode != desiredMode) {
        ESP_LOGD("TwoPointHeatPump", "room_temperature_update():: Current mode is not desired mode, attempting update from %s to %s", 
            heatpumpModeToString(currentMode), heatpumpModeToString(desiredMode));
        setModeSetting("DUAL_POINT");
        update();
    }
}