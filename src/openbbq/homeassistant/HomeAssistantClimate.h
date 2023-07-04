
#pragma once

#include "HomeAssistant.h"

class HomeAssistantProbe : public HomeAssistantEntity
{
public:
  HomeAssistantProbe(const String &id, const String &name)
      : HomeAssistantEntity(id, name),
        CurrentTemperature("current_temperature"),
        Temperature("temperature"),
        Mode("mode")

  {
    add(CurrentTemperature);
    add(Temperature);
    add(Mode);
  }

  const char *component() override { return "climate"; }

  void extendConfig(JsonDocument &doc) override
  {
    auto modes = doc.createNestedArray("modes");
    modes.add("off");
    modes.add("heat");

    doc["temperature_unit"] = "F";
    doc["precision"] = 0.1;
    doc["min_temp"] = 32;
    doc["max_temp"] = 32 + 2 * (210 - 32);
  }

  OutputValue CurrentTemperature;
  ControlValue Temperature;
  ControlValue Mode;
};

class HomeAssistantCook : public HomeAssistantProbe
{
public:
  HomeAssistantCook(const String &id, const String &name)
      : HomeAssistantProbe(id, name),
        Action("action"),
        PresetMode("preset_mode")
  {
    add(Action);
    add(PresetMode);
  }

  const char *component() override { return "climate"; }

  void extendConfig(JsonDocument &doc) override
  {
    auto preset_modes = doc.createNestedArray("preset_modes");
    preset_modes.add("Preheat");
    preset_modes.add("Normal");

    HomeAssistantProbe::extendConfig(doc);
  }

  // Valid values: off, heating, cooling, drying, idle, fan.
  OutputValue Action;
  ControlValue PresetMode;
};
