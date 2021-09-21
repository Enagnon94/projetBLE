#include <Arduino.h>

// For the screen
#include <TFT_eSPI.h>

// For bluetooth
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

int scanTime = 5; //In seconds
BLEScan* pBLEScan;

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Scanning...");

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(false);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value

  tft.begin();
  tft.setRotation(1);
  tft.setTextSize(2);
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(100, 100);
  tft.println("hello");
}

void loop() {
  // put your main code here, to run repeatedly:
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  uint32_t nbFoundDevices = foundDevices.getCount();

  for (uint32_t i = 0; i < nbFoundDevices; i++) {
    BLEAdvertisedDevice foundDevice = foundDevices.getDevice(i);

    BLEAddress address = foundDevice.getAddress();

    if (address.toString() == "58:2d:34:3b:7d:3c") {

      uint8_t* payloadRaw = foundDevice.getPayload();
      size_t payloadLength = foundDevice.getPayloadLength();

      Serial.println();
      Serial.print("################################");
      Serial.println();
      Serial.printf("BLE:    Result %i [%s] payload: 0x", i, address.toString().c_str());
      for (int i = 0; i < payloadLength; i++) {
        Serial.print(payloadRaw[i], HEX);
        Serial.print(" ");
      }
      
      Serial.println();
      Serial.print("################################");
      Serial.println();
    }
    
  }

  Serial.print("Devices found: ");
  Serial.println(foundDevices.getCount());
  Serial.println("Scan done!");
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(100, 100);
  tft.println(foundDevices.getCount());

  delay(2000);

}