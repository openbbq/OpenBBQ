#pragma once

#if defined(ARDUINO_ARCH_ESP32)
#include <WebServer.h>
typedef String RequestHandlerUriParameter;
#elif defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WebServer.h>
typedef const String &RequestHandlerUriParameter;
#endif

#include <IPAddress.h>
#include <WString.h>
#include <DNSServer.h>
#include <ArduinoJson.h>

#include <map>

#include <openbbq/config/Config.h>
#include <openbbq/control/ControlSignal.h>

#include <Display.h>
#include <display/Interface.h>

struct SoftAPConfig
{
    IPAddress local_ip;
    IPAddress gateway;
    IPAddress subnet;
    IPAddress dhcp_lease_start = (uint32_t)0;

    String ssid;
    String passphrase;
};

class SysWiFi : public ConfigBase
{
public:
    void marshal(JsonVariant json) override;
    void unmarshal(JsonVariant json) override;

    void add(const String &uri, IConfig *source) { _apis.add(uri, source); }
    void add(const String &uri, display::Interface *source) { _display.enable(uri, source); }

    bool begin(const SoftAPConfig &softAP);
    bool loop();

    const String &id() const { return _id; }

    ControlSignal<String> state = {""};
    ControlSignal<String> status = {""};

private:
    enum StateMachine
    {
        UNSPECIFIED = 0,
        FAILED,
        CONNECT,
        WAIT_FOR_CONNECT_RESULT,
        ACTIVE,
    };
    StateMachine _state = {UNSPECIFIED};
    uint32_t _stateMillis = 0;

    class ApiRequestHandler : public RequestHandler
    {
    public:
        void add(const String &uri, IConfig *source) { _resources[uri] = source; }
        bool canHandle(HTTPMethod method, RequestHandlerUriParameter uri) override;
        bool handle(WebServer &server, HTTPMethod requestMethod, RequestHandlerUriParameter requestUri) override;

    private:
        std::map<String, IConfig *> _resources;
    };

    class DisplayRequestHandler : public RequestHandler
    {
    public:
        void enable(const String &uri, display::Interface *interface)
        {
            _uri = uri;
            _interface = interface;
        }
        bool canHandle(HTTPMethod method, RequestHandlerUriParameter uri) override;
        bool handle(WebServer &server, HTTPMethod requestMethod, RequestHandlerUriParameter requestUri) override;

    private:
        String _uri;
        display::Interface *_interface;
    };

    void handleNotFound();
    bool captivePortal();

    DNSServer dnsServer;
    WebServer server = {80};
    ApiRequestHandler _apis;
    DisplayRequestHandler _display;

    String _id;
    String _ssid;
    String _passphrase;

    unsigned int _status = WL_IDLE_STATUS;
};
