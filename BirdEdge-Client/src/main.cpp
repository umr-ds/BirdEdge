#include "Arduino.h"
#include "ArduinoJson.h"
#include "AsyncJson.h"
#include "driver/adc.h"
#include "driver/i2s.h"
#include "esp_adc_cal.h"
#include "mdns.h"
#include "soc/sens_reg.h"
#include <ESP32Ping.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <sys/socket.h>

#define PIN_BATTERY 35
#define PIN_LED 13

// variable for adc2 control register, see https://github.com/espressif/arduino-esp32/issues/440#issuecomment-1008165291
uint32_t adc_register;
uint32_t wifi_register;

#if __has_include("secrets.h")
#include "secrets.h"
#else
#define WIFI_SSID "SSID"
#define WIFI_PASS "PASS"
#endif

// ### I2S Config
#define CHUNKSIZE ((size_t)1024)
#define QUEUE_SIZE (32)
QueueHandle_t queue;

static const i2s_port_t i2s_num = I2S_NUM_0;

static const i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 44100,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_MSB,
    .intr_alloc_flags = 0,
    .dma_buf_count = 32, // dma_desc_num
    .dma_buf_len = 1024, // dma_frame_num
    .use_apll = false,
};

const i2s_pin_config_t pin_config = {
    .bck_io_num = 14,   // BCKL
    .ws_io_num = 15,    // LRCL
    .data_out_num = -1, // not used (only for speakmers)
    .data_in_num = 32   // DOUT
};

// ### WAV Header definitions
typedef struct {
    uint32_t chunk_id;
    uint32_t chunk_size;
    uint32_t format;
} chunk_riff_t;

typedef struct {
    uint32_t chunk_id;
    uint32_t chunk_size;
    uint16_t audio_format;
    uint16_t num_of_channels;
    uint32_t samplerate;
    uint32_t byterate;
    uint16_t block_align;
    uint16_t bits_per_sample;
} chunk_fmt_t;

typedef struct {
    uint32_t chunk_id;
    uint32_t chunk_size;
} chunk_data_t;

typedef struct {
    chunk_riff_t riff;
    chunk_fmt_t fmt;
    chunk_data_t data;
} wav_header_t;

// ### Streaming handling
AsyncWebServer server(80);

void streaming_wav_header_init(wav_header_t *w) {
    int len = 0xFFFFFFFF;

    // init chunk_riff_t
    w->riff.chunk_id = 0X46464952; // "RIFF"
    w->riff.chunk_size = len;
    w->riff.format = 0X45564157; // "WAVE"

    // init chunk_fmt_t
    w->fmt.chunk_id = 0X20746D66;
    w->data.chunk_size = len;
    w->fmt.audio_format = 1;
    w->fmt.num_of_channels = 1;
    w->fmt.chunk_size = 16;
    w->fmt.samplerate = i2s_config.sample_rate;
    w->fmt.byterate = i2s_config.sample_rate * w->fmt.num_of_channels * i2s_config.bits_per_sample / 8;
    w->fmt.block_align = w->fmt.num_of_channels * i2s_config.bits_per_sample / 8;
    w->fmt.bits_per_sample = i2s_config.bits_per_sample;

    // init chunk_data_t
    w->data.chunk_id = 0X61746164;
    w->fmt.chunk_size = 16;
}

#define STREAMING_TIMEOUT_MS (1000)
int last_read = 0;

bool streaming_in_use() {
    // if last_read was more than timeout ago
    return (millis() - last_read) < STREAMING_TIMEOUT_MS;
}

// Web server
size_t streaming_wav_get_chunk(uint8_t *buffer, size_t maxLen, size_t index) {
    // send header for index 0
    if (index == 0) {
        Serial.printf("Stream: starting on core %i\n", xPortGetCoreID());
        streaming_wav_header_init((wav_header_t *)buffer);
        return sizeof(wav_header_t);
    }

    size_t chunksize;

    // read data from i2s
    if (ESP_OK != i2s_read(i2s_num, buffer, maxLen, &chunksize, portMAX_DELAY)) {
        Serial.printf("Stream: i2s_read failed, ending stream.\n");
        return 0;
    };

    Serial.printf("Stream: sending %i (of max %i)\n", chunksize, maxLen);
    last_read = millis();
    return chunksize;
}

void delay_restart(void *parameters) {
    // delay for 3s
    vTaskDelay(pdMS_TO_TICKS(3000));

    esp_restart();
}

char hostname[32];

// ### Main Setup
void setup() {
    // Enable Serial Debug
    Serial.begin(115200);
    Serial.setDebugOutput(false);

    // Retrieve hostname
    uint64_t chipid = ESP.getEfuseMac();
    snprintf(hostname, 32, "birdclient-%04x", (uint16_t)(chipid >> 32));
    printf("Hostname: %s\n", hostname);

    // Initialize ADC
    uint16_t battery = analogRead(PIN_BATTERY);
    Serial.printf("Battery: %i\n", battery);
    adc_register = READ_PERI_REG(SENS_SAR_READ_CTRL2_REG);

    // Wi-Fi connection
    Serial.print("Wifi: connecting to SSID ");
    Serial.print(WIFI_SSID);
    WiFi.setHostname(hostname);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    int counter = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");

        // If WiFi connection fails, send ESP32 to sleep
        if (60 == counter++) {
            Serial.println("");
            Serial.println("WiFi: could not connect, sleeping...");
            esp_deep_sleep(10 * 1000 * 1000);
        }
    }
    Serial.println("");
    Serial.println("WiFi: connected");
    wifi_register = READ_PERI_REG(SENS_SAR_READ_CTRL2_REG);

    if (ESP_OK != i2s_driver_install(i2s_num, &i2s_config, 0, NULL)) {
        Serial.println("I2S: driver install failed, sleeping...");
        esp_deep_sleep(10 * 1000 * 1000);
    }
    REG_SET_BIT(I2S_TIMING_REG(i2s_num), BIT(9));
    REG_SET_BIT(I2S_CONF_REG(i2s_num), I2S_RX_MSB_SHIFT);
    if (ESP_OK != i2s_set_pin(i2s_num, &pin_config)) {
        Serial.println("I2S: set pin failed, sleeping...");
        esp_deep_sleep(10 * 1000 * 1000);
    }

    // Initialized mDNS announcement
    mdns_init();
    mdns_hostname_set(hostname);
    mdns_service_add(NULL, "_birdedge", "_tcp", 80, NULL, 0);

    // Enable LED
    pinMode(PIN_LED, OUTPUT);

    // Mount SPIFFS
    SPIFFS.begin();

    // Initalize webserver
    server.on("/stream.wav", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (streaming_in_use()) {
            AsyncWebServerResponse *response = request->beginResponse(
                409, "text/plain", "Microphone already in use. Stop other running stream and retry...");
            request->send(response);
            return;
        }

        AsyncWebServerResponse *response = request->beginChunkedResponse("audio/x-wav", streaming_wav_get_chunk);
        request->send(response);
    });
    server.on("/status.json", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncJsonResponse *response = new AsyncJsonResponse();
        JsonObject root = response->getRoot();

        // fill json
        root["hostname"] = hostname;
        root["ip"] = WiFi.localIP().toString();
        root["ping"] = Ping.averageTime();
        root["uptime"] = String(millis() / 1000);
        root["wifi_rssi"] = String(WiFi.RSSI());
        root["heap"] = ESP.getFreeHeap();
        root["streaming"] = streaming_in_use();

        // analogRead on ADC2 failes when using WiFi:
        // https://github.com/espressif/arduino-esp32/issues/4782

        // The workaround used here only works in older versions
        // https://github.com/espressif/arduino-esp32/issues/102#issuecomment-1008165811
        WRITE_PERI_REG(SENS_SAR_READ_CTRL2_REG, adc_register);
        SET_PERI_REG_MASK(SENS_SAR_READ_CTRL2_REG, SENS_SAR2_DATA_INV);
        // read two times and add up, to compensate for the voltage divider and to poor mans average
        root["battery"] =
            (analogRead(PIN_BATTERY) + analogRead(PIN_BATTERY) + analogRead(PIN_BATTERY) + analogRead(PIN_BATTERY) +
             analogRead(PIN_BATTERY) + analogRead(PIN_BATTERY) + analogRead(PIN_BATTERY) + analogRead(PIN_BATTERY)) /
            4 / 1000.0;
        WRITE_PERI_REG(SENS_SAR_READ_CTRL2_REG, wifi_register);

        size_t json_len = response->setLength();

        uint8_t buf[512];
        response->_fillBuffer(buf, sizeof(buf));
        buf[json_len] = 0;
        Serial.printf("Status: %s\n", buf);

        // finish response
        request->send(response);
    });
    server.on("/restart", HTTP_POST, [](AsyncWebServerRequest *request) {
        Serial.println("System: Restarting Bird@Edge Mic...");
        xTaskCreatePinnedToCore(delay_restart, "RESTART", 2000, NULL, tskIDLE_PRIORITY, NULL, 0);

        request->redirect("/restart.html");
    });

    // serve static files from www folder
    server.serveStatic("/", SPIFFS, "/www/").setDefaultFile("index.html");

    Serial.printf("Stream: ready at http://%s/stream.wav\n", WiFi.localIP().toString().c_str());
    server.begin();
}

// ### main loop unused, because httpd handles the connection
void loop() {
    while (Ping.ping(WiFi.gatewayIP(), 1)) {
        Serial.printf("Main: core %i\n", xPortGetCoreID());

        // LED off if stream is running (effectively implements blinking on streaming)
        digitalWrite(PIN_LED, !streaming_in_use());
        delay(500);

        // LED on if ESP is on
        digitalWrite(PIN_LED, 1);
        delay(500);
    }

    digitalWrite(PIN_LED, 0);
    Serial.printf("WiFi: connection lost, restarting...");
    esp_restart();
}
