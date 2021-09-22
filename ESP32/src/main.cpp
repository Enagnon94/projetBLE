#include <Arduino.h>

// For the screen
#include <TFT_eSPI.h>

// For bluetooth
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

int scanTime = 1.5; //In seconds
BLEScan *pBLEScan;

TFT_eSPI tft = TFT_eSPI(); // Invoke library, pins defined in User_Setup.h

// Result of ble mi temp
class BLEResult
{
public:
  double temperature = -200.0f;
  double humidity = -1.0f;
  int16_t battery_level = -1;
};

BLEResult result;

void displayResult()
{
  tft.fillScreen(TFT_BLACK);

  if (result.temperature > -200.0f)
  {
    Serial.printf("temperature: %.2f", result.temperature);
    Serial.println();    
    
    tft.setCursor(80, 80);
    tft.println(result.temperature);
  }
  if (result.humidity > -1.0f)
  {
    Serial.printf("humidity: %.2f", result.humidity);
    Serial.println();

    tft.setCursor(80, 100);
    tft.println(result.humidity);
  }
  if (result.battery_level > -1)
  {
    Serial.printf("battery_level: %d", result.battery_level);
    Serial.println();

    tft.setCursor(80, 120);
    tft.println(result.battery_level);
  }
}

// Callback when find device ble
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    BLEAddress address = advertisedDevice.getAddress();

    // Filter by mac address of mi temp
    if (address.toString() == "58:2d:34:3b:7d:3c")
    {

      uint8_t *payloadRaw = advertisedDevice.getPayload();
      size_t payloadLength = advertisedDevice.getPayloadLength();

      Serial.println();
      Serial.println("################################");
      Serial.print("Raw: ");

      // For each data of ble advertise
      for (int i = 0; i < payloadLength; i++)
      {
        // Show the data
        Serial.printf("%02X ", payloadRaw[i]);

        // Need min 3 char to start to check
        if (i > 3)
        {
          uint8_t raw = payloadRaw[i - 3];     // type
          uint8_t check = payloadRaw[i - 2];   // must always be 0x10
          int data_length = payloadRaw[i - 1]; // length of data

          if (check == 0x10)
          {
            // temperature, 2 bytes, 16-bit signed integer (LE), 0.1 °C
            if ((raw == 0x04) && (data_length == 2) && (i + data_length < payloadLength))
            {
              const int16_t temperature = uint16_t(payloadRaw[i + 0]) | (uint16_t(payloadRaw[i + 1]) << 8);
              result.temperature = temperature / 10.0f;
            }
            // humidity, 2 bytes, 16-bit signed integer (LE), 0.1 %
            else if ((raw == 0x06) && (data_length == 2) && (i + data_length < payloadLength))
            {
              const int16_t humidity = uint16_t(payloadRaw[i + 0]) | (uint16_t(payloadRaw[i + 1]) << 8);
              result.humidity = humidity / 10.0f;
            }
            // battery, 1 byte, 8-bit unsigned integer, 1 %
            else if ((raw == 0x0A) && (data_length == 1) && (i + data_length < payloadLength))
            {
              result.battery_level = payloadRaw[i + 0];
            }
            // temperature + humidity, 4 bytes, 16-bit signed integer (LE) each, 0.1 °C, 0.1 %
            else if ((raw == 0x0D) && (data_length == 4) && (i + data_length < payloadLength))
            {
              const int16_t temperature = uint16_t(payloadRaw[i + 0]) | (uint16_t(payloadRaw[i + 1]) << 8);
              const int16_t humidity = uint16_t(payloadRaw[i + 2]) | (uint16_t(payloadRaw[i + 3]) << 8);
              result.temperature = temperature / 10.0f;
              result.humidity = humidity / 10.0f;
            }
          }
        }
      }

      Serial.println();
      displayResult();

      Serial.println("################################");
    }
  }
};

void setup()
{
  // Monitor speed
  Serial.begin(115200);

  // Initialise BLE scan
  Serial.println("Scanning...");
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99); // less or equal setInterval value

  // Initialise screen
  tft.begin();
  tft.setRotation(1);
  tft.setTextSize(2);
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(80, 80);
  tft.println("hello");
}

void loop()
{
  // put your main code here, to run repeatedly:
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);

  pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory

}