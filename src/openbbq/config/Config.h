#pragma once

#include <ArduinoJson.h>
#include <WString.h>

#include <memory>
#include <map>
#include <vector>

class IConfig
{
public:
    virtual ~IConfig() = default;
    virtual void unmarshal(JsonVariant json) = 0;
    virtual void unmarshal(JsonVariant json, const String &key) { unmarshal(json[key]); }
    virtual void marshal(JsonVariant json) = 0;
    virtual void marshal(JsonVariant json, const String &key) { marshal(json[key]); }
    virtual bool changed() = 0;
    virtual void changed(bool value) = 0;
};

class ConfigBase : public IConfig
{
public:
    bool changed() override { return _changed; }
    void changed(bool value) override { _changed = value; }

private:
    bool _changed = false;
};

class ConfigObject : public IConfig
{
public:
    void add(const String &name, IConfig *config) { _props[name] = config; }
    void unmarshal(JsonVariant json) override;
    void marshal(JsonVariant json) override;
    bool changed() override;
    void changed(bool value) override;

private:
    std::map<String, IConfig *> _props;
};

class ConfigSection : public IConfig
{
public:
    ConfigSection(const String &section, IConfig *config) : _section(section), _config(config) {}
    void unmarshal(JsonVariant json) override { _config->unmarshal(json[_section]); }
    void marshal(JsonVariant json) override { _config->marshal(json.createNestedObject(_section)); }
    bool changed() override { return _config->changed(); }
    void changed(bool value) override { _config->changed(value); }

protected:
    String _section;
    IConfig *_config;
};

class Config
{
public:
    Config();

    void add(const String &section, IConfig *config)
    {
        _sections.emplace_back(new ConfigSection(section, config));
    }

    bool load();
    void save();

    bool changed();
    void changed(bool value);

    void commit();

    void print();

    String BROKER_ADDR;
    int BROKER_PORT;
    String BROKER_USERNAME;
    String BROKER_PASSWORD;

private:
    bool _changed = false;
    std::vector<std::unique_ptr<IConfig>> _sections;
};

extern Config config;
