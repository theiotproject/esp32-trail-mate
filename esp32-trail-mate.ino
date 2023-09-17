#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <string>
#include <LinkedList.h>  
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define SCAN_TIME 1     // seconds
#define SCAN_INTERVAL 2   //seconds
#define MINIMUM_RSSI -100  // ignores devices with weaker signal - increasing it to -55 will limit the scan range to around 1m-2m

BLEScan* pBLEScan = nullptr;

char jsonBuffer[2048]; 

const u_int ledPin = 2; 

const char *ssid = "";
const char *password = "";
const char *ntpServer = "pool.ntp.org"; // NTP server to fetch time from

const unsigned long MAC_ENTRY_TIMEOUT = 5 * 60 * 1000; // 5 minutes in milliseconds, if you want to filter unique MACs for a longer period, you can increase the first value
/*
  @brief Struct that stores unique MAC with the apperance timestamp.
*/
struct MacEntry {
  std::string macAddress;
  unsigned long timestamp;
};

LinkedList<MacEntry> macList;

WiFiUDP ntpUDP; // Create an instance of WiFiUDP to handle NTP requests
NTPClient timeClient(ntpUDP, ntpServer); // Create an instance of NTPClient

/*
  @brief A function that checks if the MAC address has appeared within the given period. If not, it stores it in a linked list.
*/
void addUniqueMacAddress(const String& mac);

/*
  @brief Prints a list of recent MAC addresses along with the time of their appearance.
*/
void printMacList();
/*
  @brief Function checks if the MAC address in the linked list got outdated. If so, it deletes it.
*/
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
  // Check if MAC already present
  for (int i = 0; i < macList.size(); i++) {
    if (macList.get(i).macAddress == mac.c_str()) {
      return;
    }
  }

  // MAC address is not in the list, add it
  MacEntry newEntry;
  newEntry.macAddress = mac.c_str();
  newEntry.timestamp = timeClient.getEpochTime();
  macList.add(newEntry);
  /*
      TODO - create a firebase entry 
  */
  Serial.println("Added MAC: \n" + mac);
}

void deleteOutdatedEntries() {
  unsigned long currentTime = timeClient.getEpochTime();

  for (int i = macList.size() - 1; i >= 0; i--) {
    if (currentTime - macList.get(i).timestamp >= MAC_ENTRY_TIMEOUT) {
      Serial.printf("Deleted outdated MAC address: %s \n", macList.get(i).macAddress.c_str());
      macList.remove(i);
    }
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting");
  }

  // Initialize NTPClient
  timeClient.begin();
  timeClient.setTimeOffset(7200); // Set the time offset in seconds (0 for UTC)
  timeClient.forceUpdate();

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(), false); //create callback
  pBLEScan->setActiveScan(true); // Set the active scan flag
  pBLEScan->setInterval(SCAN_INTERVAL); // Set the scanning interval to the desired value
  pBLEScan->setWindow(SCAN_INTERVAL - 2); // Set the scanning window slightly less than the interval

  Serial.printf("BLE scan for %d s\n", SCAN_TIME);
}

void printMacList() {
  Serial.println("MAC Address List:");
  for (int i = 0; i < macList.size(); i++) {
    // Format the timestamp to a human-readable date and time
    char formattedTime[30];
    time_t timestamp = macList.get(i).timestamp;
    struct tm timeinfo;
    gmtime_r(&timestamp, &timeinfo);
    strftime(formattedTime, sizeof(formattedTime), "%Y-%m-%d %H:%M:%S", &timeinfo);

    Serial.printf("MAC: %s, TMSTMP: %s\n", macList.get(i).macAddress.c_str(), formattedTime);
  }
}


void loop() {
  BLEScanResults foundDevices = pBLEScan->start(SCAN_TIME);
  pBLEScan->clearResults();
  printMacList();
  deleteOutdatedEntries();
}
