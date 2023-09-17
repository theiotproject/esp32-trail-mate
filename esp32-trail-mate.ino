#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <string>
#include <LinkedList.h>  

#define SCAN_TIME 30     // seconds
#define MINIMUM_RSSI -100  // ignores devices with weaker signal

BLEScan* pBLEScan = nullptr;

char jsonBuffer[2048]; 

const u_int ledPin = 2; 

const unsigned long MAC_ENTRY_TIMEOUT = 5 * 60 * 1000; // 5 minutes in milliseconds

struct MacEntry {
  std::string macAddress;
  unsigned long timestamp;
};

LinkedList<MacEntry> macList;

void addUniqueMacAddress(const String& mac);
void printMacList();
void deleteOutdatedEntries();

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.getRSSI() > MINIMUM_RSSI )
    {
    addUniqueMacAddress(advertisedDevice.getAddress().toString().c_str());
    }
  }
};

void addUniqueMacAddress(const String& mac) {
  for (int i = 0; i < macList.size(); i++) {
    if (macList.get(i).macAddress == mac.c_str()) {
      return;
    }
  }
  
  // MAC address is not in the list, add it
  MacEntry newEntry;
  newEntry.macAddress = mac.c_str();
  newEntry.timestamp = millis();
  macList.add(newEntry);
  /*
      TODO - create a firebase entry 
  */
  Serial.println("Added MAC address: \n" + mac);
}

void deleteOutdatedEntries() {
  unsigned long currentTime = millis();

  for (int i = macList.size() - 1; i >= 0; i--) {
    if (currentTime - macList.get(i).timestamp >= MAC_ENTRY_TIMEOUT) {
      Serial.printf("Deleted outdated MAC address: %s \n", macList.get(i).macAddress.c_str());
      macList.remove(i);
    }
  }
}

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

void printMacList() {
  Serial.println("MAC Address List:");
  for (int i = 0; i < macList.size(); i++) {
    Serial.printf("MAC: %s, Timestamp: %ld\n", macList.get(i).macAddress.c_str(), macList.get(i).timestamp);
  }
}


void loop() {
  BLEScanResults foundDevices = pBLEScan->start(SCAN_TIME);
  pBLEScan->clearResults();
  printMacList();
  deleteOutdatedEntries();
}
