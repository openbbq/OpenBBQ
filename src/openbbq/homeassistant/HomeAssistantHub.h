#pragma once

class HomeAssistantUptime : public HomeAssistantEntity {
    public:
    HomeAssistantUptime(const String &id, const String &name) 
  : HomeAssistantEntity(id, name)
  {
    add(Uptime);
  }

  const char *component() override { return "sensor"; }

  void extendConfig(JsonDocument &doc) override
  {
    doc["device_class"] = "duration";
    doc["state_class"] = "total_increasing";
    doc["unit_of_measurement"] = "s";
  }

  OutputValue Uptime;
};
