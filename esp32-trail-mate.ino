#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <dummy.h>
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <string>
#include <LinkedList.h>  
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Firebase_ESP_Client.h>

#define SCAN_TIME 1     // seconds
#define SCAN_INTERVAL 2   //seconds
#define MINIMUM_RSSI -100  // ignores devices with weaker signal

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long dataMillis = 0;
int count = 0;

BLEScan* pBLEScan = nullptr;

const u_int ledPin = 2; 

const char *ssid = "ResetHack";
const char *password = "BBDays2023";
const char *ntpServer = "pool.ntp.org"; // NTP server to fetch time from
const char *api_key = "AIzaSyDs0YZEfpDS0jKQpMSxoEuCSgXRhYzarpM";
const char *firebase_project_id = "endurobb-db";
const char *user_emali = "jan.kowalski@poczta.pl";
const char *user_password = "haslo123";

const unsigned long MAC_ENTRY_TIMEOUT = 60 * 60 * 1000; // 15 minutes in milliseconds
int events_counter = 0;

struct MacEntry {
  std::string macAddress;
  unsigned long timestamp;
};

LinkedList<MacEntry> macList;

WiFiUDP ntpUDP; // Create an instance of WiFiUDP to handle NTP requests
NTPClient timeClient(ntpUDP, ntpServer); // Create an instance of NTPClient

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
  newEntry.timestamp = timeClient.getEpochTime();
  macList.add(newEntry);
  Serial.println("Event scanned");
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

  Serial.print("Connecting to Wi-Fi");
  unsigned long ms = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }

  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the project host and api key (required) */
  config.api_key = api_key;

  /* Assign the user sign in credentials */
  auth.user.email = user_emali;
  auth.user.password = user_password; /* Assign the callback function for the long running token generation task */

  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

  // Limit the size of response payload to be collected in FirebaseData
  fbdo.setResponseSize(2048);

  Firebase.begin(&config, &auth);

  // Initialize NTPClient
  timeClient.begin();
  timeClient.setTimeOffset(7200); // Set the time offset in seconds (0 for UTC)
  timeClient.forceUpdate();

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(), false);
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(SCAN_INTERVAL); // Set the scanning interval to the desired value
  pBLEScan->setWindow(SCAN_INTERVAL - 2); // Set the scanning window slightly less than the interval

  Serial.printf("BLE scan for %d s\n", SCAN_TIME);
}

void printMacList() {
  Serial.println("MAC Address List:");
  events_counter = macList.size();
  for (int i = 0; i < events_counter; i++) {
    // Format the timestamp to a human-readable date and time
    char formattedTime[30];
    time_t timestamp = macList.get(i).timestamp;
    struct tm timeinfo;
    gmtime_r(&timestamp, &timeinfo);
    strftime(formattedTime, sizeof(formattedTime), "%Y-%m-%d %H:%M:%S", &timeinfo);

    Serial.printf("MAC: %s, TMSTMP: %s\n", macList.get(i).macAddress.c_str(), formattedTime);
  }
}
void firestoreDataUpdate(){
  if(WiFi.status() == WL_CONNECTED && Firebase.ready()){
    String documentPath = "trails/MaM0rb1liG1C1cWIouKR"; // hard coded trail_id (stefanka)

    FirebaseJson content;

    content.set("fields/events/doubleValue", String(events_counter).c_str());

    if(Firebase.Firestore.patchDocument(&fbdo, firebase_project_id, "", documentPath.c_str(), content.raw(), "events")){
      Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
      return;
    }else{
      Serial.println(fbdo.errorReason());
    }

    if(Firebase.Firestore.createDocument(&fbdo, firebase_project_id, "", documentPath.c_str(), content.raw())){
      Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
      return;
    }else{
      Serial.println(fbdo.errorReason());
    }
  }
}

void loop() {
  BLEScanResults foundDevices = pBLEScan->start(SCAN_TIME);
  pBLEScan->clearResults();
  printMacList();
  deleteOutdatedEntries();
  firestoreDataUpdate();
}