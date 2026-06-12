#ifndef HARDWARESCALEPLUGIN_H
#define HARDWARESCALEPLUGIN_H
#include "../core/Plugin.h"
#include <stdint.h>

void on_scale_measurement(float value);

class HardwareScalePlugin : public Plugin {
  public:
    HardwareScalePlugin();

    void setup(Controller *controller, PluginManager *pluginManager) override;
    void loop() override {};
    void tare();
    void calibrate(uint8_t cell, float calibrationWeight);

    bool isConnected() const {  return _isAvailable; }
  private:
    void onMeasurement(float value);
    void onProcessStart();

    const char *LOG_TAG = "HardwareScalePlugin";
    bool _isAvailable;
    float _scaleFactor1 = 1.0f, _scaleFactor2 = 1.0f;

    Controller *controller = nullptr;
    PluginManager *pluginManager = nullptr;
};

extern HardwareScalePlugin HardwareScales;

#endif // HARDWARESCALEPLUGIN_H
