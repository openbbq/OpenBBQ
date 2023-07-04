
#include <openbbq/config/Config.h>

#include <ArduinoJson.h>
#include <LittleFS.h>
#include <openbbq/DebugSerial.h>

Config config;

Config::Config()
{
}

bool Config::changed()
{
    if (!_changed)
    {
        for (auto const &section : _sections)
        {
            if (section->changed())
            {
                _changed = true;
                break;
            }
        }
    }

    return _changed;
}

void Config::changed(bool value)
{
    _changed = value;
    if (!value)
    {
        for (auto const &section : _sections)
        {
            section->changed(false);
        }
    }
}

void Config::commit()
{
    if (changed())
    {
        DebugSerial.println("Config changed, saving...");
        save();
        changed(false);
    }
}

bool Config::load()
{
    File file = LittleFS.open("/config.json", "r");

    // Allocate a temporary JsonDocument
    // Don't forget to change the capacity to match your requirements.
    // Use arduinojson.org/v6/assistant to compute the capacity.
    StaticJsonDocument<1024> doc;

    // Deserialize the JSON document

    auto error = deserializeJson(doc, file);
    file.close();

    if (error)
    {
        DebugSerial.printf("Failed to read file: %s\n", error.c_str());
        _changed = true;
    }

    // Copy values from the JsonDocument to the Config
    BROKER_ADDR = doc["BROKER_ADDR"];
    BROKER_PORT = doc["BROKER_PORT"];
    BROKER_USERNAME = doc["BROKER_USERNAME"];
    BROKER_PASSWORD = doc["BROKER_PASSWORD"];

    for (auto const &section : _sections)
    {
        section->unmarshal(doc.as<JsonObject>());
    }

    if (error)
    {
        DebugSerial.printf("Failed to read file: %s\n", error.c_str());
        changed(true);
        return true;
    }
    else
    {
        changed(false);
        return false;
    }
}

void Config::save()
{
    // Open file for writing
    File file = LittleFS.open("/config.json", "w");
    if (!file)
    {
        DebugSerial.println(F("Failed to create file"));
        return;
    }

    // Allocate a temporary JsonDocument
    // Don't forget to change the capacity to match your requirements.
    // Use arduinojson.org/assistant to compute the capacity.
    StaticJsonDocument<1024> doc;

    // Set the values in the document
    doc["BROKER_ADDR"] = BROKER_ADDR;
    doc["BROKER_PORT"] = BROKER_PORT;
    doc["BROKER_USERNAME"] = BROKER_USERNAME;
    doc["BROKER_PASSWORD"] = BROKER_PASSWORD;

    // PID.write(doc.createNestedObject("PID"));
    for (auto const &section : _sections)
    {
        section->marshal(doc.as<JsonObject>());
    }

    // Serialize JSON to file
    if (serializeJsonPretty(doc, file) == 0)
    {
        DebugSerial.println(F("Failed to write to file"));
    }

    // Close the file
    file.close();
}

void Config::print()
{
    // Open file for reading
    File file = LittleFS.open("/config.json", "r");
    if (!file)
    {
        DebugSerial.println(F("Failed to print file"));
        return;
    }

    // Extract each characters by one by one
    while (file.available())
    {
        DebugSerial.print((char)file.read());
    }
    DebugSerial.println();

    // Close the file
    file.close();
}

void ConfigObject::unmarshal(JsonVariant json)
{
    for (auto prop : _props)
    {
        prop.second->unmarshal(json, prop.first);
    }

    Serial.println("Unmarshalling...");
    serializeJsonPretty(json, Serial);
    Serial.println();
}

void ConfigObject::marshal(JsonVariant json)
{
    for (auto prop : _props)
    {
        prop.second->marshal(json, prop.first);
    }

    Serial.println("Marshalling...");
    serializeJsonPretty(json, Serial);
    Serial.println();
}

bool ConfigObject::changed()
{
    for (auto prop : _props)
    {
        if (prop.second->changed())
        {
            return true;
        }
    }
    return false;
}

void ConfigObject::changed(bool value)
{
    for (auto prop : _props)
    {
        prop.second->changed(value);
    }
}

/*

Setpoint

170.00 °F

alpha 10.00
signal 170.00

Proportional
Kp 1.00

Integral
Min -2.00
Max 50.00
Ki 0.10

Derivative
alpha 3.00
Kd 3.00


*/