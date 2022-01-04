//* Library ----------------------------------------------------------------------------------------------------------------------*//

#include <Arduino.h>	  // Arduino
#include "NewPing.h"	  // Sensor HC-SR04
#include <WiFi.h>		  // Wifi
#include "PubSubClient.h" // MQTT PubSub
#include "ArduinoJson.h"  // JSON

//* Connection Variable Definition -----------------------------------------------------------------------------------------------*//

#define ID 1

#if (ID == 1)
const int machineID = 1;
const char *hostname = "machineID: 1";
const char *ssid = "parkirAP_B1";
const char *password = "parkiresp";		   // Password local network
const char *brokerMQTT = "192.168.1.100";  // Alamat MQTT Broker pada local network (192.168.0.100)
const int portMQTT = 1883;				   // Port MQTT Broker
const char *outTopic = "parkirUKM/GSG/B1"; // Topic untuk mengirim data
const char *inTopic = "parkirUKM/cmd";	   // Topic from Node-RED
IPAddress local_IP(192, 168, 1, 110);	   // Static IP
IPAddress gateway(192, 168, 1, 1);		   // Gateway IP
#elif (ID == 2)
const int machineID = 2;
const char *hostname = "machineID: 2";
const char *ssid = "parkirAP_B2";
const char *password = "parkiresp";		   // Password local network
const char *brokerMQTT = "192.168.1.100";  // Alamat MQTT Broker pada local network (192.168.0.100)
const int portMQTT = 1883;				   // Port MQTT Broker
const char *outTopic = "parkirUKM/GSG/B2"; // Topic untuk mengirim data
const char *inTopic = "parkirUKM/cmd";	   // Topic from Node-RED
IPAddress local_IP(192, 168, 1, 111);	   // Static IP
IPAddress gateway(192, 168, 1, 1);		   // Gateway IP
#endif

//* Sensor Pin Def ---------------------------------------------------------------------------------------------------------------*//

const int numOfSensor = 3;					   // Jumlah sensor jarak yang dipakai
const int trigPin = 4;						   // Pin untuk Trigger
const int echoPin[numOfSensor] = {36, 34, 35}; // Pin untuk Echo
const int ledPin[numOfSensor] = {19, 18, 5};   // Pin untuk LED
int dummyState[numOfSensor] = {};			   // State untuk LED Before (Berisi 0 karena kondisional untuk state di Node-Red)
int ledState[numOfSensor] = {};				   // State untuk LED After (Berisi state sebenarnya karena sudah menerima data Node-Red)
unsigned int jarak[numOfSensor] = {};		   // Hasil pengukuran jarak
char jsonBuf[MQTT_MAX_PACKET_SIZE];			   // MQTT_MAX_PACKET_SIZE = 256 in PubSubClient.h
char incomingBuf[MQTT_MAX_PACKET_SIZE];		   // For callback
const int programDelay = 1000;				   // Delay untuk program (with millis)
unsigned long timeNow = 0;					   // Define waktu awal
const int maxJarak = 300;					   // Jarak maksiumum untuk sensor

//* Object & Client --------------------------------------------------------------------------------------------------------------*//

WiFiClient espClient;			// Create client
PubSubClient client(espClient); // Init client instance
NewPing sonar[numOfSensor] = {	// Sensor object array
	NewPing(trigPin, echoPin[0], maxJarak),
	NewPing(trigPin, echoPin[1], maxJarak),
	NewPing(trigPin, echoPin[2], maxJarak)};
DynamicJsonDocument doc(MQTT_MAX_PACKET_SIZE); // ESP32 has more Heap (Dyn) mem than Stack mem (Static) >> Use Dynamic

//* Function ---------------------------------------------------------------------------------------------------------------------*//

void reconnectMQTT()
{
	while (!client.connected())
	{
		Serial.print("Attempting MQTT connection. ");
		if (client.connect(hostname)) // Connected to MQTT Broker
		{
			Serial.println("Connected to MQTT");
			client.subscribe(inTopic); // Subscribe ulang ke topic
		}
		else // Jika gagal connect
		{
			Serial.print("Failed, rc=");
			Serial.println(client.state()); // client.state() see PubSubClient.h
			Serial.println("Try again in 2 seconds");
			delay(2000);
		}
	}
}

void connectMQTT()
{
	// Serial.printf("Free: %d\tMaxAlloc: %d\t PSFree: %d\n",ESP.getFreeHeap(),ESP.getMaxAllocHeap(),ESP.getFreePsram());
	Serial.println("Attempting MQTT connection. ");
	client.connect(hostname); // ESP will connect to MQTT broker with hostname
	{
		Serial.println("Connected to MQTT");
		// client.publish(outTopic, "Connected to MQTT Broker");
		client.subscribe(inTopic);

		if (!client.connected())
		{
			reconnectMQTT();
		}
	}
}

void callback(char *topic, byte *payload, unsigned int length)
{
	doc.clear();								 // Clear JSON document
	memset(incomingBuf, 0, sizeof(incomingBuf)); // Clear incomingBuf
	for (int i = 0; i < length; i++)
	{
		incomingBuf[i] = payload[i]; // Write streams of byte dari payload ke incomingBuf (masih JSON)
	}
	Serial.println("JSON from MQTT");
	Serial.println(incomingBuf); // Print incoming data

	// Print LED State
	if (doc[machineID - 1] == machineID)
	{
		for (int i = 0; i < numOfSensor; i++)
		{
			ledState[i] = !(doc[machineID - 1]["state"][i]); // Assign nilai state dari JSON ke ledState[i]
			digitalWrite(ledPin[i], ledState[i]);			 // Assign nilai state ke LED
			// Serial.println(ledState[i]);					 // Serial.println(ledState[i]);
			if (ledState[i] == 1)
			{
				Serial.println("Hijau/Kosong");
			}
			else
			{
				Serial.println("Merah/Ada Mobil");
			}
		}
	}

	Serial.println();
	delay(10);
}

void initWiFi()
{
	WiFi.mode(WIFI_STA); // WIFI_STA untuk koneksi ke AP
	WiFi.config(local_IP, gateway, INADDR_NONE, INADDR_NONE);
	WiFi.setHostname(hostname); // Set hostname
	WiFi.begin(ssid, password); // Begin WiFi Connection
	Serial.print("Connecting to ");
	Serial.println(ssid);
	for (int count = 0; count < numOfSensor; count++)
	{
		digitalWrite(ledPin[count], LOW);
	}
	while (WiFi.status() != WL_CONNECTED)
	{
		if (millis() - timeNow > 500)
		{
			timeNow = millis();
			Serial.print('.');
			for (int count = 0; count < numOfSensor; count++)
			{
				digitalWrite(ledPin[count], ledState[count]);
				ledState[count] = !ledState[count];
			}
		}
	}
	Serial.println("WiFi connected");
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP()); // ESP32's WiFi IP Address
	Serial.print("RRSI: ");
	Serial.println(WiFi.RSSI());
	delay(200);
	connectMQTT(); // MQTT Connection Initialization
}

void setup()
{
	Serial.begin(115200);	  // Starts serial communication
	pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
	for (int i = 0; i < numOfSensor; i++)
	{
		pinMode(echoPin[i], INPUT);	   // Sets the echoPin as an Input
		pinMode(ledPin[i], OUTPUT);	   // Sets the ledPin as an Output
		digitalWrite(ledPin[i], HIGH); // Give LED initial value
	}

	//* WiFi Connection Initialization
	Serial.println();
	Serial.println("Start WiFi Setup");
	delay(200);
	WiFi.disconnect();
	initWiFi();
	delay(10);
}

void loop()
{
	if (millis() - timeNow > programDelay)
	{
		timeNow = millis();
		if (WiFi.status() != WL_CONNECTED)
		{
			Serial.print("WiFi Disconnected. Reconnecting ");
			Serial.println(ssid);
			WiFi.disconnect();
			WiFi.reconnect();
		}
		if (!client.connected())
		{
			reconnectMQTT();
		}
		doc.clear(); // Clear JSON document

		//* Get Distance
		for (uint8_t i = 0; i < numOfSensor; i++)
		{
			delay(200);					   // Wait 100ms between pings
			jarak[i] = sonar[i].ping_cm(); // get distance
		}

		/*
		Target JSON
			{
			"machineID": 1,
			"jarak": [
				200,
				150,
				100
			],
			"rssi":xx
			}
		*/

		JsonObject obj = doc.to<JsonObject>();				 // Create JSON object
		obj["machineID"] = machineID;						 // Create JSON keys
		JsonArray jarakArr = obj.createNestedArray("jarak"); // Create JSON Array
		// JsonArray stateArr = obj.createNestedArray("state"); // Create JSON Array
		for (int i = 0; i < numOfSensor; i++)
		{
			jarakArr.add(jarak[i]); // Add values to JSON Array
									// stateArr.add(dummyState[i]); // Add values to JSON Array
		}
		int rssi = WiFi.RSSI();
		obj["rssi"] = rssi;
		// obj["heartbeat"] = hb; // Create JSON keys

		serializeJson(obj, jsonBuf, sizeof(jsonBuf)); // Parse JSON
		Serial.println("JSON to MQTT");
		serializeJson(obj, Serial); //? Print JSON to Serial

		client.publish(outTopic, jsonBuf); // Publish JSON to MQTT

		Serial.println();
		client.loop(); // Have to be called atleast every loop
	}
}
