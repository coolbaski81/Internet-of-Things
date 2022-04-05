#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* WiFi Access Point *********************************/

#define WIFI_SSID "your wifi usr name"
#define WIFI_PASS "your wifi pwd"

/************************* Adafruit.io Setup (entering the server URL, port, username, and key) ****************************/

#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883
#define AIO_USERNAME ""
#define AIO_KEY ""

int led = D0;

/************ Setup the MQTT client ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

Adafruit_MQTT_Subscribe onoff = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/ONOFF");
Adafruit_MQTT_Publish LightsStatus = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/LightsStatus");


void setup()
{
  Serial.begin(9600);
  pinMode(led, OUTPUT);


  //Connect to WiFi
  Serial.print("\n\nConnecting Wifi.");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  digitalWrite(led, LOW);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(50);
  }

  Serial.println("OK!");

  //Subscribe to the onoff topic
  mqtt.subscribe(&onoff);


}

void loop()
{
  //Connect/Reconnect to MQTT
  MQTT_connect();

  //Read from our subscription queue until we run out, or
  //wait up to 5 seconds for subscription to update
  Adafruit_MQTT_Subscribe * subscription;
  while ((subscription = mqtt.readSubscription(5000)))
  {
    //If we're in here, a subscription updated...
    if (subscription == &onoff)
    {
      //Print the new value to the serial monitor
      Serial.print("onoff: ");
      Serial.println((char*) onoff.lastread);

      //If the new value is  "ON", turn the light on.
      //Otherwise, turn it off.
      if (!strcmp((char*) onoff.lastread, "OFF"))
      {
        //active low logic
        digitalWrite(led, HIGH);
        LightsStatus.publish("ON");
      }
      else if (!strcmp((char*) onoff.lastread, "ON"))
      {
        digitalWrite(led, LOW);
        LightsStatus.publish("OFF");

      }
      else
      {
        LightsStatus.publish("ERROR");
      }
    }
    else
    {
      //LightsStatus.publish("ERROR");
    }
  }
  //  if (!mqtt.ping())
  //  {
  //    mqtt.disconnect();
  //  }
}


void MQTT_connect()
{

  //  // Stop if already connected
  if (mqtt.connected() && mqtt.ping())
  {
    //    mqtt.disconnect();
    return;
  }

  int8_t ret;

  mqtt.disconnect();

  Serial.print("Connecting to MQTT... ");
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) // connect will return 0 for connected
  {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0)
    {
      ESP.reset();
    }
  }
  Serial.println("MQTT Connected!");
}
