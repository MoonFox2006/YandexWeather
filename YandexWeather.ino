#define ENABLE_GxEPD2_GFX 0

//#define SIMULATION

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Update.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <GxEPD2_4G_4G.h>
//#include "Fonts/Arial32.h"
//#include "Fonts/Arial24.h"
#include "Fonts/Arial18.h"
#include "Fonts/Arial14.h"
#include "Fonts/Arial12.h"
#include <qrcode.h>
#include "Parameters.h"
#include "private.h"
#include "blinks.h"
#include "HtmlHelper.h"
#include "StrUtils.h"
#include "favicon.h"
#include "icons.h"

struct __attribute__((__packed__)) config_t {
  char wifi_ssid[32];
  char wifi_pswd[64];
  uint16_t wifi_timeout;

  char yw_key[37];
  float latitude;
  float longitude;
};

enum align_t : uint8_t { ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT };

constexpr uint8_t LED_PIN = 8;
constexpr uint8_t LED_LEVEL = LOW;
constexpr uint8_t LED_BRIGHT = 127; // 50%

constexpr uint8_t BTN_PIN = 9;
constexpr uint8_t BTN_LEVEL = LOW;

constexpr int8_t EPD_SCK = 4;
constexpr int8_t EPD_MISO = -1;
constexpr int8_t EPD_MOSI = 5;
constexpr int8_t EPD_SS = 6;
constexpr int8_t EPD_DC = 0;
constexpr int8_t EPD_RES = 1;
constexpr int8_t EPD_BUSY = 2;

#define SET_STR_PARAM(p, v) strlcpy(p, v, sizeof(p))
#define STRSIZE(s)          (sizeof(s) - 1)

#define PARAM_WIFI_SSID     "wifi_ssid"
#define PARAM_WIFI_PSWD     "wifi_pswd"
#define PARAM_WIFI_TIMEOUT  "wifi_timeout"

#define PARAM_YW_KEY        "yw_key"
#define PARAM_LATITUDE      "lat"
#define PARAM_LONGITUDE     "lon"

#define CP_SSID     "YW32"
#define CP_PSWD     "1029384756"
#define CP_CHANNEL  1

#define URL_ROOT    "/"
#define URL_WIFI    "/wifi"
#define URL_STORE   "/store"
#define URL_RESTART "/restart"
#define URL_RESET   "/reset"
#define URL_OTA     "/ota"

#ifdef SIMULATION
const char *ANSWER = "{\"now\":1751541263,\"now_dt\":\"2025-07-03T11:14:23.165517Z\",\"info\":{\"n\":true,\"url\":\"https://yandex.ru/pogoda/?lat=0\u0026lon=0\",\"lat\":0,\"lon\":0,\"tzinfo\":{\"name\":\"Europe/Moscow\",\"abbr\":\"MSK\",\"dst\":false,\"offset\":10800},\"def_pressure_mm\":747,\"def_pressure_pa\":995,\"zoom\":10,\"nr\":true,\"ns\":true,\"nsr\":true,\"p\":false,\"f\":true,\"_h\":false},\"fact\":{\"daytime\":\"d\",\"obs_time\":1751541263,\"season\":\"summer\",\"source\":\"station\",\"uptime\":1751541263,\"cloudness\":0.5,\"condition\":\"cloudy\",\"feels_like\":24,\"humidity\":45,\"icon\":\"bkn_d\",\"is_thunder\":false,\"polar\":false,\"prec_prob\":0,\"prec_strength\":0,\"prec_type\":0,\"temp\":25,\"uv_index\":6,\"wind_angle\":320,\"wind_dir\":\"nw\",\"wind_gust\":10.7,\"wind_speed\":4.4},\"forecasts\":[{\"date\":\"2025-07-03\",\"date_ts\":1751490000,\"week\":27,\"sunrise\":\"03:50\",\"sunset\":\"21:16\",\"rise_begin\":\"02:51\",\"set_end\":\"22:16\",\"moon_code\":12,\"moon_text\":\"moon-code-12\",\"parts\":{\"day\":{\"daytime\":\"d\",\"_source\":\"12,13,14,15,16,17\",\"cloudness\":0.75,\"condition\":\"cloudy\",\"fresh_snow_mm\":0,\"humidity\":46,\"icon\":\"bkn_d\",\"polar\":false,\"prec_mm\":0,\"prec_period\":360,\"prec_prob\":0,\"prec_strength\":0,\"prec_type\":0,\"temp_avg\":25,\"temp_max\":26,\"temp_min\":23,\"feels_like\":24,\"uv_index\":6,\"wind_angle\":315,\"wind_dir\":\"nw\",\"wind_gust\":10.9,\"wind_speed\":4.7},\"day_short\":{\"daytime\":\"d\",\"_source\":\"6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21\",\"cloudness\":0.75,\"condition\":\"cloudy\",\"fresh_snow_mm\":0,\"humidity\":56,\"icon\":\"bkn_d\",\"polar\":false,\"prec_mm\":0,\"prec_period\":960,\"prec_prob\":0,\"prec_strength\":0,\"prec_type\":0,\"temp\":26,\"temp_min\":15,\"feels_like\":21,\"uv_index\":6,\"wind_angle\":315,\"wind_dir\":\"nw\",\"wind_gust\":10.9,\"wind_speed\":4.7},\"evening\":{\"daytime\":\"d\",\"_source\":\"18,19,20,21\",\"cloudness\":0.5,\"condition\":\"cloudy\",\"humidity\":55,\"icon\":\"bkn_d\",\"polar\":false,\"prec_period\":240,\"prec_strength\":0,\"prec_type\":0,\"temp_avg\":25,\"temp_max\":26,\"temp_min\":23,\"feels_like\":25,\"uv_index\":1,\"wind_angle\":270,\"wind_dir\":\"w\",\"wind_gust\":8.2,\"wind_speed\":2.3},\"morning\":{\"daytime\":\"d\",\"_source\":\"6,7,8,9,10,11\",\"cloudness\":1,\"condition\":\"overcast\",\"humidity\":66,\"icon\":\"ovc\",\"polar\":false,\"prec_period\":360,\"prec_strength\":0,\"prec_type\":0,\"temp_avg\":19,\"temp_max\":21,\"temp_min\":15,\"feels_like\":18,\"uv_index\":6,\"wind_angle\":315,\"wind_dir\":\"nw\",\"wind_gust\":10.6,\"wind_speed\":4.3},\"night\":{\"daytime\":\"n\",\"_source\":\"0,1,2,3,4,5\",\"cloudness\":0.75,\"condition\":\"light-rain\",\"humidity\":85,\"icon\":\"bkn_-ra_n\",\"polar\":false,\"prec_period\":480,\"prec_strength\":0.25,\"prec_type\":1,\"temp_avg\":14,\"temp_max\":15,\"temp_min\":13,\"feels_like\":12,\"uv_index\":0,\"wind_angle\":315,\"wind_dir\":\"nw\",\"wind_gust\":8.9,\"wind_speed\":3.3},\"night_short\":{\"daytime\":\"n\",\"_source\":\"0,1,2,3,4,5\",\"cloudness\":0.75,\"condition\":\"light-rain\",\"humidity\":85,\"icon\":\"bkn_-ra_n\",\"polar\":false,\"prec_period\":480,\"prec_strength\":0.25,\"prec_type\":1,\"temp\":13,\"feels_like\":12,\"uv_index\":0,\"wind_angle\":315,\"wind_dir\":\"nw\",\"wind_gust\":8.9,\"wind_speed\":3.3}},\"hours\":[]},{\"date\":\"2025-07-04\",\"date_ts\":1751576400,\"week\":27,\"sunrise\":\"03:51\",\"sunset\":\"21:16\",\"rise_begin\":\"02:53\",\"set_end\":\"22:15\",\"moon_code\":13,\"moon_text\":\"moon-code-13\",\"parts\":{\"day\":{\"daytime\":\"d\",\"_source\":\"12,13,14,15,16,17\",\"cloudness\":0.25,\"condition\":\"light-rain\",\"humidity\":50,\"icon\":\"bkn_-ra_d\",\"polar\":false,\"prec_period\":360,\"prec_strength\":0.25,\"prec_type\":1,\"temp_avg\":25,\"temp_max\":26,\"temp_min\":25,\"feels_like\":24,\"uv_index\":5,\"wind_angle\":270,\"wind_dir\":\"w\",\"wind_gust\":12.2,\"wind_speed\":5.3},\"day_short\":{\"daytime\":\"d\",\"_source\":\"6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21\",\"cloudness\":0.5,\"condition\":\"light-rain\",\"humidity\":54,\"icon\":\"bkn_-ra_d\",\"polar\":false,\"prec_period\":960,\"prec_strength\":0.25,\"prec_type\":1,\"temp\":26,\"temp_min\":19,\"temp_water\":19,\"feels_like\":23,\"uv_index\":7,\"wind_angle\":225,\"wind_dir\":\"sw\",\"wind_gust\":12.2,\"wind_speed\":5.3},\"evening\":{\"daytime\":\"d\",\"_source\":\"18,19,20,21\",\"cloudness\":0.75,\"condition\":\"cloudy\",\"humidity\":46,\"icon\":\"bkn_d\",\"polar\":false,\"prec_period\":240,\"prec_strength\":0,\"prec_type\":0,\"temp_avg\":22,\"temp_max\":24,\"temp_min\":20,\"feels_like\":19,\"uv_index\":1,\"wind_angle\":315,\"wind_dir\":\"nw\",\"wind_gust\":11,\"wind_speed\":4.4},\"morning\":{\"daytime\":\"d\",\"_source\":\"6,7,8,9,10,11\",\"cloudness\":0.25,\"condition\":\"partly-cloudy\",\"humidity\":63,\"icon\":\"skc_d\",\"polar\":false,\"prec_period\":360,\"prec_strength\":0,\"prec_type\":0,\"temp_avg\":23,\"temp_max\":26,\"temp_min\":19,\"feels_like\":23,\"uv_index\":7,\"wind_angle\":225,\"wind_dir\":\"sw\",\"wind_gust\":12.1,\"wind_speed\":4.7},\"night\":{\"daytime\":\"n\",\"_source\":\"22,23,0,1,2,3,4,5\",\"cloudness\":0.25,\"condition\":\"partly-cloudy\",\"humidity\":78,\"icon\":\"skc_n\",\"polar\":false,\"prec_period\":480,\"prec_strength\":0,\"prec_type\":0,\"temp_avg\":19,\"temp_max\":21,\"temp_min\":18,\"feels_like\":19,\"uv_index\":0,\"wind_angle\":225,\"wind_dir\":\"sw\",\"wind_gust\":2.8,\"wind_speed\":2.1},\"night_short\":{\"daytime\":\"n\",\"_source\":\"22,23,0,1,2,3,4,5\",\"cloudness\":0.25,\"condition\":\"partly-cloudy\",\"humidity\":78,\"icon\":\"skc_n\",\"polar\":false,\"prec_period\":480,\"prec_strength\":0,\"prec_type\":0,\"temp\":18,\"feels_like\":19,\"uv_index\":0,\"wind_angle\":225,\"wind_dir\":\"sw\",\"wind_gust\":2.8,\"wind_speed\":2.1}},\"hours\":[]}]}";
#endif

// 4.2" EPD Module
GxEPD2_4G_4G<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> display(GxEPD2_420_GDEY042T81(EPD_SS, EPD_DC, EPD_RES, EPD_BUSY)); // GDEY042T81, 400x300, SSD1683 (no inking)
Parameters<config_t> config;
blinks_handle_t blinks = nullptr;
WebServer www;
JsonDocument json;
uint32_t timestamp = 0;

static void initDisplay() {
  SPI.begin(EPD_SCK, EPD_MISO, EPD_MOSI);
  display.init(0, true);
  display.setFullWindow();
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
}

static void drawText(int16_t x, int16_t y, const char *str, align_t align = ALIGN_LEFT, uint16_t color = GxEPD_BLACK, const GFXfont *font = nullptr) {
  int16_t tbx, tby;
  uint16_t tbw, tbh;

  if (font)
    display.setFont(font);
  display.getTextBounds(str, 0, 0, &tbx, &tby, &tbw, &tbh);
  if (align == ALIGN_CENTER)
    x -= tbw / 2;
  else if (align == ALIGN_RIGHT)
    x -= tbw;
  display.setCursor(x - tbx, y - tby);
  display.setTextColor(color);
  display.print(str);
}

static void drawIcon(int16_t x, int16_t y, const uint8_t *data, uint16_t width, uint16_t height, uint8_t scale = 1) {
  const uint16_t COLORS[] = { GxEPD_BLACK, GxEPD_DARKGREY, GxEPD_LIGHTGREY, GxEPD_WHITE };

  for (uint16_t i = 0; i < (width / 4) * height; ++i) {
    uint8_t bits = data[i];

    for (uint8_t j = 0; j < 8; j += 2) {
      uint16_t color = COLORS[(bits >> j) & 0x03];

      if (color != GxEPD_WHITE) {
        if (scale > 1)
          display.fillRect(x + ((i % (width / 4)) * 4 + j / 2) * scale, y + (i / (width / 4)) * scale, scale, scale, color);
        else
          display.drawPixel(x + (i % (width / 4)) * 4 + j / 2, y + i / (width / 4), color);
      }
    }
  }
}

static const uint8_t *findIcon(const char *name) {
  for (uint8_t i = 0; i < sizeof(ICONS) / sizeof(ICONS[0]); ++i) {
    if (! strcasecmp(ICONS[i].name, name))
      return ICONS[i].data;
  }
  return nullptr;
}

static const char *translateCondition(const char *condition) {
  const struct {
    const char *eng;
    const char *rus;
  } CONDITIONS[] = {
    {
      .eng = "clear",
      .rus = "\xDF\xF1\xED\xEE" // "Ясно"
    },
    {
      .eng = "partly-cloudy",
      .rus = "\xCC\xE0\xEB\xEE\xEE\xE1\xEB\xE0\xF7\xED\xEE" // "Малооблачно"
    },
    {
      .eng = "cloudy",
      .rus = "\xCE\xE1\xEB\xE0\xF7\xED\xEE\x20\xF1\x20\xEF\xF0\xEE\xFF\xF1\xED\xE5\xED\xE8\xFF\xEC\xE8" // "Облачно с прояснениями"
    },
    {
      .eng = "overcast",
      .rus = "\xCF\xE0\xF1\xEC\xF3\xF0\xED\xEE" // "Пасмурно"
    },
    {
      .eng = "light-rain",
      .rus = "\xCD\xE5\xE1\xEE\xEB\xFC\xF8\xEE\xE9\x20\xE4\xEE\xE6\xE4\xFC" // "Небольшой дождь"
    },
    {
      .eng = "rain",
      .rus = "\xC4\xEE\xE6\xE4\xFC" // "Дождь"
    },
    {
      .eng = "heavy-rain",
      .rus = "\xD1\xE8\xEB\xFC\xED\xFB\xE9\x20\xE4\xEE\xE6\xE4\xFC" // "Сильный дождь"
    },
    {
      .eng = "showers",
      .rus = "\xCB\xE8\xE2\xE5\xED\xFC" // "Ливень"
    },
    {
      .eng = "wet-snow",
      .rus = "\xC4\xEE\xE6\xE4\xFC\x20\xF1\xEE\x20\xF1\xED\xE5\xE3\xEE\xEC" // "Дождь со снегом"
    },
    {
      .eng = "light-snow",
      .rus = "\xCD\xE5\xE1\xEE\xEB\xFC\xF8\xEE\xE9\x20\xF1\xED\xE5\xE3" // "Небольшой снег"
    },
    {
      .eng = "snow",
      .rus = "\xD1\xED\xE5\xE3" // "Снег"
    },
    {
      .eng = "snow-showers",
      .rus = "\xD1\xED\xE5\xE3\xEE\xEF\xE0\xE4" // "Снегопад"
    },
    {
      .eng = "hail",
      .rus = "\xC3\xF0\xE0\xE4" // "Град"
    },
    {
      .eng = "thunderstorm",
      .rus = "\xC3\xF0\xEE\xE7\xE0" // "Гроза"
    },
    {
      .eng = "thunderstorm-with-rain",
      .rus = "\xC4\xEE\xE6\xE4\xFC\x20\xF1\x20\xE3\xF0\xEE\xE7\xEE\xE9" // "Дождь с грозой"
    },
    {
      .eng = "thunderstorm-with-hail",
      .rus = "\xC3\xF0\xEE\xE7\xE0\x20\xF1\x20\xE3\xF0\xE0\xE4\xEE\xEC" // "Гроза с градом"
    }
  };

  for (uint8_t i = 0; i < sizeof(CONDITIONS) / sizeof(CONDITIONS[0]); ++i) {
    if (! strcasecmp(CONDITIONS[i].eng, condition))
      return CONDITIONS[i].rus;
  }
  return "";
}

static uint16_t drawFact(const JsonObjectConst fact) {
  char str[16];
  const uint8_t *icon;
  constexpr uint16_t left = ICO_WIDTH * 3;
  uint16_t top = 0;

  display.setFont(&Arial14);
  drawText(0, top, translateCondition(fact["condition"]));
  top += 18;
  icon = findIcon(fact["icon"]);
  if (icon)
    drawIcon(0, top, icon, ICO_WIDTH, ICO_HEIGHT, 3);
  top += 14;
  display.setFont(&Arial18);
  snprintf(str, sizeof(str), "%.0f\x7F""C", fact["temp"].as<float>());
  drawText(left, top, str);
  top += 32;
  snprintf(str, sizeof(str), "%.0f%%", fact["humidity"].as<float>());
  drawText(left, top, str);
  top += 32;
/*
  if (! fact["pressure_pa"].isNull())
    snprintf(str, sizeof(str), "%.0f hPa", fact["pressure_pa"].as<float>());
  else
    strlcpy(str, "- hPa", sizeof(str));
  drawText(left, top, str);
  top += 32;
*/
  snprintf(str, sizeof(str), "%.1f \xEC/\xF1", fact["wind_speed"].as<float>()); // "м/с"
  if ((! fact["wind_dir"].isNull()) && strcasecmp(fact["wind_dir"], "c")) {
    auto len = strlen(str);

    str[len++] = ' ';
    strcpy(&str[len], fact["wind_dir"]); // strcat
//    strupr(&str[len]);
    while (str[len]) {
      if ((str[len] == 'N') || (str[len] == 'n'))
        str[len] = '\xD1'; // 'С'
      else if ((str[len] == 'S') || (str[len] == 's'))
        str[len] = '\xDE'; // 'Ю'
      else if ((str[len] == 'W') || (str[len] == 'w'))
        str[len] = '\xC7'; // 'З'
      else if ((str[len] == 'E') || (str[len] == 'e'))
        str[len] = '\xC2'; // 'В'
      ++len;
    }
  }
  drawText(left, top, str);
  top += 32;
  if (top < ICO_HEIGHT * 3 + 18)
    top = ICO_HEIGHT * 3 + 18;
  return top;
}

static void getHours(char *str, const char *source) {
  while (*source != ',') {
    *str++ = *source++;
  }
  *str++ = '-';
  source = strrchr(source, ',');
  if (source) {
    ++source; // Skip ','
    while (*source) {
      *str++ = *source++;
    }
  }
  *str = '\0';
}

static void drawPart(const JsonObjectConst part, uint16_t x, uint16_t y) {
  char str[10];
  const uint8_t *icon;

  if (x > 0)
    display.drawLine(x, y, x, display.height() - 1, GxEPD_BLACK);
  display.setFont(&Arial14);
  getHours(str, part["_source"]);
  drawText(x + 50, y, str, ALIGN_CENTER, GxEPD_DARKGREY);
  y += 28;
  icon = findIcon(part["icon"]);
  if (icon)
    drawIcon(x + 26, y, icon, ICO_WIDTH, ICO_HEIGHT);
  y += ICO_HEIGHT;
//  display.setFont(&Arial12);
  snprintf(str, sizeof(str), "%.0f\x7F", part["temp_avg"].as<float>());
  drawText(x + 50, y, str, ALIGN_CENTER);
  y += 28;
  snprintf(str, sizeof(str), "%.0f%%", part["humidity"].as<float>());
  drawText(x + 50, y, str, ALIGN_CENTER);
//  y += 28;
}

static uint16_t drawForecast(const JsonObjectConst forecast, int8_t start, uint16_t x, uint16_t y) {
  const char *PARTS[] = {
    "night", "morning", "day", "evening"
  };

  const JsonObjectConst parts = forecast["parts"];
  uint8_t i = 0;

  if (start >= 0) {
    if (start < 6)
      i = 0; // night
    else if (start < 12)
      i = 1; // morning
    else if (start < 18)
      i = 2; // day
    else if (start < 22)
      i = 3; // evening
  }
  while (i < sizeof(PARTS) / sizeof(PARTS[0])) {
    drawPart(parts[PARTS[i]], x, y);
    x += 100;
    if (x >= display.width())
      break;
    ++i;
  }
  return x;
}

static void drawForecasts(const JsonArrayConst forecasts, uint8_t start, uint16_t top) {
  uint16_t x = 0;

  x = drawForecast(forecasts[0], start, x, top);
  if (x < display.width()) {
    drawForecast(forecasts[1], -1, x, top);
  }
}

static void drawTimestamp(uint32_t timestamp) {
  char str[10];

  snprintf(str, sizeof(str), "%u:%02u:%02u", (uint8_t)(timestamp / 3600), (uint8_t)((timestamp % 3600) / 60), (uint8_t)(timestamp % 60));
  display.setFont(&Arial12);
  drawText(display.width() - 1, 32, str, ALIGN_RIGHT, GxEPD_LIGHTGREY);
  Serial.println(str);
}

static void drawWeather() {
  uint16_t top;

  initDisplay();

  top = drawFact(json["fact"]) + 8;
  drawForecasts(json["forecasts"], timestamp / 3600, top);
  drawTimestamp(timestamp);

  display.display();
  Serial.printf("%.1f C, %.1f%%\n", json["fact"]["temp"].as<float>(), json["fact"]["humidity"].as<float>());
}

static bool getWeather() {
#ifndef SIMULATION
  HTTPClient http;
  String str;

  http.setTimeout(15000);
  str = strFormat("https://api.weather.yandex.ru/v2/forecast?lat=%.6f&lon=%.6f&limit=2&extra=false&hours=false", config->latitude, config->longitude);
  Serial.println(str);
  if (http.begin(str)) {
    int code;

    str.clear();
    http.addHeader("X-Yandex-Weather-Key", config->yw_key);
    code = http.GET();
    if (code == HTTP_CODE_OK) {
#endif
      DeserializationError error;

#ifndef SIMULATION
      str = http.getString();
      Serial.println(str);
      error = deserializeJson(json, str);
      str.clear();
#else
      error = deserializeJson(json, ANSWER);
#endif
      if (! error) {
        timestamp = (json["now"].as<uint32_t>() + json["info"]["tzinfo"]["offset"].as<uint32_t>()) % (3600 * 24);
        return true;
      } else {
        Serial.printf("JSON parse error: \"%s\"!\n", error.c_str());
      }
#ifndef SIMULATION
    } else {
      Serial.printf("Wrong HTTP code %d!\n", code);
      Serial.println(http.getString());
    }
    http.end();
  } else {
    Serial.println("Open URL error!");
  }
#endif
  return false;
}

static bool wifiConnect(const char *ssid, const char *pswd, uint32_t timeout) {
  uint32_t start;

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pswd);
  start = millis();
  blinks_update(blinks, 0, BLINK_2HZ, LED_BRIGHT);
  Serial.printf("Connecting to \"%s\"", ssid);
  while ((! WiFi.isConnected()) && (millis() - start < timeout)) {
    delay(500);
    Serial.write('.');
  }
  blinks_update(blinks, 0, BLINK_OFF, LED_BRIGHT);
  if (WiFi.isConnected()) {
    Serial.print(" OK (IP: ");
    Serial.print(WiFi.localIP());
    Serial.println(')');
    return true;
  } else {
    WiFi.disconnect();
    Serial.println(" FAIL!");
    return false;
  }
}

static void drawQrCode(esp_qrcode_handle_t qrcode) {
  int size = esp_qrcode_get_size(qrcode);
  uint16_t halfHeight = display.height() * 2 / 3;
  uint8_t dot = halfHeight / size;
  uint8_t offset = (halfHeight - size * dot) / 2;

  for (int y = 0; y < size; ++y) {
    for (int x = 0; x < size; ++x) {
      if (esp_qrcode_get_module(qrcode, x, y)) {
        if (dot > 1)
          display.fillRect(x * dot + offset, y * dot + offset, dot, dot, GxEPD_BLACK);
        else
          display.drawPixel(x + offset, y + offset, GxEPD_BLACK);
      }
    }
  }
  display.setFont(&Arial12);
  drawText(halfHeight + 8, 0, "Connect to", ALIGN_LEFT, GxEPD_DARKGREY);
  drawText(halfHeight + 8, 24, CP_SSID);
  drawText(halfHeight + 8, 48, "with password", ALIGN_LEFT, GxEPD_DARKGREY);
  drawText(halfHeight + 8, 72, CP_PSWD);
  drawText(halfHeight + 8, 96, "and visit to", ALIGN_LEFT, GxEPD_DARKGREY);
  drawText(halfHeight + 8, 120, WiFi.softAPIP().toString().c_str());
}

static bool wifiQrCode(const char *ssid, const char *pswd) {
  const esp_qrcode_config_t cfg = {
    .display_func = drawQrCode,
    .max_qrcode_version = 20, // 97x97
    .qrcode_ecc_level = ESP_QRCODE_ECC_LOW
  };

  char str[120];

  snprintf(str, sizeof(str), "WIFI:T:WPA;S:%s;P:%s;;", ssid, pswd);
  return esp_qrcode_generate((esp_qrcode_config_t*)&cfg, str) == ESP_OK;
}

static String encodeString(const char *str, uint16_t len = 0) {
  String result;

  if (len)
    result.reserve(len);
  while (*str) {
    if (*str == '\'')
      result.concat("&apos;");
    else if (*str == '"')
      result.concat("&quot;");
    else if (*str == '<')
      result.concat("&lt;");
    else if (*str == '>')
      result.concat("&gt;");
    else if (*str == '&')
      result.concat("&amp;");
    else
      result.concat(*str);
    if (len) {
      if (! --len)
        break;
    }
    ++str;
  }
  return result;
}

static void webNotFound() {
  if (! www.hostHeader().equals(WiFi.softAPIP().toString())) {
    www.sendHeader("Location", "http://" + WiFi.softAPIP().toString(), true);
    www.send(302, TEXT_PLAIN, "");
  } else {
    if (www.uri().equalsIgnoreCase("/favicon.ico")) {
/*
      extern const uint8_t favicon_ico_start[] asm("_binary_favicon_ico_start");
      extern const uint8_t favicon_ico_end[] asm("_binary_favicon_ico_end");

      www.send_P(200, "image/x-icon", (const char*)favicon_ico_start, favicon_ico_end - favicon_ico_start);
*/
      www.send_P(200, "image/x-icon", (const char*)FAVICON, sizeof(FAVICON));
    } else
      www.send(404, TEXT_PLAIN, "Page not found!");
  }
}

static void webRoot() {
  String page;

  page = HTML_HEADER("Yandex Weather")
    HTML_STYLE("body{background-color:#eee;}\n"
      ".roundbtn{border:1px solid gray;border-radius:5px;padding:5px 10px;margin:10px 0;width:20%}\n")
    HTML_SCRIPT(
      "function refreshWiFi(btn){\n"
      "let x=new XMLHttpRequest();\n"
      "x.open('GET','" URL_WIFI "?dummy='+Date.now(),true);\n"
      "x.onreadystatechange=function(){\n"
      "if((x.readyState==4)&&(x.status==200)){\n"
      "let d=JSON.parse(x.responseText);\n"
      "if(Array.isArray(d.wifi)){\n"
      "let list=document.getElementById('wifi');\n"
      "list.innerHTML='';\n"
      "for(let i=0;i<d.wifi.length;++i){\n"
      "let option=document.createElement('option');\n"
      "option.value=d.wifi[i];\n"
      "list.appendChild(option);\n"
      "}\n"
      "}\n"
      "btn.disabled=false;\n"
      "}\n"
      "}\n"
      "x.send(null);\n"
      "btn.disabled=true;\n"
      "}\n"
      "function getLocation(){\n"
      "if(navigator.geolocation){\n"
      "navigator.geolocation.getCurrentPosition(function(position){\n"
      "document.getElementById('" PARAM_LATITUDE "').value=position.coords.latitude;\n"
      "document.getElementById('" PARAM_LONGITUDE "').value=position.coords.longitude;\n"
      "},function(){\n"
      "alert('Unable to retrieve your location!');\n"
      "});\n"
      "}else{\n"
      "alert('Geolocation is not supported by your browser!');\n"
      "}\n"
      "}\n"
      JS_TRIM
      JS_VALIDATE_INT
      JS_VALIDATE_FLOAT)
    HTML_BODY
    HTML_TAG("h3", "Yandex Weather") "\n"
    "<form method=\"post\" action=\"" URL_STORE "\">\n"
    "<label for=\"" PARAM_WIFI_SSID "\">WiFi SSID:</label><br/>\n"
    "<input type=\"text\" name=\"" PARAM_WIFI_SSID "\" size=30 maxlength=31 value=\"";
  page.concat(encodeString(config->wifi_ssid, STRSIZE(config_t::wifi_ssid)));
  page.concat("\" onblur=\"trim(this)\" list=\"wifi\">\n"
    "<input type=\"button\" value=\"Refresh\" onclick=\"refreshWiFi(this)\"><br/>\n"
    "<datalist id=\"wifi\"></datalist>\n"
    "<label for=\"" PARAM_WIFI_PSWD "\">WiFi password:</label><br/>\n"
    "<input type=\"password\" name=\"" PARAM_WIFI_PSWD "\" size=30 maxlength=63 value=\"");
  page.concat(encodeString(config->wifi_pswd, STRSIZE(config_t::wifi_pswd)));
  page.concat("\"><br/>\n"
    "<label for=\"" PARAM_WIFI_TIMEOUT "\">WiFi timeout (sec.):</label><br/>\n"
    "<input type=\"text\" name=\"" PARAM_WIFI_TIMEOUT "\" size=5 maxlength=5 value=\"");
  page.concat(config->wifi_timeout);
  page.concat("\" onblur=\"validateInt(this,15,65535)\"><br/><br/>\n"
    "<label for=\"" PARAM_YW_KEY "\">YW key:</label><br/>\n"
    "<input type=\"text\" name=\"" PARAM_YW_KEY "\" size=30 maxlength=36 value=\"");
  page.concat(encodeString(config->yw_key, STRSIZE(config_t::yw_key)));
  page.concat("\" onblur=\"trim(this)\"><br/>\n"
    "<label for=\"" PARAM_LATITUDE "\">Latitude:</label><br/>\n"
    "<input type=\"text\" name=\"" PARAM_LATITUDE "\" size=11 maxlength=11 value=\"");
  if (! isnan(config->latitude))
    page.concat(strFormat("%.6f", config->latitude));
  page.concat("\" onblur=\"validateFloat(this,-90,90)\">\n"
    "<input type=\"button\" value=\"Locate\" onclick=\"getLocation()\"><br/>\n"
    "<label for=\"" PARAM_LONGITUDE "\">Longitude:</label><br/>\n"
    "<input type=\"text\" name=\"" PARAM_LONGITUDE "\" size=12 maxlength=12 value=\"");
  if (! isnan(config->longitude))
    page.concat(strFormat("%.6f", config->longitude));
  page.concat("\" onblur=\"validateFloat(this,-180,180)\"><br/>\n"
    "<input type=\"submit\" class=\"roundbtn\" value=\"Store\">\n"
    "<input type=\"button\" class=\"roundbtn\" value=\"Reboot\" onclick=\"if(confirm('Are you sure?')) window.location.href='" URL_RESTART "'\">\n"
    "<input type=\"button\" class=\"roundbtn\" value=\"Reset!\" onclick=\"if(confirm('Are you sure?')) window.location.href='" URL_RESET "'\">\n"
    "</form>\n"
    HTML_FOOTER);
  www.send(200, TEXT_HTML, page);
}

static void webWiFi() {
  String list;
  int16_t n;

  WiFi.enableSTA(true);
  n = WiFi.scanNetworks();
  list = "{\"wifi\":[";
  if (n > 0) {
    for (auto i = 0; i < n; ++i) {
      if (i)
        list.concat(',');
      list.concat('"');
      list.concat(encodeString(WiFi.SSID(i).c_str()));
      list.concat('"');
    }
  }
  WiFi.scanDelete();
  WiFi.enableSTA(false);
  list.concat("]}");
  www.send(200, "application/json", list);
}

static void webStore() {
  bool success = true;

  for (uint8_t i = 0; i < www.args(); ++i) {
    const String &name = www.argName(i);
    const String &value = www.arg(i);

    if (! strcmp(name.c_str(), PARAM_WIFI_SSID)) {
      SET_STR_PARAM(config->wifi_ssid, value.c_str());
    } else if (! strcmp(name.c_str(), PARAM_WIFI_PSWD)) {
      SET_STR_PARAM(config->wifi_pswd, value.c_str());
    } else if (! strcmp(name.c_str(), PARAM_WIFI_TIMEOUT)) {
      config->wifi_timeout = max(15L, value.toInt());
    } else if (! strcmp(name.c_str(), PARAM_YW_KEY)) {
      SET_STR_PARAM(config->yw_key, value.c_str());
    } else if (! strcmp(name.c_str(), PARAM_LATITUDE)) {
      config->latitude = value.toFloat();
    } else if (! strcmp(name.c_str(), PARAM_LONGITUDE)) {
      config->longitude = value.toFloat();
    }
  }
  if (! config) {
    success = config.commit() == ESP_OK;
    if (success)
      Serial.println("NVS parameters updated successfully");
    else
      Serial.println("Error updating NVS parameters!");
  }
  if (success) {
    if ((! *config->wifi_ssid) || (! *config->yw_key) || isnan(config->latitude) || isnan(config->longitude)) {
      www.send(200, TEXT_HTML, HTML_HEADER("Store Configuration")
        "<meta http-equiv=\"refresh\" content=\"5;URL=" URL_ROOT "\">\n"
        HTML_STYLE("body{background-color:#eee;color:red;}\n")
        HTML_BODY
        "Configuration incomplete!\n"
        HTML_FOOTER);
    } else {
      www.send(200, TEXT_HTML, HTML_HEADER("Store Configuration")
        "<meta http-equiv=\"refresh\" content=\"5;URL=" URL_ROOT "\">\n"
        HTML_STYLE("body{background-color:#eee;}\n")
        HTML_BODY
        "Configuration stored successfully\n"
        HTML_FOOTER);
    }
  } else {
    www.send(500, TEXT_HTML, HTML_HEADER("Store Configuration")
      "<meta http-equiv=\"refresh\" content=\"5;URL=" URL_ROOT "\">\n"
      HTML_STYLE("body{background-color:#eee;color:red;}\n")
      HTML_BODY
      "Store configuration fail!\n"
      HTML_FOOTER);
  }
}

static void webRestart() {
  www.sendHeader("Connection", "close");
  www.send(200, TEXT_HTML, HTML_HEADER("Restart")
    "<meta http-equiv=\"refresh\" content=\"5;URL=" URL_ROOT "\">\n"
    HTML_STYLE("body{background-color:#eee;}\n")
    HTML_BODY
    "Restarting...\n"
    HTML_FOOTER);
  delay(500);
  esp_restart();
}

static void webReset() {
  config.clear();
  if (config.commit() == ESP_OK) {
    www.send(200, TEXT_HTML, HTML_HEADER("Configuration Reset")
      "<meta http-equiv=\"refresh\" content=\"5;URL=" URL_ROOT "\">\n"
      HTML_STYLE("body{background-color:#eee;}\n")
      HTML_BODY
      "Configuration resets successfully\n"
      HTML_FOOTER);
  } else {
    www.send(500, TEXT_HTML, HTML_HEADER("Configuration Reset")
      "<meta http-equiv=\"refresh\" content=\"5;URL=" URL_ROOT "\">\n"
      HTML_STYLE("body{background-color:#eee;color:red;}\n")
      HTML_BODY
      "Configuration reset fail!\n"
      HTML_FOOTER);
  }
}

static void webOta() {
  if (www.method() == HTTP_GET) {
    www.send(200, TEXT_HTML, HTML_HEADER("Firmware Update")
      HTML_STYLE("body{background-color:#eee;}\n"
        "input[type=\"button\"],input[type=\"submit\"]{border:1px solid gray;border-radius:5px;padding:5px 10px;margin:10px 0;width:25%}\n")
      HTML_BODY
      "<form method=\"POST\" action=\"\" enctype=\"multipart/form-data\" onsubmit=\"if(document.getElementsByName('update')[0].files.length==0){alert('No firmware file selected!');return false;}\">\n"
      "<label for=\"update\">Select firmware file:</label><br/>\n"
      "<input type=\"file\" name=\"update\" accept=\".bin\"><br/>\n"
      "<input type=\"submit\" value=\"Update!\">\n"
      "<input type=\"button\" value=\"Back\" onclick=\"window.location.href='" URL_ROOT "'\">\n"
      "</form>\n"
      HTML_FOOTER);
  } else if (www.method() == HTTP_POST) {
    if (Update.hasError()) {
      www.send(200, TEXT_HTML, HTML_HEADER("Firmware Update")
        "<meta http-equiv=\"refresh\" content=\"5;URL=" URL_ROOT "\">\n"
        HTML_STYLE("body{background-color:#eee;color:red;}\n")
        HTML_BODY
        "Firmware update fail!\n"
        HTML_FOOTER);
    } else {
      www.sendHeader("Connection", "close");
      www.send(200, TEXT_HTML, HTML_HEADER("Firmware Update")
        "<meta http-equiv=\"refresh\" content=\"5;URL=" URL_ROOT "\">\n"
        HTML_STYLE("body{background-color:#eee;}\n")
        HTML_BODY
        "Firmware updated successfully. Restarting...\n"
        HTML_FOOTER);
      delay(500);
      esp_restart();
    }
  } else {
    www.send(405, TEXT_PLAIN, "Method not allowed!");
  }
}

static void webOtaing() {
  HTTPUpload &upload = www.upload();

  if (upload.status == UPLOAD_FILE_START) {
    uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;

    Serial.print("Update firmware from file \"");
    Serial.print(upload.filename);
    Serial.println('"');
    if (! Update.begin(maxSketchSpace)) { // start with max available size
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    Serial.print('.');
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) { // true to set the size to the current progress
      Serial.println();
    } else {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_ABORTED) {
    Update.abort();
    Serial.println();
    Serial.println("Update was aborted!");
  }
  yield();
}

static bool captivePortal() {
  WiFi.mode(WIFI_AP);
  Serial.printf("Creating AP \"%s\" with password \"%s\" ", CP_SSID, CP_PSWD);
  if (WiFi.softAP(CP_SSID, CP_PSWD, CP_CHANNEL)) {
    DNSServer dns;

    blinks_update(blinks, 0, BLINK_4HZ, LED_BRIGHT);
    Serial.println("OK");

    initDisplay();
    wifiQrCode(CP_SSID, CP_PSWD);
    display.display();

    dns.setErrorReplyCode(DNSReplyCode::NoError);
    dns.start(53, "*", WiFi.softAPIP());
    www.onNotFound(webNotFound);
    www.on(URL_ROOT, HTTP_GET, webRoot);
    www.on(URL_WIFI, HTTP_GET, webWiFi);
    www.on(URL_STORE, HTTP_POST, webStore);
    www.on(URL_RESTART, HTTP_GET, webRestart);
    www.on(URL_RESET, HTTP_GET, webReset);
    www.on(URL_OTA, HTTP_ANY, webOta, webOtaing);
    www.begin();
    while (true) {
      dns.processNextRequest();
      www.handleClient();
      delay(1);
    }
  } else
    Serial.println("FAIL!");
  return false;
}

void setup() {
#if CONFIG_XTAL_FREQ == 26
  Serial.begin(74880);
#else
  Serial.begin(115200);
#endif

  config.onClear([](config_t *cfg) {
#ifdef DEF_WIFI_SSID
    SET_STR_PARAM(cfg->wifi_ssid, DEF_WIFI_SSID);
#endif
#ifdef DEF_WIFI_PSWD
    SET_STR_PARAM(cfg->wifi_pswd, DEF_WIFI_PSWD);
#endif
#ifdef DEF_WIFI_TIMEOUT
    cfg->wifi_timeout = DEF_WIFI_TIMEOUT;
#else
    cfg->wifi_timeout = 30; // 30 sec.
#endif
#ifdef DEF_YW_KEY
    SET_STR_PARAM(cfg->yw_key, DEF_YW_KEY);
#endif
#ifdef DEF_LATITUDE
    cfg->latitude = DEF_LATITUDE;
#else
    cfg->latitude = NAN;
#endif
#ifdef DEF_LONGITUDE
    cfg->longitude = DEF_LONGITUDE;
#else
    cfg->longitude = NAN;
#endif
  });
  ESP_ERROR_CHECK(config.begin());

  WiFi.persistent(false);

  blinks = blinks_init();
  assert(blinks);
  ESP_ERROR_CHECK(blinks_add(blinks, (gpio_num_t)LED_PIN, LED_LEVEL, nullptr));

  bool cp = (! *config->wifi_ssid) || (! *config->yw_key) || isnan(config->latitude) || isnan(config->longitude);

  if ((! cp) && (esp_reset_reason() != ESP_RST_DEEPSLEEP)) {
    uint32_t start = millis();

    pinMode(BTN_PIN, BTN_LEVEL ? INPUT : INPUT_PULLUP);
    blinks_update(blinks, 0, BLINK_ON, LED_BRIGHT);
    while ((! (cp = digitalRead(BTN_PIN) == BTN_LEVEL)) && (millis() - start < 1000)) {
      delay(1);
    }
    blinks_update(blinks, 0, BLINK_OFF, LED_BRIGHT);
  }

  if (cp || ((! wifiConnect(config->wifi_ssid, *config->wifi_pswd ? config->wifi_pswd : nullptr, 1000UL * config->wifi_timeout)) && (esp_reset_reason() != ESP_RST_DEEPSLEEP))) {
    if (! captivePortal()) {
      Serial.flush();
      ESP.restart();
    }
  }

  if (WiFi.isConnected()) {
    if (getWeather())
      drawWeather();

    WiFi.disconnect();
  }
  WiFi.mode(WIFI_OFF);

  if (timestamp) { // getWeather() returns true
    if (timestamp / 3600 > 21) { // Late evening
      timestamp = 3600 * 24 - timestamp; // Seconds to midnight
      timestamp += 3600 * 8 + 300; // 8:05
    } else if (timestamp / 3600 < 8) { // Early morning
      timestamp = 3600 * 8 - timestamp + 300; // 8:05
    } else {
      timestamp %= 3600;
      timestamp = 3600 - timestamp + 300;
    }
    if (timestamp < 300) // 5 min.
      timestamp += 3600; // Skip nearest hour
  } else
    timestamp = 300; // 5 min.
  esp_sleep_enable_timer_wakeup(1000000ULL * timestamp);
  Serial.printf("Sleeping for %lu seconds...\n", timestamp);
  Serial.flush();
  esp_deep_sleep_disable_rom_logging();
  esp_deep_sleep_start();
}

void loop() {}
