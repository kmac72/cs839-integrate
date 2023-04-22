
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <time.h>
#include "BLEDevice.h"
#include "BluetoothSerial.h"
#include <HTTPClient.h>
#include "arduino_secrets.h"
#include "device.h"
#include <SpotifyArduino.h>
#include <SpotifyArduinoCert.h>
#include <ArduinoJson.h>


// 839 Integrate
const char* ssid = SECRET_SSID;
const char* password = SECRET_PASSWORD;

char clientId[] = CLIENT_ID;
char clientSecret[] = CLIENT_SECRET;

static boolean cadence_connected = false;

static BLERemoteCharacteristic* sensorCharacteristic;
static BLEAdvertisedDevice* device;
static BLEClient* client;
static BLEScan* scanner;

static int cadence = 0;
static unsigned long runtime = 0;
static unsigned long last_millis = 0;

static int prevCumulativeCrankRev = 0;
static int prevCrankTime = 0;
static double rpm = 0;
static double prevRPM = 0;
static int prevCrankStaleness = 0;
static int stalenessLimit = 4;
static int scanCount = 0;

#define debug 0

static int cadenceAverageNumerator = 75;
static int cadenceAverageDenominator = 1;
static int curCadenceTimeValue = 0;
static int curCadenceAverage = 0;

static int timestampPlayingSongDone = 0;
static int timestampToFetchNextSong = 0;

static String currentPlayingSongTrackId = "";
static String nextSongToPlayTrackId = "";
static int nextSongToPlayDuration_ms = 0;

#define debug 0
#define maxCadence 120


// Country code, including this is advisable
#define SPOTIFY_MARKET "US"

// Spotify
WiFiClientSecure wifi_client;
HTTPClient http_client;
SpotifyArduino spotify(wifi_client, clientId, clientSecret, SPOTIFY_REFRESH_TOKEN);

unsigned long prev_timestamp;
unsigned int loop_delay = 10000;  // 10 secs

static bool is_bit_set(unsigned value, unsigned bitindex)
{
    return (value & (1 << bitindex)) != 0;
}

// Called when device sends update notification
static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* data, size_t length, bool isNotify) {


  bool hasWheel = is_bit_set(data[0], 0);
  bool hasCrank = is_bit_set(data[0], 1);

  int crankRevIndex = 1;
  int crankTimeIndex = 3;
  if(hasWheel)
  {
    crankRevIndex = 7;
    crankTimeIndex = 9;
  }
  
  int cumulativeCrankRev = int((data[crankRevIndex + 1] << 8) + data[crankRevIndex]);
  int lastCrankTime = int((data[crankTimeIndex + 1] << 8) + data[crankTimeIndex]);

  // if(debug)    
  // {
  //   Serial.println("Notify callback for characteristic");
  //   Serial.print("cumulativeCrankRev: ");
  //   Serial.println(cumulativeCrankRev);
  //   Serial.print("lastCrankTime: ");
  //   Serial.println(lastCrankTime);
  // }
    
  int deltaRotations = cumulativeCrankRev - prevCumulativeCrankRev;
  if (deltaRotations < 0) 
  { 
    deltaRotations += 65535; 
  }

  int timeDelta = lastCrankTime - prevCrankTime;
  if (timeDelta < 0) 
  { 
    timeDelta += 65535; 
  }

  // if(debug)    
  // {
  //   Serial.print("deltaRotations: ");
  //   Serial.println(deltaRotations);
  //   Serial.print("timeDelta: ");
  //   Serial.println(timeDelta);
  // }
  
  // In Case Cad Drops, we use PrevRPM 
  // to substitute (up to 4 seconds before reporting 0)
  if (timeDelta != 0)
  {
      prevCrankStaleness = 0;
      double timeMins = ((double)timeDelta) / 1024.0 / 60.0;
      rpm = ((double)deltaRotations) / timeMins;
      prevRPM = rpm;
      
    if(debug)    
    {
      // Serial.print("timeMins: ");
      // Serial.println(timeMins);
      // Serial.print("timeDelta != 0: rpm - ");
      Serial.println(rpm);
    }
  }
  else if (timeDelta == 0 && prevCrankStaleness < stalenessLimit)
  {
      rpm = prevRPM;
      prevCrankStaleness += 1;
      if(debug)    
      {
        Serial.print("timeDelta == 0 and not stale yet, rpm -");
        Serial.println(rpm);
      }
  }
  else if (prevCrankStaleness >= stalenessLimit)
  {
      rpm = 0.0;
      if(debug)    
      {
        Serial.print("stale");
        Serial.println(rpm);
      }      
  }

  prevCumulativeCrankRev = cumulativeCrankRev;
  prevCrankTime = lastCrankTime;
  
  if(debug)    
  {
    Serial.print("prevCumulativeCrankRev: ");
    Serial.println(prevCumulativeCrankRev);
    Serial.print("prevCrankTime: ");
    Serial.println(prevCrankTime);
  }
  
  cadence = (int)rpm;
  //Test
  //cadence = cadence + 2;

  curCadenceTimeValue = cadence * timeDelta;
  cadenceAverageNumerator = cadenceAverageNumerator + curCadenceTimeValue;
  cadenceAverageDenominator = cadenceAverageDenominator + timeDelta;
  if(cadenceAverageDenominator > 0){
    curCadenceAverage = cadenceAverageNumerator / cadenceAverageDenominator;
  }else{
    curCadenceAverage = 0;
  }
  
  // if(debug) {
  //   Serial.print("CALLBACK(");
  //   Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
  //   Serial.print(":");
  //   Serial.print(length);
  //   Serial.print("):");
  //   for(int x = 0; x < length; x++) {
  //     if(data[x] < 16) {
  //       Serial.print("0");
  //     }
  //     Serial.print(data[x], HEX);
  //   }
  //   Serial.println();
  // }

  
}
// Called on connect or disconnect
class ClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
    // digitalWrite(LED,HIGH);
    Serial.println("Connected!");
  }
  void onDisconnect(BLEClient* pclient) {
    cadence_connected = false;
    delete(client);
    client = nullptr;
    //digitalWrite(LED,LOW);
    Serial.println("Disconnected!");
  }
};

class AdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());
    if(advertisedDevice.getName().size() > 0) {
      BLEAdvertisedDevice * d = new BLEAdvertisedDevice;
      *d = advertisedDevice;
      addDevice(d);
    }
  }
};


bool connectToServer() {
  Serial.print("Connecting to ");
  Serial.println(device->getName().c_str());
    
  client = BLEDevice::createClient();
  client->setClientCallbacks(new ClientCallback());
  client->connect(device);

  // Sometimes it immediately disconnects - client will be null if so
  delay(200);
  if(client == nullptr) {
    return false;
  }
  
  BLERemoteService* remoteService = client->getService(serviceUUID);
  if (remoteService == nullptr) {
    Serial.print("Failed to find service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    client->disconnect();
    return false;
  }
  Serial.println("Found device.");

  // Look for the sensor
  sensorCharacteristic = remoteService->getCharacteristic(notifyUUID);
  if (sensorCharacteristic == nullptr) {
    Serial.print("Failed to find sensor characteristic UUID: ");
    Serial.println(notifyUUID.toString().c_str());
    client->disconnect();
    return false;
  }
  sensorCharacteristic->registerForNotify(notifyCallback);
  Serial.println("Enabled sensor notifications.");

  
  Serial.println("Activated status callbacks.");

  return true;
}

String fetchSongByBPM(int bpm) {

  bpm = 160;

  char url[100];
  sprintf(url, "http://18.119.17.68:3000/songs?tempo_gte=%d&tempo_lte=%d&danceability_gte=.8", bpm - 1, bpm + 1);
  Serial.print("Request: ");
  Serial.println(url);
  
  http_client.begin(url);
  int response = http_client.GET();
  String payload = http_client.getString();
  http_client.end();

  DynamicJsonDocument doc(1024);
  deserializeJson(doc, payload);
  JsonArray song_list = doc.as<JsonArray>();
  JsonObject song = song_list[0].as<JsonObject>();

  String song_id = song["id"].as<String>();
  nextSongToPlayDuration_ms = song["duration_ms"].as<int>();

  Serial.print("Song ID: ");
  Serial.println(song_id);
  Serial.printf("Duration: %d\n", nextSongToPlayDuration_ms);
  return song_id;
}

void playSong(String trackUri) {
    //char trackUri[] = "spotify:track:4uLU6hMCjMI75M1A2tKUQC";
    char body[200];
    sprintf(body, "{\"uris\" : [\"spotify:track:%s\"]}", trackUri.c_str());
    spotify.playAdvanced(body); //, deviceId);
}

void setup() {
  Serial.begin(115200);
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  prev_timestamp = millis();

  /*
  BLEDevice::init("");
  scanner = BLEDevice::getScan();
  scanner->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks());
  scanner->setInterval(1349);
  scanner->setWindow(449);
  scanner->setActiveScan(true);
  */



  // Spotify stuff
  wifi_client.setCACert(spotify_server_cert);

  Serial.println("Refreshing Access Tokens");
  if (!spotify.refreshAccessToken())
  {
    Serial.println("Failed to get access tokens");
  }
}

void loop() {
  /*
  // Start scan
  if(!cadence_connected){
    Serial.println("Start Scan!");
    scanner->start(11, false); // Scan for 10 seconds
    BLEDevice::getScan()->stop();

    device = selectDevice(); // Pick a device
    if(device != nullptr) {
      cadence_connected = connectToServer();
      if(!cadence_connected) {
        Serial.println("Failed to connect...");
        delay(3100);
        return;
      }
      
    } else {
      Serial.println("No device found...");
      
      delay(3100);
      return;
    }
  }
  */

  // If we get here it's connected
  unsigned long curr_timestamp = millis();

  if(curr_timestamp >= timestampPlayingSongDone) {
    int curAverageBpm = curCadenceAverage * 2;
    nextSongToPlayTrackId = fetchSongByBPM(curAverageBpm);
    playSong(nextSongToPlayTrackId);
    timestampPlayingSongDone = millis() + nextSongToPlayDuration_ms;
    cadenceAverageDenominator = 0;
    cadenceAverageNumerator = 0;
  }

  //if(curr_timestamp - prev_timestamp > loop_delay) {
    //int bpm = cadence * 2;
    //String trackUri = fetchSongByBPM(bpm);
    //playSong(trackUri);
    //prev_timestamp = millis();
  //}
}