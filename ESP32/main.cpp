#include <WiFiConnect.h>
#include "AudioFileSourceHTTPStream.h"
#include "AudioFileSourceBuffer.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2SNoDAC.h"
#include <HTTPClient.h>

AudioGeneratorMP3 *mp3;
HTTPClient http;
WiFiConnect wc;

String TempString1, TempString2;
int ThisEpisode, LatestEpisode;

void setup()
{
  WiFi.mode(WIFI_STA);
  Serial2.begin(115200);
  // Try to connect to last Wi-Fi or wait until connected
  if (!wc.autoConnect())
  {
    wc.setAPName("");
    wc.startConfigurationPortal(AP_WAIT);
    ESP.restart();
  }
  Serial2.println("Connected");
  http.begin("https://www.uh.edu/engines/keywords.htm");
  http.GET();
  while (TempString1.indexOf("footer") == -1)
  {
    TempString1 = http.getStream().readStringUntil('\n');
    TempString1.toLowerCase();
    if (TempString1.indexOf("href") > -1)
      TempString2 = TempString1;
  }
  http.end();
  TempString2 = TempString2.substring(TempString2.indexOf("\"") + 4, TempString2.indexOf("."));
  LatestEpisode = TempString2.toInt();
  ThisEpisode = random(1, LatestEpisode);
}

void loop()
{
  delay(1000);
  http.begin("https://www.uh.edu/engines/epi" + String(ThisEpisode) + ".htm");
  http.GET();
  TempString1 = http.getString();
  http.end();
  TempString2 = TempString1.substring(TempString1.indexOf("<title>") + 7, TempString1.indexOf("</title>"));
  TempString2.replace("\n", " ");
  TempString2.trim();
  Serial2.println(("\n" + TempString2).c_str());
  TempString2 = TempString1.substring(TempString1.indexOf("description"), TempString1.indexOf("keywords"));
  TempString2 = TempString2.substring(TempString2.indexOf("=") + 1, TempString2.indexOf(">"));
  TempString2.replace("\n", " ");
  TempString2.trim();
  Serial2.println(("\n" + TempString2).c_str());
  TempString2 = "http://www.kuhf.org/programaudio/engines/eng" + String(ThisEpisode) + "_64k.mp3";
  mp3 = new AudioGeneratorMP3();
  mp3->begin(new AudioFileSourceBuffer(new AudioFileSourceHTTPStream(TempString2.c_str()), 4096), new AudioOutputI2SNoDAC());
  while (mp3->loop())
  {
    if (Serial2.available())
    {
      TempString2 = Serial2.readString();
      if (TempString2 == "<")
        ThisEpisode--;
      else if (TempString2 == "_")
        ThisEpisode = random(1, LatestEpisode);
      else if (TempString2 == ">")
        ThisEpisode++;
      else
        ThisEpisode = TempString2.toInt();
      if (ThisEpisode > 0)
      {
        Serial2.printf("Requesting %d ...", ThisEpisode);
        mp3->stop();
        mp3->~AudioGeneratorMP3();
      }
    }
  }
  if (TempString2.indexOf("http") == 0)
    ThisEpisode++;
}
