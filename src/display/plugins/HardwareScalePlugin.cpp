#include "HardwareScalePlugin.h"
#include <display/core/Controller.h>

HardwareScalePlugin HardwareScales;

HardwareScalePlugin::HardwareScalePlugin() = default;

void HardwareScalePlugin::setup(Controller *controller, PluginManager *pluginManager) {
    this->controller = controller;
    this->pluginManager = pluginManager;

    pluginManager->on("controller:ready", [this](Event const &) {
        _isAvailable = this->controller->getSystemInfo().capabilities.hwScale;
        _scaleFactor1 = this->controller->getSettings().getScaleFactor1();
        _scaleFactor2 = this->controller->getSettings().getScaleFactor2();

        ESP_LOGI(LOG_TAG, "Hardware scale available: %s", _isAvailable ? "true" : "false");

        if (_scaleFactor1 != 0.0f && _scaleFactor2 != 0.0f) {
            this->controller->getClientController()->sendScaleCalibration(_scaleFactor1, _scaleFactor2);
            delay(50);
        }
        this->controller->getClientController()->tare();

        this->controller->setHardwareScaleAvailable(_isAvailable);
    });

    pluginManager->on("controller:brew:start", [this](Event const &) {
       onProcessStart();
    });

    pluginManager->on("controller:scale:measurement", [this](Event const &event) {
        float value = event.getFloat("value");
        ESP_LOGI(LOG_TAG, "Scale measurement: %.2f", value);
        this->onMeasurement(value);
    });

    pluginManager->on("controller:scale:calibrate-update", [this](Event const &event) {
        float scaleFactor1 = this->controller->getSettings().getScaleFactor1();
        float scaleFactor2 = this->controller->getSettings().getScaleFactor2();
        if (event.getFloat("scaleFactor1") != 0.0f) {
            scaleFactor1= event.getFloat("scaleFactor1");
        }
        if (event.getFloat("scaleFactor2") != 0.0f) {
            scaleFactor2= event.getFloat("scaleFactor2");
        }
        _scaleFactor1 = scaleFactor1;
        _scaleFactor2 = scaleFactor2;

        this->controller->getSettings().setScaleFactors(_scaleFactor1, _scaleFactor2);
    });
}

void HardwareScalePlugin::tare() {
    if (_isAvailable) {
        ESP_LOGI(LOG_TAG, "Taring hardware scale");
        controller->getClientController()->tare();
    }
}

void HardwareScalePlugin::calibrate(uint8_t cell, float calibrationWeight) {
    if (_isAvailable) {
        ESP_LOGI(LOG_TAG, "Calibrating hardware scale: cell %d, weight %.2f", cell, calibrationWeight);
        controller->getClientController()->sendCalibrateScale(cell, calibrationWeight);
    }else {
        pluginManager->trigger("controller:scale:calibrate-failed", "reason", "Hardware scale not available");
    }
}

void HardwareScalePlugin::onProcessStart() {
    if (_isAvailable) {
        ESP_LOGI(LOG_TAG, "Starting tare process for hardware scale");
        controller->getClientController()->tare();
        delay(200);

        ESP_LOGI(LOG_TAG, "Scale process start completed successfully");
    }
}

void HardwareScalePlugin::onMeasurement(float value) {
    this->_lastMeasurement = value;
    controller->onVolumetricMeasurement(value, VolumetricMeasurementSource::MEASUREMENT);
}