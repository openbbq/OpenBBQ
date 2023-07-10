
#include <Arduino.h>
#include <OpenBBQ.h>

#include <LittleFS.h>

#ifdef ARDUINO_ARCH_ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

#include <ArduinoOTA.h>

#include <openbbq/DebugSerial.h>

#include <openbbq/config/Config.h>

#include <openbbq/sys/SysDebug.h>
#include <openbbq/sys/SysWiFi.h>
#include <openbbq/sys/SysOTA.h>
#include <openbbq/sys/SysDisplay.h>
#include <openbbq/sys/SysTemperature.h>
#include <openbbq/sys/SysFan.h>

#include <openbbq/control/ControlPeriod.h>
#include <openbbq/control/ControlPID.h>
#include <openbbq/control/ControlThermostat.h>

#include <openbbq/homeassistant/HomeAssistant.h>
#include <openbbq/homeassistant/HomeAssistantSwitch.h>
#include <openbbq/homeassistant/HomeAssistantClimate.h>
#include <openbbq/homeassistant/HomeAssistantHub.h>
#include <openbbq/homeassistant/HomeAssistantNumber.h>
#include <openbbq/homeassistant/HomeAssistantSensor.h>
#include <openbbq/homeassistant/HomeAssistantFan.h>

#include <Adafruit_ILI9341.h>
#include <Display_Adafruit_GFX.h>
#include <Display_XPT2046_Touchscreen.h>

#include <openbbq/ui/ThermostatPage.h>

#if defined(ARDUINO_ESP8266_NODEMCU_ESP12E)

const int8_t FAN_PIN = 4;
const int8_t TEMPERATURE_CS = 5;
const int8_t FOOD1_CS = 0;

#elif defined(ARDUINO_FEATHER_ESP32)

const int8_t FAN_PIN = 14;
const int8_t TEMPERATURE_CS = 32;
const int8_t FOOD1_CS = 15;
const int8_t DISPLAY_CS = 33;
const int8_t DISPLAY_RST = 27;
const int8_t DISPLAY_DC = 12;
const int8_t TOUCH_CS = 13;

#else

#endif

display::Clipped_GFX<Adafruit_ILI9341> tft(DISPLAY_CS, DISPLAY_DC, DISPLAY_RST);
display::Interface_Adafruit_GFX interface(tft);
XPT2046_Touchscreen ts(TOUCH_CS);

SysDebug debug;
SysWiFi wifi;
SysOTA ota;

SysDisplay gui(&interface);

SysTemperature temperature(TEMPERATURE_CS);
SysTemperature food1(FOOD1_CS);
SysFan fan(FAN_PIN);

ControlThermostat grillThermostat;
ControlThermostat food1Thermostat;
ControlPID pid;
ControlPeriod uptimePeriod;
ControlPeriod publishPeriod;

namespace _hub
{
  HomeAssistantSwitch led("led", "LED");
  HomeAssistantUptime uptime("uptime", "Uptime");
  HomeAssistantDevice device("bbqhub", "BBQ Hub", {led, uptime});
}

namespace _pid
{
  class FahrenheitSensor : public HomeAssistantSensor
  {
  public:
    FahrenheitSensor(const String &id, const String &name) : HomeAssistantSensor(id, name) {}
    void extendConfig(JsonDocument &doc) override
    {
      doc["suggested_display_precision"] = 1;
      doc["unit_of_measurement"] = "F";
    }
  };
  class PercentSensor : public HomeAssistantSensor
  {
  public:
    PercentSensor(const String &id, const String &name) : HomeAssistantSensor(id, name) {}
    void extendConfig(JsonDocument &doc) override
    {
      doc["suggested_display_precision"] = 2;
      doc["unit_of_measurement"] = "%";
    }
  };

  FahrenheitSensor sp("sp", "Setpoint");
  FahrenheitSensor pv("pv", "Process Value");
  PercentSensor out("out", "Output");

  HomeAssistantNumber spa("spa", "Setpoint Alpha", 0, 100);
  HomeAssistantNumber band1("band1", "Band 1", -100, 100);
  HomeAssistantNumber kp1("kp", "Coefficient P", -100, 100);
  HomeAssistantNumber ki1("ki", "Coefficient I", -100, 100);
  HomeAssistantNumber kd1("kd", "Coefficient D", -100, 100);
  HomeAssistantNumber band2("band2", "Band 2", -100, 100);
  HomeAssistantNumber kp2("kp2", "Coefficient P 2", -100, 100);
  HomeAssistantNumber ki2("ki2", "Coefficient I 2", -100, 100);
  HomeAssistantNumber kd2("kd2", "Coefficient D 2", -100, 100);
  HomeAssistantNumber alpha("alpha", "Derivative Alpha", 0, 100);
  HomeAssistantNumber imax("imax", "Integral Max", -100, 100);
  HomeAssistantNumber imin("imin", "Integral Min", -100, 100);

  PercentSensor p("p", "Output P");
  PercentSensor i("i", "Output I");
  PercentSensor d("d", "Output D");

  HomeAssistantDevice device("bbqpid", "BBQ PID",
                             {sp, pv, out, spa, band1, kp1, ki1, kd1, band2, kp2, ki2, kd2, alpha, imax, imin, p, i, d});
}

namespace _cyberq
{
  HomeAssistantCook cook("cook", "Cook");
  HomeAssistantProbe food1("food1", "Food 1");
  HomeAssistantProbe food2("food2", "Food 2");
  HomeAssistantProbe food3("food3", "Food 3");
  HomeAssistantFan fan("fan", "Fan");
  HomeAssistantDevice device("cyberq", "CyberQ", {cook, food1, food2, food3, fan});
}

namespace _ha
{
  WiFiClient wifi;
  PubSubClient mqtt(wifi);
  HomeAssistantClient client(mqtt);
  HomeAssistant ha(client, {_hub::device, _pid::device, _cyberq::device});
}

HomeAssistant &ha = _ha::ha;

void fatal(const char *message)
{
  DebugSerial.println(message);
  DebugSerial.println("Rebooting...");
  delay(2000);
  ESP.restart();
}

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  delay(1000);
  Serial.println("\nBooting");

  if (!LittleFS.begin())
  {
    Serial.println("LittleFS.begin() failed!");
  }

  config.add("WIFI", &wifi);
  config.add("PID", &pid);
  config.add("grill", &grillThermostat);
  config.add("food1", &food1Thermostat);
  config.print();
  if (!config.load())
  {
    Serial.println("Config load error");
  }

  SoftAPConfig softAP;
  softAP.local_ip = {172, 217, 28, 1};
  softAP.gateway = softAP.local_ip;
  softAP.subnet = {255, 255, 255, 0};
  softAP.ssid = "esp8266";
  softAP.passphrase = "987654321";

  wifi.add("/display.bmp", &interface);
  wifi.add("/wifi.json", &wifi);
  wifi.add("/pid.json", &pid);
  wifi.add("/grill.json", &grillThermostat);
  wifi.add("/food1.json", &food1Thermostat);
  if (!wifi.begin(softAP))
  {
    Serial.println("WiFi access point setup failed");
  }

  if (!debug.begin())
  {
    fatal("Could not initialize debug support.");
  }

  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(ILI9341_BLACK);

  ts.begin();

  interface.device<display::Display_XPT2046_Touchscreen>(ts)->calibration(
      TS_Point(4050, 200, 0),  // (0,0)
      TS_Point(4050, 3900, 0), // (240,0)
      TS_Point(580, 200, 0));  // (0,320)

  if (!gui.begin(bbq::ui::ThermostatList::ViewModel{
          {"Fan", fan},
          {"Pit", grillThermostat},
          {"#1", food1Thermostat},
          {"#2", food1Thermostat},
          {"#3", food1Thermostat},
      }))
  {
    fatal("Could not initialize display.");
  }

  if (!fan.begin(3000, 30))
  {
    fatal("Could not initialize fan.");
  }

  if (!temperature.begin(100))
  {
    fatal("Could not initialize grill thermocouple.");
  }

  if (!grillThermostat.begin())
  {
    fatal("Could not initialize grill thermostat.");
  }

  if (!food1.begin(100))
  {
    fatal("Could not initialize food1 thermocouple.");
  }

  if (!food1Thermostat.begin())
  {
    fatal("Could not initialize food1 thermostat.");
  }

  if (!pid.begin(100))
  {
    fatal("Could not initialize PID.");
  }

  if (!uptimePeriod.begin(2000, true))
  {
    fatal("Could not initialize uptime delay.");
  }

  if (!publishPeriod.begin(1250, false))
  {
    fatal("Could not initialize publish delay.");
  }

  ha.init(wifi.id(), config.BROKER_ADDR, config.BROKER_PORT, config.BROKER_USERNAME, config.BROKER_PASSWORD);

  // WebSerial is accessible at "<IP Address>/webserial" in browser
  //  WebSerial.begin(&server);
  //  server.begin();
  ota.begin();

  // connect thermocouples to thermostats
  grillThermostat.current.connect(temperature.output);
  food1Thermostat.current.connect(food1.output);

  gui.line1.connect(temperature.output);
  gui.line2.connect(food1.output);
  gui.line3.connect(wifi.state);
  gui.line4.connect(wifi.status);

  // connect pid to thermostat
  pid._setpoint.signal.connect(grillThermostat.temperature);
  pid._processValue.connect(grillThermostat.current);

  // connect fan to pid
  fan.signal.connect(pid._out);

  // IoT "climate" for grill
  _cyberq::cook.CurrentTemperature.connect(grillThermostat.current);
  _cyberq::cook.Temperature.connect(grillThermostat.temperature);
  _cyberq::cook.Mode.connect(grillThermostat.mode);
  _cyberq::cook.Action.connect(grillThermostat.action);

  // IoT "climate" for food1
  _cyberq::food1.CurrentTemperature.connect(food1Thermostat.current);
  _cyberq::food1.Temperature.connect(food1Thermostat.temperature);
  _cyberq::food1.Mode.connect(food1Thermostat.mode);

  // IoT "number" entities
  _pid::spa.Number.connect(pid._setpoint.alpha);
  _pid::band1.Number.connect(pid.band1);
  _pid::kp1.Number.connect(pid.Kp1);
  _pid::ki1.Number.connect(pid.Ki1);
  _pid::kd1.Number.connect(pid.Kd1);
  _pid::band2.Number.connect(pid.band2);
  _pid::kp2.Number.connect(pid.Kp2);
  _pid::ki2.Number.connect(pid.Ki2);
  _pid::kd2.Number.connect(pid.Kd2);
  _pid::alpha.Number.connect(pid._alpha);
  _pid::imax.Number.connect(pid._integralMax);
  _pid::imin.Number.connect(pid._integralMin);

  // IoT "sensor" entities
  _pid::sp.Value.connect(pid._setpoint.output);
  _pid::p.Value.connect(pid._proportional);
  _pid::i.Value.connect(pid._integral);
  _pid::d.Value.connect(pid._derivative);
  _pid::out.Value.connect(pid._out);

  DebugSerial.println("setup complete.");
}

void loop()
{
  // DebugSerial.println("loop beginning.");

  int32_t timeMs = millis();

  wifi.loop();

  // DebugSerial.println("Processing ArduinoOTA...");
  ota.loop();

  // DebugSerial.println("Processing temperature...");
  if (temperature.loop(timeMs))
  {
  }

  if (food1.loop(timeMs))
  {
  }

  if (grillThermostat.loop(timeMs))
  {
  }

  if (food1Thermostat.loop(timeMs))
  {
  }

  // DebugSerial.println("Processing PID...");
  if (pid.loop(timeMs))
  {
    // TODO(loudej) need a signal mux that changes the input based on mode so fan isn't always wired to pid out
    if (grillThermostat.mode.value() == "heat")
    {
      if (fan.signal > 0)
      {
        grillThermostat.action = "heating";
      }
      else
      {
        grillThermostat.action = "idle";
      }
    }
    else
    {
      grillThermostat.action = "off";
    }
  }

  // DebugSerial.println("Processing fan...");
  fan.loop(timeMs);

  // DebugSerial.println("Processing uptime...");
  if (uptimePeriod.loop(timeMs))
  {
    _hub::uptime.Uptime.set(timeMs / 1000);
  }

  // DebugSerial.println("Processing HomeAssistant...");
  // confirm or re-establish connection
  if (ha.loop())
  {
    // TODO have publish period be a per-topic cooldown instead?
    if (publishPeriod.loop(timeMs))
    {
      ha.publish();
    }
  }

  // handle touch and tft
  gui.loop();

  // support remote debugging
  debug.loop();

  // DebugSerial.println("Processing config...");
  config.commit();

  // DebugSerial.println("loop complete.");
}
