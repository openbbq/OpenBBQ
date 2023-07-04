#pragma once

#include "HomeAssistant.h"

class HomeAssistantSensor : public HomeAssistantEntity
{
public:
  HomeAssistantSensor(const String &id, const String &name)
      : HomeAssistantEntity(id, name)
  {
    add(Value);
  }

  const char *component() override { return "sensor"; }


  void extendConfig(JsonDocument &doc) override
  {
  }

  OutputValue Value;
};
