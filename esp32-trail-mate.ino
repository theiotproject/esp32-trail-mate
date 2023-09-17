#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#define SCAN_TIME 30     // seconds

BLEScan* pBLEScan = nullptr;

char jsonBuffer[2048]; 

const u_int ledPin = 2; 

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.getRSSI() > -55 )
    {
      snprintf(jsonBuffer, sizeof(jsonBuffer),
             "{\"Address\":\"%s\",\"Rssi\":%d,\"Name\":\"%s\",\"Appearance\":%d,\"ServiceUUID\":\"%s\",\"TxPower\":%f}",
             advertisedDevice.getAddress().toString().c_str(),
             advertisedDevice.getRSSI(),
             advertisedDevice.getName().c_str(),
             advertisedDevice.getAppearance(),
             advertisedDevice.getServiceUUID().toString().c_str(),
             advertisedDevice.getTXPower());

    Serial.println("Advertised Device:");
    Serial.println(jsonBuffer);
    }
  }
};

void setup() {
  Serial.begin(115200);
  Serial.println("BLEDevice::init()");
  BLEDevice::init("");

  pinMode(ledPin, OUTPUT); // Set LED pin as output

  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(), false);
  pBLEScan->setActiveScan(true); //active scan uses more power, but gets results faster
  pBLEScan->setInterval(0x50);
  pBLEScan->setWindow(0x30);

  Serial.printf("Start BLE scan for %d seconds...\n", SCAN_TIME);
}

void loop() {

  // Perform BLE scan
  BLEScanResults foundDevices = pBLEScan->start(SCAN_TIME);
  pBLEScan->clearResults();

  // Blink the LED for 1 second when the scan finishes
  digitalWrite(ledPin, HIGH);
  delay(1000);
  digitalWrite(ledPin, LOW);
}
