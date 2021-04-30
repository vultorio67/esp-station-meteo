#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include "Adafruit_SI1145.h"
#include <ArduinoJson.h>
 
Adafruit_SI1145 uv = Adafruit_SI1145();

Adafruit_BME280 bme;

int rainCounter;

float temperature, humidity, pressure, altitude;
#define SEALEVELPRESSURE_HPA (1013.25) 

int interval=3000;
// Tracks the time since last event fired
unsigned long previousMillis=0;
 
const char* ssid = "Livebox-B9E6";
const char* password = "WMvhfodCQpZ7Gbfywx";

float rains;



boolean findGY1145;
boolean findBme;
 
void setup () {


 
  Serial.begin(115200);
  delay(100);

  WifiConnexion();

  HTTPClient http;

    http.useHTTP10(true);
  http.begin("http://192.168.1.60:8080/json.htm?username=<user>=&password=<password>=&type=devices&rid=11");
  http.GET();

  // Parse response
  DynamicJsonDocument doc(2048);
  deserializeJson(doc, http.getStream());

  String result = doc["result"];

  //result.remove(0);


    result.remove(0,1);


  deserializeJson(doc, result);
  JsonObject obj = doc.as<JsonObject>();

  String data = obj[String("Data")];
  data.remove(0,2);

  Serial.println("start !!!!!!!!!!!!!!!!!!!!!!");


  String dataS = data;
  float mac = dataS.toFloat();
  Serial.println(mac);
  rains = mac;

  //check si le bme est on
  if (! bme.begin(0x76))
  {
    Serial.println("Dindn't find bme.");
    http.begin("http://192.168.1.60:8080/json.htm?username=<user>=&password=<password>=&type=command&param=addlogmessage&message=::::::::::::::::::::Dindn't find bme.&level=1");
    findBme = false;
  }
  else
  {
    Serial.println("find Bme");
    findBme = true;
    http.begin("http://192.168.1.60:8080/json.htm?username=<user>=&password=<password>=&type=command&param=addlogmessage&message=::::::::::::::::::::find bme.&level=1");
  }

    //check si le capteur uv est co
    if (! uv.begin()) {
    Serial.println("Didn't find Si1145");
    http.begin("http://192.168.1.60:8080/json.htm?username=<user>=&password=<password>=&type=command&param=addlogmessage&message=::::::::::::::::::::Dindn't find SI1145&level=1");
      Serial.println(WiFi.localIP());

    findGY1145 = false;
  }
  else
  {
    Serial.println("find Si1145");
    http.begin("http://192.168.1.60:8080/json.htm?username=<user>=&password=<password>=&type=command&param=addlogmessage&message=::::::::::::::::::::find SI1145.&level=1");
    findGY1145 = true;
  }
 
} 
 
void loop() {

   getRain();

   long long currentMillis = millis();
 
   // How much time has passed, accounting for rollover with subtraction!
   if ((unsigned long)(currentMillis - previousMillis) >= interval) {

     Serial.println(rains);
      // It's time to do something! // Toggle the LED on Pin 13
        Serial.println("reading temperature, humidity ");
              getBmeData();
    
      HTTPClient http;
    
      String a = "http://192.168.1.60:8080/json.htm?username=<user>=&password=<password>=&type=command&param=udevice&idx=9&nvalue=0&svalue=";
      String b = ";";
      String c = ";HUM_STAT;";
      String d = ";";
    
    
      http.begin(a+temperature+b+humidity+c+pressure+d+pressure); 
      Serial.println(a+temperature+b+humidity+c+pressure+d+pressure);//Specify request destination
      int httpCode2 = http.GET();                                  //Send the request
    
      if (httpCode2 > 0) { //Check the returning code
  
          String payload = http.getString();   //Get the request response payload
          Serial.println(payload);  
          delay(5);    
        }
       else
       {
        Serial.println("error :: the esp is not connecte to the network or the domoticz station don't respond.");
        Serial.println("trying to reconnect.");
        WifiConnexion();
       }
    
      if (findGY1145 == true)
        {
          getUv();
        }
      
      previousMillis = currentMillis;
   }   
  }

void WifiConnexion()
{
    WiFi.begin(ssid, password);
 
    while (WiFi.status() != WL_CONNECTED) {
 
    delay(250);
    Serial.print("Connecting");
    delay(250);
    Serial.print(".");
    delay(250);
    Serial.print(".");
    delay(250);
    Serial.print(".");

 
  }

  HTTPClient http;

  Serial.println(WiFi.localIP());
  http.begin("http://192.168.1.60:8080/json.htm?username=<user>=&password=<password>=&type=command&param=addlogmessage&message=the esp of météo station is connect to the network&level=1");
}


void getBmeData() {
  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure() / 100.0F;
  altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
}

void getRain(){

  HTTPClient http;

  int rain = analogRead(A0);
  if ( rain == 1024 )
  {

    Serial.println("le capteur a bougé");
    http.begin("http://192.168.1.60:8080/json.htm?username=<user>=&password=<password>=&type=command&param=addlogmessage&message=the rain counter have move&level=1");
    rainCounter = rainCounter + 1;
    Serial.println(rainCounter);
    rains = rains+0.28;
    Serial.println(rains);

    //http://192.168.1.60:8080/json.htm?type=command&param=udevice&idx=11&nvalue=0&svalue=111;18
    String z = "http://192.168.1.60:8080/json.htm?username=<user>=&password=<password>=&type=command&param=udevice&idx=11&nvalue=0&svalue=0;";
    

      http.begin(z+rains);
      Serial.println(z+rains); 
      //Serial.println(a+temperature+b+humidity+c+pressure+d+pressure);//Specify request destination
      int httpCode3 = http.GET();                                  //Send the request
    
      if (httpCode3 > 0) { //Check the returning code
  
          String payload = http.getString();   //Get the request response payload
          Serial.println(payload);  
          delay(5);    
        }
       else
       {
        Serial.println("error :: the esp is not connecte to the network or the domoticz station don't respond.");
        Serial.println("trying to reconnect.");
        WifiConnexion();

       }

    delay(500);
  }

}



//pour l'instant inactif
void getUv()
{

  HTTPClient http;
  Serial.println("===================");
  Serial.print("Vis: "); Serial.println(uv.readVisible());
  Serial.print("IR: "); Serial.println(uv.readIR());

  float UVindex = uv.readUV();
  UVindex /= 100.0;  
  Serial.print("UV: ");  Serial.println(UVindex);
  String e = "http://192.168.1.60:8080/json.htm?username=<user>=&password=<password>=&type=command&param=udevice&idx=10&nvalue=0&svalue=";
  String f = ";TEMP";

  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
  
        //Declare an object of class HTTPClient
  
      ///json.htm?type=command&param=udevice&idx=3&nvalue=0&svalue=450;100;hot
  
      http.begin(e+UVindex+f);
      Serial.println(e+UVindex+f);
      int httpCode = http.GET();                                  //Send the request
  
        if (httpCode > 0) { //Check the returning code
    
          String payload = http.getString();   //Get the request response payload
          Serial.println(payload);  
          delay(5);    //Send a request every 30 seconds
        }
  
    }
}
