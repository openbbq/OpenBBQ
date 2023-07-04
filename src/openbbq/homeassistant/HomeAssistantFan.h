
#pragma once

#include "HomeAssistant.h"

class HomeAssistantFan : public HomeAssistantEntity
{
public:
  HomeAssistantFan(const String &id, const String &name)
      : HomeAssistantEntity(id, name),
        Percentage("percentage"),
        PresetMode("preset_mode")

  {
    add(State);
    add(Percentage);
    add(PresetMode);
  }

  const char *component() override { return "fan"; }

  void extendConfig(JsonDocument &doc) override
  {
    auto preset_modes = doc.createNestedArray("preset_modes");
    preset_modes.add("Automatic");
    preset_modes.add("Manual");

    doc["speed_range_min"] = 1;
    doc["speed_range_max"] = 100;
  }

  ControlValue State;
  ControlValue Percentage;
  ControlValue PresetMode;
};
