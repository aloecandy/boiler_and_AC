
#include <ESP8266WiFi.h>
#include "DHT.h"
#include "IRremoteESP8266.h"

DHT dht(2, DHT11); 
IRsend irsend(14);
unsigned int data[8][43] = {
  { 4450,4400, 600,1650, 550,550, 550,550, 550,550, 600,500, 600,550, 550,550, 550,550, 550,550, 600,1650, 550,550, 550,550, 550,1650, 600,1650, 550,1650, 600,1600, 600,1650, 550,1700, 550,500, 600,550, 550 },
  { 4500,4400, 600,1600, 600,500, 650,500, 550,550, 600,500, 600,550, 550,500, 600,550, 550,550, 550,1700, 550,550, 550,550, 550,550, 600,1650, 550,550, 550,550, 550,1650, 600,1650, 550,550, 550,550, 600 },
  { 4500,4400, 550,1650, 550,550, 600,500, 600,500, 600,550, 550,550, 550,550, 600,500, 600,550, 550,1650, 550,550, 600,550, 550,1650, 550,550, 550,550, 600,550, 550,1650, 600,1600, 600,550, 550,550, 550 },
  { 4450,4400, 600,1600, 550,550, 600,550, 550,550, 550,550, 600,500, 600,500, 600,550, 550,550, 550,1650, 600,500, 600,550, 550,550, 550,550, 600,1600, 600,1650, 550,1700, 550,1650, 550,550, 550,550, 550 },
  { 4450,4400, 600,1600, 600,500, 600,550, 550,500, 650,500, 550,550, 550,550, 600,550, 550,550, 550,1650, 550,550, 600,550, 550,1650, 600,500, 600,500, 600,1650, 550,1650, 600,1650, 550,550, 550,550, 550 },
  { 4450,4400, 600,1600, 600,500, 600,500, 600,500, 650,500, 550,550, 550,550, 600,550, 550,550, 550,1650, 550,550, 600,550, 550,550, 550,550, 600,1600, 600,550, 550,1650, 600,1600, 600,550, 550,550, 550 },
  { 4450,4400, 650,1600, 550,550, 550,550, 550,550, 600,500, 600,550, 550,550, 550,550, 550,550, 600,1650, 550,550, 550,550, 550,600, 550,1650, 550,550, 550,1650, 600,550, 550,1650, 550,550, 550,550, 600 },
  { 4500,4400, 600,1600, 550,550, 600,500, 550,550, 600,550, 550,550, 550,550, 600,500, 600,550, 550,1650, 550,550, 600,500, 600,550, 550,550, 550,1650, 600,1600, 600,550, 600,1600, 600,550, 550,550, 550 }
};
//0 운전/정지,  1 온도+, 2 온도-, 3 냉방, 4 송풍, 5 바람세기, 6. 상하바람, 7. 터보
int kHz = 38;                    // default frequency
bool acState = false;
int acTemp = 18;
bool swing = false;
bool turbo = false;

bool lowFanSpeedA = false;
bool lowFanSpeedB = false;

float h, t, hic;

bool mode = false;  //false->cool true->fan

const char* ssid = "dlink-A3D8";  //your AP
const char* password = "vtpii82150"; //AP password

				   // Create an instance of the server
				   // specify the port to listen on as an argument
WiFiServer server(80);


void  sendir(int index) {
	for (int i = 0; i < 4; i++) {
		irsend.sendRaw(data[index], 43, kHz);
	}
	if (index == 0) {
		acState = !acState;
		acTemp = 18;
		turbo = false;
		lowFanSpeedA = false;
		lowFanSpeedB = false;
		mode = false;
	}
	else if (acState) {
		if (index == 1) {
			if (acTemp < 27) acTemp++;
		}
		else if (index == 2) {
			if (acTemp > 18) acTemp--;
		}

		else if (index == 3)  mode = false;
		else if (index == 4) mode = true;
		else if (index == 5) {
			if (mode) lowFanSpeedB = !(lowFanSpeedB);
			else lowFanSpeedA = !(lowFanSpeedA);
		}
		else if (index == 6) {
			swing = !(swing);
		}
		else if (index == 7) {
			turbo = !(turbo);
		}
	}


}
String html0 = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html><head><title>Hello1</title><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width, user-scalable=no\"></head><body><table border=\"1\" width=\"100%\" cellpadding=\"10px\"><tr align=\"center\"><td>Humidity</td><td>";
String html1 = "</td></tr><tr align=\"center\"><td>Temperature</td><td>";
String html2 = "</td></tr><tr align=\"center\"><td>HeatIndex</td><td>";
String html3 = "</td></tr><tr align=\"center\"><td>AirConditioner</td><td>";
String html9 = "</td></tr><tr align=\"center\"><td>Boiler</td><td>";
String html4 = "</td></tr></table></body></html>";

String html5 = "</td></tr><tr align=\"center\"><td>MODE</td><td>";
String html6 = "</td></tr><tr align=\"center\"><td>FanSpeed</td><td>";

String html7 = "</td></tr><tr align=\"center\"><td>SWING</td><td>";
String html8 = "</td></tr><tr align=\"center\"><td>TURBO</td><td>";
void setup() {
	WiFi.mode(WIFI_STA);
	Serial.begin(115200);
	delay(10);

  dht.begin();
  
	irsend.begin();
  pinMode(5,OUTPUT);
  pinMode(5,HIGH);
	pinMode(16, OUTPUT);
	digitalWrite(16, LOW);
	// Connect to WiFi network
	Serial.println();
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(ssid);

	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("");
	Serial.println("WiFi connected");

	// Start the server
	server.begin();
	Serial.println("Server started");

	// Print the IP address
	Serial.println(WiFi.localIP());
}
unsigned long checkTime = 0;

void dhtCheck(){
  float temp=dht.readHumidity();
  if(!(isnan(temp))) h=temp;
  temp=dht.readTemperature();
  if(!(isnan(temp))) t=temp;
  temp=dht.computeHeatIndex(t, h, false);
  if(!(isnan(temp))) hic=temp;
}
unsigned long boilertime;
void loop() {

	if (millis() - checkTime > 500 || millis() - checkTime < 0) {
		dhtCheck();
   checkTime=millis();
	}
 if(millis()-boilertime>43200000 || millis()-boilertime<0){
  pinMode(5,LOW);
  delay(3000);
  pinMode(5,HIGH);
 }
	if (WiFi.status() != WL_CONNECTED) {
		Serial.print("Wifi not connected");
		WiFi.begin(ssid, password);
		while (WiFi.status() != WL_CONNECTED) {
			delay(500);
			Serial.print(".");
		}
		server.begin();
	}
	else {
		// Check if a client has connected
		WiFiClient client = server.available();
		if (!client) {
			return;
		}
		// Wait until the client sends some data
		Serial.println("new client");
		unsigned int a = millis();
		while (!client.available()) {
			if (millis() - a > 10000 || millis() - a < 0) {
				Serial.print("client is not available");
				if (millis() - checkTime > 500 || millis() - checkTime < 0) {
					dhtCheck();
          checkTime=millis();
				}
				return;
			}
		}
		String req = client.readStringUntil('\r');
		Serial.println(req);
		client.flush();
		// Match the request

		if (req.indexOf("?AC=TOGGLE") != -1)
			sendir(0);
		else if (req.indexOf("?AC=ON") != -1) {
			if (!acState)  sendir(0);
		}
		else if (req.indexOf("?AC=OFF") != -1) {
			if (acState) sendir(0);
		}
		else if (req.indexOf("?AC=UP") != -1) {
			sendir(1);
		}
		else if (req.indexOf("?AC=DOWN") != -1) {
			sendir(2);
		}
		else if (req.indexOf("?BOILER=TOGGLE") != -1) {
			digitalWrite(16, (!digitalRead(16)));
		}

		else if (req.indexOf("?BOILER=OFF") != -1) {
			digitalWrite(16, LOW);
		}

		else if (req.indexOf("?BOILER=ON") != -1) {
			digitalWrite(16, HIGH);
		}



		client.flush();


		client.print(html0);
		client.print(h);
		client.print(html1);
		client.print(t);
		client.print(html2);
		client.print(hic);
		client.print(html3);
		if (acState) {
			client.print("ON,");
			client.print(acTemp);
			client.print(html5);
			if (mode) {
				client.print("Fan");
				client.print(html6);
				if (lowFanSpeedB) client.print("Low");
				else  client.print("High");
			}
			else {
				client.printf("Cooling");
				client.print(html6);
				if (lowFanSpeedA) client.print("Low");
				else  client.print("High");
			}
			client.print(html7);
			if (swing)  client.print("True");
			else client.print("False");
			client.print(html8);
			if (turbo)  client.print("True");
			else client.print("False");

		}
		else {
			client.print("OFF");
		}
		client.print(html9);
		if (digitalRead(16) == HIGH)  client.print("ON");
		else  client.print("OFF");
		client.print(html4);

		delay(1);
		client.stop(); //very important
		client.flush();
		Serial.println("Client disconnected");
		return;

	}
}

