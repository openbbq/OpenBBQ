
#include "SysWiFi.h"

#if defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <ESPmDNS.h>
#elif defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#endif

#include <LittleFS.h>
#include <ArduinoJson.h>

const char *myHostname = "openbbq";

void SysWiFi::unmarshal(JsonVariant json)
{
    if (_state == SysWiFi::ACTIVE)
    {
        _state = SysWiFi::CONNECT;
    }
    _ssid = json["SSID"].as<String>();
    _passphrase = json["PASSPHRASE"].as<String>();

    Serial.println("Unmarshalling SysWiFi...");
    serializeJsonPretty(json, Serial);
    Serial.println();
}

void SysWiFi::marshal(JsonVariant json)
{
    json["SSID"] = _ssid;
    json["PASSPHRASE"] = _passphrase;

    Serial.println("Marshalling SysWiFi...");
    serializeJsonPretty(json, Serial);
    Serial.println();
}

bool SysWiFi::begin(const SoftAPConfig &softAP)
{
    Serial.println("Configuring access point...");

    if (!WiFi.softAPConfig(softAP.local_ip, softAP.gateway, softAP.subnet), softAP.dhcp_lease_start)
    {
        _state = SysWiFi::FAILED;
        _stateMillis = millis();
        return false;
    }
    delay(500); // Without delay I've seen the IP address blank
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());

    byte mac[6];
    WiFi.macAddress(mac);
    for (auto b : mac)
    {
        _id += "0123456789abcdef"[b / 0x10];
        _id += "0123456789abcdef"[b % 0x10];
    }

    if (!WiFi.softAP(softAP.ssid + "-" + _id, softAP.passphrase))
    {
        _state = SysWiFi::FAILED;
        _stateMillis = millis();
        return false;
    }

    /* Setup the DNS server redirecting all the domains to the apIP */
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(53, "*", softAP.local_ip);

    server.addHandler(&_apis);
    server.addHandler(&_display);
    server.serveStatic("/", LittleFS, "/www/");
    server.onNotFound([this]()
                      { this->handleNotFound(); });

    server.begin();

    Serial.println("HTTP server started");

    _state = SysWiFi::CONNECT;
    _stateMillis = millis();
    return true;
}

bool SysWiFi::loop()
{
    uint32_t elapsed = millis() - _stateMillis;
    unsigned int s = WiFi.status();

    switch (_state)
    {
    case SysWiFi::CONNECT:
        Serial.println("Connect requested");
        Serial.println("Connecting as wifi client...");
        WiFi.disconnect();
        WiFi.begin(_ssid, _passphrase);
        _state = SysWiFi::WAIT_FOR_CONNECT_RESULT;
        _stateMillis = millis();
        elapsed = 0;
        s = WiFi.status();
        // fall through
    case SysWiFi::WAIT_FOR_CONNECT_RESULT:
        if (s == WL_DISCONNECTED)
        {
            if (elapsed < 60000)
            {
                return false;
            }
        }
        Serial.print("connRes: ");
        Serial.println(s);
        _state = SysWiFi::ACTIVE;
        _stateMillis = millis();
        elapsed = 0;
        // fall through
    case SysWiFi::ACTIVE:
        if (s == WL_IDLE_STATUS && elapsed > 60000)
        {
            /* If WLAN disconnected and idle try to connect */
            /* Don't set retry time too low as retry interfere the softAP operation */
            _state = SysWiFi::CONNECT;
            _stateMillis = millis();
            elapsed = 0;
        }

        if (_status != s)
        { // WLAN status change
            Serial.print("Status: ");
            Serial.println(s);
            _status = s;
            if (s == WL_CONNECTED)
            {
                /* Just connected to WLAN */
                Serial.println("");
                Serial.print("Connected to ");
                Serial.println(_ssid);
                Serial.print("IP address: ");
                Serial.println(WiFi.localIP());

                // Setup MDNS responder
                if (!MDNS.begin(myHostname))
                {
                    Serial.println("Error setting up MDNS responder!");
                }
                else
                {
                    Serial.println("mDNS responder started");
                    // Add service to MDNS-SD
                    MDNS.addService("http", "tcp", 80);
                }
            }
            else if (s == WL_NO_SSID_AVAIL)
            {
                WiFi.disconnect();
            }
        }
    }

    // Do work:
    // DNS
    dnsServer.processNextRequest();
    // HTTP
    server.handleClient();

    state = String(_state);
    status = String(_status);
    return true;
}

/** Is this an IP? */
boolean isIp(String str)
{
    for (size_t i = 0; i < str.length(); i++)
    {
        int c = str.charAt(i);
        if (c != '.' && (c < '0' || c > '9'))
        {
            return false;
        }
    }
    return true;
}

/** IP to String? */
String toStringIp(IPAddress ip)
{
    String res = "";
    for (int i = 0; i < 3; i++)
    {
        res += String((ip >> (8 * i)) & 0xFF) + ".";
    }
    res += String(((ip >> 8 * 3)) & 0xFF);
    return res;
}

void SysWiFi::handleNotFound()
{
    if (captivePortal())
    { // If caprive portal redirect instead of displaying the error page.
        return;
    }
    String message = F("File Not Found\n\n");
    message += F("URI: ");
    message += server.uri();
    message += F("\nMethod: ");
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += F("\nArguments: ");
    message += server.args();
    message += F("\n");

    for (uint8_t i = 0; i < server.args(); i++)
    {
        message += String(F(" ")) + server.argName(i) + F(": ") + server.arg(i) + F("\n");
    }
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");
    server.send(404, "text/plain", message);
}

/** Redirect to captive portal if we got a request for another domain. Return true in that case so the page handler do not try to handle the request again. */
boolean SysWiFi::captivePortal()
{
    if (!isIp(server.hostHeader()) && server.hostHeader() != (String(myHostname) + ".local"))
    {
        Serial.println("Request redirected to captive portal");
        server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()), true);
        server.send(302, "text/plain", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
        server.client().stop();             // Stop is needed because we sent no content length
        return true;
    }
    return false;
}

bool SysWiFi::ApiRequestHandler::canHandle(HTTPMethod method, RequestHandlerUriParameter uri)
{
    return (method == HTTP_GET || method == HTTP_PUT) && _resources.find(uri) != _resources.end();
}

bool SysWiFi::ApiRequestHandler::handle(WebServer &server, HTTPMethod requestMethod, RequestHandlerUriParameter requestUri)
{
    if (!canHandle(requestMethod, requestUri))
    {
        return false;
    }

    auto resource = _resources.find(requestUri);

    StaticJsonDocument<1024> doc;
    if (requestMethod == HTTP_GET)
    {
        resource->second->marshal(doc);
    }
    else if (requestMethod == HTTP_PUT)
    {
        if (!server.hasArg("plain"))
        {
            doc["error"] = "invalid request";
        }
        else
        {
            auto err = deserializeJson(doc, server.arg("plain"));
            if (err)
            {
                doc["error"] = err.c_str();
            }
            else
            {
                resource->second->unmarshal(doc);
                resource->second->changed(true);
            }
        }
    }

    server.setContentLength(measureJson(doc));
    server.send(200, "application/json", "");

    auto client = server.client();
    serializeJson(doc, client);
    return true;
}

bool SysWiFi::DisplayRequestHandler::canHandle(HTTPMethod method, RequestHandlerUriParameter uri)
{
    return method == HTTP_GET && _uri == uri;
}

WiFiClient &operator<<(WiFiClient &client, uint16_t value)
{
    log_d("uint16_t:%04x", value);
    client.write(uint8_t(value));
    client.write(uint8_t(value >> 8));
    return client;
}

WiFiClient &operator<<(WiFiClient &client, uint32_t value)
{
    log_d("uint32_t:%08x", value);
    client.write(uint8_t(value));
    client.write(uint8_t(value >> 8));
    client.write(uint8_t(value >> 16));
    client.write(uint8_t(value >> 24));
    return client;
}

WiFiClient &operator<<(WiFiClient &client, int32_t value)
{
    return operator<<(client, *(uint32_t *)(&value));
}

#include <DrawContext_Adafruit_GFX.h>
class ImageBand_GFX : public Adafruit_GFX
{
public:
    ImageBand_GFX(int16_t width, int16_t height, int16_t top, int16_t bottom, uint16_t *pixels)
        : Adafruit_GFX(width, height), _top(top), _bottom(bottom), _pixels(pixels)
    {
    }

    void drawPixel(int16_t x, int16_t y, uint16_t color) override
    {
        if (y < _top || y >= _bottom || x < 0 || x >= _width)
        {
            return;
        }

        // int bandRow = y - _top;
        // int bmpRow = (_bottom - _top) - bandRow - 1;

        _pixels[x + _width * (_bottom - y - 1)] = color;
    }

private:
    int16_t _top;
    int16_t _bottom;
    uint16_t *_pixels;
};

// See references
// "Saving Screenshots from a TFT Display" http://www.technoblogy.com/show?398X
// "Simplified Windows BMP Bitmap File Format Specification" https://cdn.hackaday.io/files/274271173436768/Simplified%20Windows%20BMP%20Bitmap%20File%20Format%20Specification.htm#The%20Image%20Header
bool SysWiFi::DisplayRequestHandler::handle(WebServer &server, HTTPMethod requestMethod, RequestHandlerUriParameter requestUri)
{
    if (!canHandle(requestMethod, requestUri))
    {
        return false;
    }

    display::Size sz = _interface->size();
    uint16_t width = sz.width();
    uint16_t height = sz.height();
    size_t contentLength = 14 + 40 + 12 + width * height * 2;
    server.setContentLength(contentLength);
    server.send(200, "image/bmp", "");

    auto client = server.client();

    //
    // File header: 14 bytes
    client.write('B');
    client.write('M');
    client << uint32_t(contentLength) // File size in bytes
           << uint32_t(0)
           << uint32_t(14 + 40 + 12); // Offset to image data from start

    //
    // Image header: 40 bytes
    client << uint32_t(40)     // Header size
           << uint32_t(width)  // Image width
           << uint32_t(height) // Image height
           << uint16_t(1)      // Planes
           << uint16_t(16)     // Bits per pixel
           << uint32_t(3)      // Compression (none)
           << uint32_t(0)      // Image size (0 for uncompressed)
           << uint32_t(0)      // Preferred X resolution (ignore)
           << uint32_t(0)      // Preferred Y resolution (ignore)
           << uint32_t(0)      // Colour map entries (ignore)
           << uint32_t(0);     // Important colours (ignore)

    //
    // Colour masks: 12 bytes
    client << uint32_t(0b1111100000000000)  // Red
           << uint32_t(0b0000011111100000)  // Green
           << uint32_t(0b0000000000011111); // Blue

    //
    // Image data: width * height * 2 bytes

    const uint32_t bandPixels = 20000; // ~40k ram max
    uint16_t bandWidth = width;
    uint16_t bandHeight = bandPixels / bandWidth;
    std::vector<uint16_t> bandMemory(bandWidth * bandHeight);
    int16_t bandCount = (height + bandHeight - 1) / bandHeight;

    log_i("bitmap of %dx%d screen in %d bands of %d height", width, height, bandCount, bandHeight);

    int16_t top = (bandCount - 1) * bandHeight;
    int16_t bottom = height;
    while (bottom != 0)
    {
        log_v("starting band %d:%d", top, bottom);
        display::Clipped_GFX<ImageBand_GFX> gfx(width, height, top, bottom, &bandMemory[0]);

        display::Region region;
        region.include(display::Rect(0, top, width, bottom));

        display::DrawContext_Adafruit_GFX dc(&gfx, &gfx);
        dc.initialize(display::Rect(0, 0, width, height), region);

        _interface->screen()->drawHandler(&dc);

        client.write((uint8_t *)&bandMemory[0], width * (bottom - top) * sizeof(uint16_t));

        bottom = top;
        top -= bandHeight;
    }

    return true;
}
