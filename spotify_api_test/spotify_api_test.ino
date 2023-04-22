/*******************************************************************
    Allow you to specify a track/artist/album to play.

    This is based on the request described here on the
    spotify documentation:
    https://developer.spotify.com/documentation/web-api/reference/player/start-a-users-playback/

    This example shows some ways of invoking the above, but read the documentation
    for full details.

    NOTE: You need to get a Refresh token to use this example
    Use the getRefreshToken example to get it.

    Compatible Boards:
	  - Any ESP8266 or ESP32 board

    Parts:
    ESP32 D1 Mini style Dev board* - http://s.click.aliexpress.com/e/C6ds4my

 *  * = Affiliate

    If you find what I do useful and would like to support me,
    please consider becoming a sponsor on Github
    https://github.com/sponsors/witnessmenow/


    Written by Brian Lough
    YouTube: https://www.youtube.com/brianlough
    Tindie: https://www.tindie.com/stores/brianlough/
    Twitter: https://twitter.com/witnessmenow
 *******************************************************************/

// ----------------------------
// Standard Libraries
// ----------------------------

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif

#include <WiFiClientSecure.h>

// ----------------------------
// Additional Libraries - each one of these will need to be installed.
// ----------------------------

#include <SpotifyArduino.h>
// Library for connecting to the Spotify API

// Install from Github
// https://github.com/witnessmenow/spotify-api-arduino

// including a "spotify_server_cert" variable
// header is included as part of the SpotifyArduino libary
#include <SpotifyArduinoCert.h>

#include <ArduinoJson.h>
#include "arduino_secrets.h"
// Library used for parsing Json from the API responses

// Search for "Arduino Json" in the Arduino Library manager
// https://github.com/bblanchon/ArduinoJson

//------- Replace the following! ------

char ssid[] = WIFI_SSID;         // your network SSID (name)
char password[] = WIFI_PASS; // your network password

char clientId[] = CLIENT_ID;     // Your client ID of your spotify APP
char clientSecret[] = CLIENT_SECRET; // Your client Secret of your spotify APP (Do Not share this!)

// Country code, including this is advisable
#define SPOTIFY_MARKET "US"

//------- ---------------------- ------

WiFiClientSecure client;
SpotifyArduino spotify(client, clientId, clientSecret, SPOTIFY_REFRESH_TOKEN);

void playSingleTrack()
{
    char sampleTrack[] = "spotify:track:4uLU6hMCjMI75M1A2tKUQC";
    //char *deviceId = "dd1edf2aa262ca8250b03a0a8401ee387e4f80c4";
    char body[200];
    sprintf(body, "{\"uris\" : [\"%s\"]}", sampleTrack);
    spotify.playAdvanced(body); //, deviceId);
}

void setup()
{

    Serial.begin(115200);

    // Attempt to connect to Wifi network:
    Serial.print("Connecting Wifi: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(500);
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    IPAddress ip = WiFi.localIP();
    Serial.println(ip);

    client.setCACert(spotify_server_cert);
    // ... or don't!
    //client.setInsecure();

    // If you want to enable some extra debugging
    // uncomment the "#define SPOTIFY_DEBUG" in SpotifyArduino.h

    Serial.println("Refreshing Access Tokens");
    if (!spotify.refreshAccessToken())
    {
        Serial.println("Failed to get access tokens");
        return;
    }

    delay(100);
    Serial.println("Playing Single Track");
    playSingleTrack();
}

// Example code is at end of setup
void loop()
{
}