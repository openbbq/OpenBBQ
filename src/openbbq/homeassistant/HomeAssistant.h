
#pragma once

#include <functional>
#include <vector>
#include <initializer_list>

#include <StreamUtils/Prints/BufferingPrint.hpp>

#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WString.h>
#include <StreamString.h>

#include <openbbq/control/ControlSignal.h>

class HomeAssistantClient
{
public:
  HomeAssistantClient(PubSubClient &client)
  {
    _client = &client;
  }

  void init(const String &id, const String &host, int port, const String &user, const String &pass)
  {
    using namespace std::placeholders;

    _id = id;
    _user = user;
    _pass = pass;
    _client->setServer(host.c_str(), port);
    _client->setCallback(std::bind(&HomeAssistantClient::handleReceived, this, _1, _2, _3));
  }

  void onConnected(std::function<void()> callback)
  {
    _cbConnected = callback;
  }
  void onReceived(std::function<void(char *, uint8_t *, unsigned int)> callback)
  {
    _cbReceived = callback;
  }

  bool loop()
  {
    if (!_client->connected())
    {
      DebugSerial.printf("HomeAssistantClient.loop() calling connect(%s, %s, ...)\n", _id.c_str(), _user.c_str());
      _client->connect(_id.c_str(), _user.c_str(), _pass.c_str());
      if (!_client->connected())
      {
        DebugSerial.printf("connect failed\n");
        return false;
      }
      DebugSerial.printf("Firing connected event\n");
      _cbConnected();
    }

    return _client->loop();
  }

  bool publish(const String &topic, const String &payload)
  {
    // DebugSerial.printf("publish %s len: %d\n", topic.c_str(), payload.length());
    if (_client->beginPublish(topic.c_str(), payload.length(), true))
    {
      _client->print(payload.c_str());
      _client->endPublish();
      return true;
    }
    return false;
  }

  bool publish(const String &topic, JsonDocument &payload)
  {
    // DebugSerial.printf("publish %s len: %d\n", topic.c_str(), measureJson(payload));

    // serializeJsonPretty(payload, Serial);
    // DebugSerial.printf("\n\n");

    if (_client->beginPublish(topic.c_str(), measureJson(payload), true))
    {
      StreamUtils::BufferingPrint bufferedWifiClient(*_client, 128);
      serializeJson(payload, bufferedWifiClient);
      bufferedWifiClient.flush();

      _client->endPublish();
      return true;
    }
    return false;
  }

  bool subscribe(const String &topic)
  {
    DebugSerial.printf("subscribe %s\n", topic.c_str());
    return _client->subscribe(topic.c_str());
  }

private:
  void handleReceived(char *topic, uint8_t *payload, unsigned int length)
  {
    DebugSerial.printf("received %s len: %d\n", topic, length);

    _cbReceived(topic, payload, length);
  }

  String _id;
  String _user;
  String _pass;
  PubSubClient *_client;

  std::function<void()> _cbConnected;
  std::function<void(char *, uint8_t *, unsigned int)> _cbReceived;
};

class Value
{
public:
  virtual void extendConfig(JsonDocument &doc) {}
  virtual void publish(HomeAssistantClient *client, const String &baseTopic) {}
  virtual void handleConnected(HomeAssistantClient *client, const String &baseTopic) {}
  virtual void handleReceived(HomeAssistantClient *client, const String &baseTopic, char *topic, uint8_t *payload, unsigned int length) {}
};

class OutputValue : public Value
{
public:
  OutputValue()
  {
  }

  OutputValue(const String &name)
  {
    _name = name;
  }

  void extendConfig(JsonDocument &doc) override
  {
    if (_name.isEmpty())
    {
      doc["state_topic"] = "~";
    }
    else
    {
      doc[_name + "_topic"] = "~/" + _name;
    }
  }

  void publish(HomeAssistantClient *client, const String &baseTopic) override
  {
    if (_signalNumber != nullptr)
    {
      _value = String(_signalNumber->value(), 2);
    }
    if (_signalText != nullptr)
    {
      _value = _signalText->value();
    }
    if (_published != _value && !_value.isEmpty())
    {
      if (_name.isEmpty())
      {
        client->publish(baseTopic, _value);
      }
      else
      {
        client->publish(baseTopic + "/" + _name, _value);
      }
      _published = _value;
    }
  }

  void connect(IControlSignal<float> &signal) { connect(&signal); }
  void connect(IControlSignal<float> *signal) { _signalNumber = signal; }
  void connect(IControlSignal<String> &signal) { connect(&signal); }
  void connect(IControlSignal<String> *signal) { _signalText = signal; }

  void accept(const String &payload)
  {
    if (_signalNumber != nullptr)
    {
      _signalNumber->value(payload.toFloat());
    }
    else if (_signalText != nullptr)
    {
      _signalText->value(payload);
    }
    else
    {
      set(payload);
    }
  }

  void set(const String &value)
  {
    _value = value;
  }

  void set(int value)
  {
    _value = String(value);
  }

  void set(double value, unsigned int decimalPlaces)
  {
    _value = String(value, decimalPlaces);
  }

protected:
  String _name;
  String _value;
  IControlSignal<float> *_signalNumber = nullptr;
  IControlSignal<String> *_signalText = nullptr;
  String _published;
};

class ControlValue : public OutputValue
{
public:
  ControlValue() : OutputValue()
  {
    _cbReceived = [this](const String &value)
    {
      this->accept(value);
    };
  }

  ControlValue(const String &name) : OutputValue(name)
  {
    _cbReceived = [this](const String &value)
    {
      this->accept(value);
    };
  }

  void onReceived(std::function<void(const String &)> callback)
  {
    _cbReceived = callback;
  }

  void extendConfig(JsonDocument &doc) override
  {
    if (_name.isEmpty())
    {
      doc["state_topic"] = "~";
      doc["command_topic"] = "~/set";
    }
    else
    {
      doc[_name + "_state_topic"] = "~/" + _name;
      doc[_name + "_command_topic"] = "~/" + _name + "/set";
    }
  }

  void handleConnected(HomeAssistantClient *client, const String &baseTopic) override
  {
    if (_name.isEmpty())
    {
      client->subscribe(baseTopic + "/set");
    }
    else
    {
      client->subscribe(baseTopic + "/" + _name + "/set");
    }
  }

  void handleReceived(HomeAssistantClient *client, const String &baseTopic, char *topic, uint8_t *payload, unsigned int length) override
  {
    String expected;
    if (_name.isEmpty())
    {
      expected = baseTopic + "/set";
    }
    else
    {
      expected = baseTopic + "/" + _name + "/set";
    }
    if (expected == topic)
    {
      String data;
      const char *p = (const char *)payload;
      while (length--)
      {
        data += *p++;
      }
      _cbReceived(data);
      // set(payload, length);
    }
  }

private:
  std::function<void(const String &)> _cbReceived;
};

class HomeAssistantEntity
{
public:
  HomeAssistantEntity(const String &id, const String &name)
  {
    _id = id;
    _name = name;
  }

  operator HomeAssistantEntity *() { return this; }

  void init(const String &node_id, const String &device_id)
  {
    // https://www.home-assistant.io/integrations/mqtt/#discovery-topic
    // <discovery_prefix>/<component>/[<node_id>/]<object_id>/config
    // Best practice for entities with a unique_id is to set <object_id> to unique_id and omit the <node_id>.

    _topic = String("homeassistant/") + component() + "/" + node_id + "/" + device_id + "_" + _id;
  }

  virtual void handleConnected(HomeAssistantClient *client)
  {
    for (auto value : _values)
    {
      value->handleConnected(client, _topic);
    }
  }

  virtual void handleReceived(HomeAssistantClient *client, char *topic, uint8_t *payload, unsigned int length)
  {
    for (auto value : _values)
    {
      value->handleReceived(client, _topic, topic, payload, length);
    }
  }

  virtual void publish(HomeAssistantClient *client)
  {
    for (auto value : _values)
    {
      value->publish(client, _topic);
    }
  }

  virtual void publishConfig(HomeAssistantClient *client, const String &node_id, const String &device_id, const String &device_name)
  {
    DynamicJsonDocument doc(1024);
    doc["name"] = device_name + " " + _name;
    doc["object_id"] = device_id + "_" + _id;
    doc["unique_id"] = node_id + "_" + device_id + "_" + _id;
    doc["device"]["identifiers"] = node_id + "_" + device_id;
    doc["device"]["name"] = device_name;
    doc["device"]["sw_version"] = "0.1.0";
    doc["~"] = _topic;

    for (auto value : _values)
    {
      value->extendConfig(doc);
    }

    extendConfig(doc);

    client->publish(_topic + "/config", doc);
  }

  virtual const char *component() = 0;
  virtual void extendConfig(JsonDocument &doc) {}

protected:
  void add(Value &value)
  {
    _values.push_back(&value);
  }

  String _id;
  String _name;
  String _topic;
  std::vector<Value *> _values;
};

class HomeAssistantDevice
{
public:
  HomeAssistantDevice(const String &id, const String &name, std::initializer_list<HomeAssistantEntity *> entities)
  {
    _id = id;
    _name = name;
    for (auto entity : entities)
    {
      _entities.push_back(entity);
    }
  }

  operator HomeAssistantDevice *() { return this; }

  void init(const String &node_id)
  {
    for (auto entity : _entities)
    {
      entity->init(node_id, _id);
    }
  }

  void handleConnected(HomeAssistantClient *client)
  {
    for (auto entity : _entities)
    {
      entity->handleConnected(client);
    }
  }
  void handleReceived(HomeAssistantClient *client, char *topic, uint8_t *payload, unsigned int length)
  {
    for (auto entity : _entities)
    {
      entity->handleReceived(client, topic, payload, length);
    }
  }

  void publishConfig(HomeAssistantClient *client, const String &node_id)
  {
    for (auto entity : _entities)
    {
      entity->publishConfig(client, node_id, _id, _name);
    }
  }

  void publish(HomeAssistantClient *client)
  {
    for (auto entity : _entities)
    {
      entity->publish(client);
    }
  }

private:
  String _id;
  String _name;
  std::vector<HomeAssistantEntity *> _entities;
};

class HomeAssistant
{
public:
  HomeAssistant(HomeAssistantClient &client, std::initializer_list<HomeAssistantDevice *> devices)
  {
    using namespace std::placeholders;

    _client = &client;
    _client->onConnected(std::bind(&HomeAssistant::handleConnected, this));
    _client->onReceived(std::bind(&HomeAssistant::handleReceived, this, _1, _2, _3));
    for (auto device : devices)
    {
      _devices.push_back(device);
    }
  }

  void init(const String &id, const String &host, int port, const String &user, const String &pass)
  {
    _client->init(id, host, port, user, pass);

    _id = id;
    for (auto device : _devices)
    {
      device->init(_id);
    }
  }

  bool loop()
  {
    return _client->loop();
  }

  void publish()
  {
    for (auto device : _devices)
    {
      device->publish(_client);
    }
  }

private:
  void handleConnected()
  {
    for (auto device : _devices)
    {
      device->handleConnected(_client);
    }
    for (auto device : _devices)
    {
      device->publishConfig(_client, _id);
    }
  }

  void handleReceived(char *topic, uint8_t *payload, unsigned int length)
  {
    for (auto device : _devices)
    {
      device->handleReceived(_client, topic, payload, length);
    }
  }

  HomeAssistantClient *_client;
  String _id;
  String _topic;
  std::vector<HomeAssistantDevice *> _devices;
};
