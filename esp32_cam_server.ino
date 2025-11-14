#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClient.h>

// Replace with your network credentials
const char* ssid = "Josh";
const char* password = "Jitimay$$"; // Assuming this is the correct password from previous context

// ESP32-CAM (AI-Thinker) PIN Map
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#define LED_GPIO_NUM      33 // Flash LED

WebServer server(80);

// Global variable to store the last captured frame
camera_fb_t *fb = NULL;

void handle_jpg_stream(void);
void handle_still(void);
void handle_capture(void);
void handle_root(void);

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // if PSRAM is available, we can use higher resolutions
  if(psramFound()){
    config.frame_size = FRAMESIZE_SVGA; // 800x600
    config.jpeg_quality = 10; // 0-63 lower number means higher quality
    config.fb_count = 2; // if more than 1, then PSRAM must be enabled
  } else {
    config.frame_size = FRAMESIZE_CIF; // 352x288
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped horizontally and vertically
  s->set_vflip(s, 1); // flip it back
  s->set_hmirror(s, 1); // flip it back

  // WiFi connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("Camera IP Address: ");
  Serial.println(WiFi.localIP());

  // Web server routes
  server.on("/", HTTP_GET, handle_root);
  server.on("/capture", HTTP_GET, handle_capture);
  server.on("/still.jpg", HTTP_GET, handle_still);
  server.on("/stream", HTTP_GET, handle_jpg_stream);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}

void handle_root() {
  server.send(200, "text/html", "<html><body><h1>ESP32-CAM Server</h1><p>Visit /stream for live view, /capture to take a photo, /still.jpg for the last photo.</p></body></html>");
}

void handle_capture() {
  if(fb){
    esp_camera_fb_return(fb); // Return previous frame buffer if exists
    fb = NULL;
  }
  fb = esp_camera_fb_get();
  if(!fb){
    Serial.println("Camera capture failed");
    server.send(500, "text/plain", "Camera capture failed");
    return;
  }
  Serial.printf("Picture taken! Size: %zu bytes\n", fb->len);
  server.send(200, "text/plain", "Picture captured. View at /still.jpg");
}

void handle_still() {
  if(!fb){
    server.send(404, "text/plain", "No picture taken yet. Visit /capture first.");
    return;
  }
  server.sendHeader("Content-Type", "image/jpeg");
  server.sendHeader("Content-Length", String(fb->len));
  server.sendContent_P((const char *)fb->buf, fb->len);
}

void handle_jpg_stream(void) {
  WiFiClient client = server.client();
  if (!client.connected()) return;

  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: multipart/x-mixed-replace; boundary=123456789000000000000987654321\r\n";
  response += "\r\n";
  server.sendContent(response);

  camera_fb_t * fb_stream = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  char part_buf[128]; // Increased buffer size and corrected type

  while(client.connected()){
    fb_stream = esp_camera_fb_get();
    if (!fb_stream) {
      Serial.println("Camera capture failed");
      res = ESP_FAIL;
    } else {
      if(fb_stream->format != PIXFORMAT_JPEG){
        bool jpeg_converted = frame2jpg(fb_stream, 80, &_jpg_buf, &_jpg_buf_len);
        esp_camera_fb_return(fb_stream);
        fb_stream = NULL;
        if(!jpeg_converted){
          Serial.println("JPEG compression failed");
          res = ESP_FAIL;
        }
      } else {
        _jpg_buf = fb_stream->buf;
        _jpg_buf_len = fb_stream->len;
      }
    }
    if(res == ESP_OK){
      int header_len = sprintf(part_buf, "--123456789000000000000987654321\r\nContent-Type: image/jpeg\r\nContent-Length: %zu\r\n\r\n", _jpg_buf_len);
      server.sendContent(part_buf, header_len); // Pass length
      server.sendContent_P((const char *)_jpg_buf, _jpg_buf_len);
      server.sendContent("\r\n");
    } else {
      res = ESP_OK;
    }
    if(fb_stream){
      esp_camera_fb_return(fb_stream);
      fb_stream = NULL;
      _jpg_buf = NULL;
    } else if (_jpg_buf){
      free(_jpg_buf);
      _jpg_buf = NULL;
    }
  }
}
