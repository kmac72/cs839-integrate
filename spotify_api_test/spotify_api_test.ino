/*******************************************************************
    Prints song information from a Spotify search query to the
    serial monitor using an ES32 or ESP8266

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

    Search functionality by Jeremiah Ukwela
 *******************************************************************/

// ----------------------------
// Standard Libraries
// ----------------------------

#include <WiFi.h>
#include <WiFiClientSecure.h>

// ----------------------------
// Additional Libraries - each one of these will need to be installed.
// ----------------------------

#include <SpotifyArduino.h>
#include <SpotifyArduinoCert.h>
#include <ArduinoJson.h>

#include "arduino_secrets.h"

//------- Replace the following! ------

char ssid[] = WIFI_SSID;
char password[] = WIFI_PASS;

char clientId[] = CLIENT_ID;
char clientSecret[] = CLIENT_SECRET;

// Country code, including this is advisable
#define SPOTIFY_MARKET "US"

//------- ---------------------- ------

WiFiClientSecure client;
SpotifyArduino spotify(client, clientId, clientSecret, SPOTIFY_REFRESH_TOKEN);

unsigned long delayBetweenRequests = 60000; // Time between requests (1 minute)
unsigned long requestDueTime;               //time when request due

#define MAX_RESULTS 2

void setup()
{

  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Handle HTTPS Verification
  client.setCACert(spotify_server_cert);
  // ... or don't!
  //client.setInsecure();

  // If you want to enable some extra debugging
  // uncomment the "#define SPOTIFY_DEBUG" in SpotifyArduino.h

  spotify.searchDetailsBufferSize = 5000; //can be adjusted, needs about 500 per result, defaults to 3000

  Serial.println("Refreshing Access Tokens");
  if (!spotify.refreshAccessToken())
  {
    Serial.println("Failed to get access tokens");
  }
}

bool playlistSearchCallback(PlaylistSearchResult result, int index, int numResults)
{

  Serial.println("--------- Playlist Details ---------");

  Serial.print("Playlist Index: ");
  Serial.println(index);

  Serial.print("Playlist Uri: ");
  Serial.println(result.playlistUri);

  Serial.print("Playlist Name: ");
  Serial.println(result.playlistName);

  Serial.print("Playlist Id: ");
  Serial.println(result.playlistId);

  Serial.print("Number of tracks: ");
  Serial.println(result.numTracks);

  Serial.print("Track href: ");
  Serial.println(result.tracksHref);

  Serial.print("Playlist Images (url): ");
  for (int i = 0; i < result.numImages; i++){
    Serial.println(result.albumImages[i].url);
  }

  Serial.println("-------------------------------");

  return true;
}


bool playlistItemsCallback(SearchResult result, int index, int numResults)
{

  Serial.println("--------- Song Details ---------");

  Serial.print("Song Index: ");
  Serial.println(index);

  Serial.print("Song ID: ");
  Serial.println(result.trackUri);

  Serial.print("Song Name: ");
  Serial.println(result.trackName);

  Serial.print("Song Artists: ");
  for (int i = 0; i < result.numArtists; i++){
    Serial.println(result.artists[i].artistName);
  }

  Serial.print("Song Album: ");
  Serial.println(result.albumName);

  Serial.print("Song Images (url): ");
  for (int i = 0; i < result.numImages; i++){
    Serial.println(result.albumImages[i].url);
  }

  Serial.println("-------------------------------");

  return true;
}

void loop()
{
  if (millis() > requestDueTime)
  {
    Serial.print("Free Heap: ");
    Serial.println(ESP.getFreeHeap());

    Serial.println("Making Search Request:");

    String query = "/?q=140%20BPM&type=playlist&market=US&offset=1";
    
    PlaylistSearchResult results[MAX_RESULTS];
    int status = spotify.searchForPlaylist(query, MAX_RESULTS, playlistSearchCallback, results);
    //spotify.searchForSong(query, limit, callback, array);
    
    if (status == 200)
    {
      Serial.println("Successfully got results: printing information");
      if (sizeof(results) > 0) 
      {
        PlaylistSearchResult thePlaylist = results[0];

        SearchResult trackList[MAX_RESULTS];
        query = "";
        int trackStatus = spotify.getPlaylistItems(query, thePlaylist.playlistId, MAX_RESULTS, playlistItemsCallback, trackList);
        if (trackStatus == 200)
        {
          Serial.println("Successfully got tracks: printing information");

          for (int i = 0; i < MAX_RESULTS; i++)
          {
            //Plays 30 seconds of each song from the search
            char body[100];
            sprintf(body, "{\"uris\" : [\"%s\"]}", trackList[i].trackUri);
            if (spotify.playAdvanced(body))
            {
                Serial.println("sent!");
            }
            delay(30*1e3);
          }
        }

        
      }

    }

    requestDueTime = millis() + delayBetweenRequests;
  }
}