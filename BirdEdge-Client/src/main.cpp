#include "Arduino.h"
#include "driver/i2s.h"
#include "esp_http_server.h"
#include "mdns.h"
#include <WiFi.h>
#include <sys/socket.h>

#if __has_include("secrets.h")
#include "secrets.h"
#else
#define WIFI_SSID "SSID"
#define WIFI_PASS "PASS"
#endif

// ### I2S Config
static const i2s_port_t i2s_num = I2S_NUM_0;

static const i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 44100,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format =
        (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags = 0,
    .dma_buf_count = 24, // dma_desc_num
    .dma_buf_len = 64,   // dma_frame_num
    .use_apll = false};

// adjust to match i2s buffer size
#define SCRATCH_BUFSIZE (4096)

const i2s_pin_config_t pin_config = {
    .bck_io_num = 14,   // BCKL
    .ws_io_num = 15,    // LRCL
    .data_out_num = -1, // not used (only for speakers)
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

struct streaming_wav_t {
    wav_header_t hdr;
    int16_t *buf;
    int buf_size;
    int cnt;
};

// ### Streaming handling
httpd_handle_t audio_stream_httpd = NULL;

void streaming_wav_destroy(struct streaming_wav_t *wav) { free(wav->buf); }

int streaming_wav_factor(struct streaming_wav_t *wav) {
    return (wav->hdr.fmt.num_of_channels * wav->hdr.fmt.bits_per_sample / 8);
}

void streaming_wav_header(struct streaming_wav_t *wav) {
    wav_header_t *w = (wav_header_t *)&(wav->hdr);

    int len = 0xFFFFFFFF;

    w->riff.chunk_id = 0X46464952; // "RIFF"
    w->riff.format = 0X45564157;   // "WAVE"

    w->riff.chunk_size = len;

    w->fmt.chunk_id = 0X20746D66;
    w->fmt.audio_format = 1;
    w->fmt.bits_per_sample = i2s_config.bits_per_sample;
    w->fmt.num_of_channels = 1;
    w->fmt.block_align =
        w->fmt.num_of_channels * i2s_config.bits_per_sample / 8;
    w->fmt.byterate = i2s_config.sample_rate * w->fmt.num_of_channels *
                      i2s_config.bits_per_sample / 8;
    w->fmt.chunk_size = 16;
    w->fmt.samplerate = i2s_config.sample_rate;

    w->data.chunk_id = 0X61746164;
    w->data.chunk_size = len;
}

void streaming_wav_init(struct streaming_wav_t *wav, int buffer_size) {
    wav->cnt = 0;

    streaming_wav_header(wav);

    wav->buf = (int16_t *)malloc(buffer_size);
    wav->buf_size = buffer_size / streaming_wav_factor(wav);
}

static esp_err_t audio_stream_handler(httpd_req_t *req) {
    int sockfd = httpd_req_to_sockfd(req);
    char ipstr[INET6_ADDRSTRLEN];
    struct sockaddr_in6 addr;
    socklen_t addr_size = sizeof(addr);

    // extract remote IP address
    getpeername(sockfd, (struct sockaddr *)&addr, &addr_size);
    inet_ntop(AF_INET, &addr.sin6_addr.un.u32_addr[3], ipstr, sizeof(ipstr));
    Serial.printf("%s, starting audio stream\n", ipstr);

    // set response header
    httpd_resp_set_type(req, "audio/x-wav");

    // sent wave header
    streaming_wav_t wav;
    streaming_wav_init(&wav, SCRATCH_BUFSIZE);
    httpd_resp_send_chunk(req, (const char *)&(wav.hdr), sizeof(wav.hdr));

    // define buffer for audio data
    size_t chunksize = SCRATCH_BUFSIZE;
    esp_err_t err = ESP_OK;

    while (ESP_OK == err) {
        if (ESP_OK != i2s_read(i2s_num, wav.buf, SCRATCH_BUFSIZE, &chunksize,
                               portMAX_DELAY)) {
            Serial.printf("%s, i2s_read failed", ipstr);
            err = ESP_ERR_NOT_FOUND;
        };

        if (ESP_OK !=
            httpd_resp_send_chunk(req, (const char *)wav.buf, chunksize)) {
            Serial.printf("%s, failed to send chunk\n", ipstr);
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                                "Failed to send chunk");
            err = ESP_ERR_INVALID_STATE;
        }
    }

    streaming_wav_destroy(&wav);
    httpd_resp_send_chunk(req, NULL, 0);

    return err;
}

// ### Main Setup

void setup() {
    // Enable Serial Debug
    Serial.begin(115200);
    Serial.setDebugOutput(false);

    // Retrieve hostname
    char hostname[32];
    uint64_t chipid = ESP.getEfuseMac();
    snprintf(hostname, 32, "birdclient-%04x", (uint16_t)(chipid >> 32));
    printf("Hostname: %s\n", hostname);

    // Wi-Fi connection
    Serial.print("Wifi: connecting");
    WiFi.setHostname(hostname);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    int counter = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");

        // If WiFi connection fails, send ESP32 to sleep
        if (10 == counter++) {
            Serial.println("");
            Serial.println("WiFi: could not connect, sleeping...");
            esp_deep_sleep(10 * 1000);
        }
    }
    Serial.println("");
    Serial.println("WiFi: connected");

    // install and start i2s driver
    pinMode(22, INPUT);
    if (ESP_OK != i2s_driver_install(i2s_num, &i2s_config, 0, NULL)) {
        Serial.println("I2S: driver install failed, sleeping...");
        esp_deep_sleep(10 * 1000);
    }
    REG_SET_BIT(I2S_TIMING_REG(i2s_num), BIT(9));
    REG_SET_BIT(I2S_CONF_REG(i2s_num), I2S_RX_MSB_SHIFT);
    if (ESP_OK != i2s_set_pin(i2s_num, &pin_config)) {
        Serial.println("I2S: set pin failed, sleeping...");
        esp_deep_sleep(10 * 1000);
    }

    Serial.printf("Stream: ready at http://%s/stream.wav\n",
                  WiFi.localIP().toString().c_str());

    // start streaming web server
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;

    httpd_uri_t audio_stream_uri = {.uri = "/stream.wav",
                                    .method = HTTP_GET,
                                    .handler = audio_stream_handler,
                                    .user_ctx = NULL};

    if (httpd_start(&audio_stream_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(audio_stream_httpd, &audio_stream_uri);
    }

    // Initialized mDNS announcement
    mdns_init();
    mdns_hostname_set(hostname);

    mdns_service_add(NULL, "_birdedge", "_tcp", 80, NULL, 0);
}

// ### main loop unused, because httpd handles the connection
void loop() {}
