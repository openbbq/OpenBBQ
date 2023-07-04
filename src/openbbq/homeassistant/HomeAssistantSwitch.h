#pragma once

#include "HomeAssistant.h"

class HomeAssistantSwitch : public HomeAssistantEntity
{
public:
  HomeAssistantSwitch(const String &id, const String &name)
      : HomeAssistantEntity(id, name)
  {
    add(Power);
  }

  const char *component() override { return "switch"; }

  OutputValue Power;
};
