#pragma once

#include "HomeAssistant.h"

class HomeAssistantNumber : public HomeAssistantEntity
{
public:
  HomeAssistantNumber(const String &id, const String &name, float min, float max)
      : HomeAssistantEntity(id, name), _minimum(min), _maximum(max)
  {
    add(Number);
  }

  const char *component() override { return "number"; }

  void extendConfig(JsonDocument &doc) override
  {
    doc["step"] = .01;
    doc["min"] = _minimum;
    doc["max"] = _maximum;
  }

  ControlValue Number;

private:
  float _minimum;
  float _maximum;
};
