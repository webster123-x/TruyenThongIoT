#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// Danh sách các chân GPIO phổ biến của LED RGB tích hợp trên ESP32-S3
const int rgbPins[] = {48, 38, 21}; 
const int numPins = sizeof(rgbPins) / sizeof(rgbPins[0]);

#define SERVICE_UUID        "4fa86000-0523-43f0-ac8b-d00b3e0561e0"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// Biến quản lý trạng thái nháy
bool isBlinking = false;
unsigned long lastBlinkTime = 0;
uint8_t blinkR = 0, blinkG = 0, blinkB = 0;
bool blinkState = false;

// Hàm ghi màu ra tất cả các chân GPIO nghi vấn
void setAllRGB(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < numPins; i++) {
    neopixelWrite(rgbPins[i], r, g, b); 
  }
}

// Lớp callback xử lý dữ liệu gửi từ nRF Connect
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String value = pCharacteristic->getValue().c_str();

      if (value.length() > 0) {
        value.toLowerCase();
        value.trim();

        Serial.print("[nRF Connect]: ");
        Serial.println(value);

        // Mặc định dừng nháy khi nhận lệnh mới
        isBlinking = false;

        // --- CÁC MÀU TĨNH ---
        if (value == "red")          setAllRGB(255, 0, 0);
        else if (value == "green")   setAllRGB(0, 255, 0);
        else if (value == "blue")    setAllRGB(0, 0, 255);
        else if (value == "yellow")  setAllRGB(255, 255, 0);
        else if (value == "cyan")    setAllRGB(0, 255, 255);
        else if (value == "purple")  setAllRGB(128, 0, 128);
        else if (value == "white")   setAllRGB(255, 255, 255);
        else if (value == "orange")  setAllRGB(255, 165, 0);
        else if (value == "pink")    setAllRGB(255, 192, 203);
        else if (value == "off")     setAllRGB(0, 0, 0);

        // --- CÁC LỆNH NHÁY (BLINK) ---
        else if (value.startsWith("blink")) {
          isBlinking = true;
          if (value == "blink_red")         { blinkR = 255; blinkG = 0;   blinkB = 0; }
          else if (value == "blink_green")  { blinkR = 0;   blinkG = 255; blinkB = 0; }
          else if (value == "blink_blue")   { blinkR = 0;   blinkG = 0;   blinkB = 255; }
          else if (value == "blink_yellow") { blinkR = 255; blinkG = 255; blinkB = 0; }
          else if (value == "blink_purple") { blinkR = 128; blinkG = 0;   blinkB = 128; }
          else if (value == "blink_cyan")   { blinkR = 0;   blinkG = 255; blinkB = 255; }
          else                              { blinkR = 255; blinkG = 255; blinkB = 255; } // blink hoặc blink_white
        }
        else {
          Serial.println("-> Lenh khong hop le!");
        }
      }
    }
};

void setup() {
  Serial.begin(115200);

  // Cấu hình Output cho các chân GPIO
  for (int i = 0; i < numPins; i++) {
    pinMode(rgbPins[i], OUTPUT);
  }

  // Khởi động nháy nhẹ để báo hiệu nguồn sẵn sàng
  setAllRGB(255, 0, 0); delay(150);
  setAllRGB(0, 255, 0); delay(150);
  setAllRGB(0, 0, 255); delay(150);
  setAllRGB(0, 0, 0);

  Serial.println("--- ESP32-S3 BLE Onboard LED RGB Control ---");

  // Khởi tạo BLE
  BLEDevice::init("ESP32S3_Onboard_LED");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();
  
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  BLEDevice::startAdvertising();

  Serial.println("-> San sang nhan lenh qua nRF Connect!");
}

void loop() {
  // Xử lý hiệu ứng nháy đèn (Non-blocking timer)
  if (isBlinking) {
    if (millis() - lastBlinkTime >= 300) { // Tốc độ nháy 300ms
      lastBlinkTime = millis();
      blinkState = !blinkState;
      if (blinkState) {
        setAllRGB(blinkR, blinkG, blinkB);
      } else {
        setAllRGB(0, 0, 0);
      }
    }
  }
  vTaskDelay(10 / portTICK_PERIOD_MS);
}