#include <Arduino.h>
#include "Digipot.hpp"
#include "DaisyChain.hpp"
#include "pins.hpp"
#include "Volumio.h"

#include <Servo.h>

#include <Preferences.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ESPmDNS.h>

#define WIFI

#define VOLUME 0
#define BASS 1
#define TREBLE 2
#define BALANCE 3
#define BFREQ 4
#define TFREQ 5

const char* ssid = "OnePlus 7";
const char* password = "HotspotvanWouter";

const char* volumio_ip = "192.168.178.110"; 
int volumio_port = 80;

// setting PWM properties
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;

const int output = 2; //on board LED

int volume = 0;
int bass = 0;
int treble = 0;
int balance = 0;
int bfreq = 0;
int tfreq = 0;

WiFiMulti wifiMulti;
Volumio volumio;
AsyncWebServer server(80);
Preferences preferences;


// Replaces placeholder with button section in your web page
String processor(const String& var){
  //Serial.println(var);
  if (var == "volume"){
    return String(volume);
  }
  return String();
}

DaisyChain csL(SPI_CSL);    
DaisyChain csR(SPI_CSR);
Digipot potPreAmp(127);
Digipot potUnsused(127);
Digipot potHighpassLevel(127);
Digipot potLowpassLevel(127);
Digipot potHighpassFrequency(127);
Digipot potLowpassFrequency(127);

Servo volumeLeft;
Servo volumeRight;

//forward declarations
void connectWifi();
void initialiseGET();
void setPotMeter(int min, int max, int value, Digipot& pot);
void setupPots();
void setupMux();

void setup() {
    Serial.begin(115200); //initiate serial coms with PC
    delay(10);


    SPI.begin(SPI_CLK, SPI_MISO, SPI_MOSI); //initiate SPI

    pinMode(MUTE_EIND, OUTPUT);
    digitalWrite(MUTE_EIND, LOW);

    setupMux();
    setupPots();

    preferences.begin("settings", false); //open in r/w mode;
    volume = preferences.getInt("volume", 50);  //retrieve the value for volume, if it doesn't exist, use 50 as default.
    bass = preferences.getInt("bass", 0);
    treble = preferences.getInt("treble", 0);
    balance = preferences.getInt("balance", 0);
    bfreq = preferences.getInt("bfreq", 400);
    tfreq = preferences.getInt("tfreq", 8000);
    preferences.end();

    Serial.printf("The current values are: \n- vol:  %d \n- bass: %d \n- treb: %d \n- bal:  %d \n- bfre: %d \n- tfre: %d \n\n", volume, bass, treble, balance, bfreq, tfreq);


    csL.add(&potPreAmp);
    csL.add(&potUnsused);
    csL.add(&potHighpassLevel);
    csL.add(&potLowpassLevel);
    csL.add(&potHighpassFrequency);
    csL.add(&potLowpassLevel);
    csL.printValuesInChain();
    csL.WriteSPI();

    volumeLeft.attach(SERVO_L);
    volumeRight.attach(SERVO_R);

    #ifdef WIFI
    
    //initiate internal file system for the website
    if(!SPIFFS.begin(true)){
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }

    connectWifi();
    Serial.println(WiFi.localIP());

    // Setup mDNS
    if(!MDNS.begin("wavesolutions"))
    {
        Serial.println("Error starting mDNS");
        return;
    }
    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/main.html", String(), false, processor);

    });

    // Route to load style.css file
    server.on("/main.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/main.css", "text/css");
    });

    initialiseGET();
    
    // Start server
    server.begin();
    #endif
    
    /*volumio.connect(volumio_ip, volumio_port);
    if(!volumio.getConnected())
    {
        Serial.println("Volumio is not connected properly, check if volumio is working, else check the volumio IP in code");
    }
    //volumio is connected
*/

}


void loop() {
    /*for(int posDegrees = 0; posDegrees <= 180; posDegrees+= 5) {
        volumeLeft.write(posDegrees);
        Serial.println(posDegrees);
        delay(200);
    }

    for(int posDegrees = 180; posDegrees >= 0; posDegrees-=5) {
        volumeLeft.write(posDegrees);
        Serial.println(posDegrees);
        delay(200);
    }*/
}

void connectWifi()
{
    wifiMulti.addAP(ssid, password);
    int wifiCounter = 0;
    while(wifiMulti.run() != WL_CONNECTED) 
    {
        Serial.print(".");
        wifiCounter++;
        if(wifiCounter>=60)
        { //30 seconds timeout - reset board
            Serial.printf("\n restarting...\n");
            ESP.restart();
        }
        delay(500);
    }
    Serial.println("\n WifiConnected");

}

void initialiseGET()
{
    // Send a GET request to <ESP_IP>/slider?value=<inputMessage>
    server.on("/volume", HTTP_GET, [] (AsyncWebServerRequest *request) 
    {
        String inputMessage;
        // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
        if (request->hasParam("value")) 
        {
            inputMessage = request->getParam("value")->value();
            volume = inputMessage.toInt();
            //sliderValue = inputMessage;
            Serial.print("volume ");
            Serial.println(volume);

            preferences.begin("settings", false);
            preferences.putInt("volume",volume);
            preferences.end();
            
            //setPotMeter(0,100, volume);
        }
        else 
        {
            inputMessage = "No message sent";
        }
        request->send(200, "text/plain", "OK");
    });

    server.on("/treble", HTTP_GET, [] (AsyncWebServerRequest *request) 
    {
        String inputMessage;
        // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
        if (request->hasParam("value")) 
        {
            inputMessage = request->getParam("value")->value();
            treble = inputMessage.toInt();
            //sliderValue = inputMessage;
            Serial.print("treble ");
            Serial.println(treble);

            preferences.begin("settings", false);
            preferences.putInt("treble",treble);
            preferences.end();

            
            setPotMeter(-100,100, treble, potHighpassLevel);
        }
        else 
        {
            inputMessage = "No message sent";
        }
        request->send(200, "text/plain", "OK");
    });

    server.on("/tfreq", HTTP_GET, [] (AsyncWebServerRequest *request) 
    {
        String inputMessage;
        // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
        if (request->hasParam("value")) 
        {
            inputMessage = request->getParam("value")->value();
            tfreq = inputMessage.toInt();
            //sliderValue = inputMessage;
            Serial.print("tfreq ");
            Serial.println(freq);

            preferences.begin("settings", false);
            preferences.putInt("tfreq",tfreq);
            preferences.end();

            setPotMeter(2500,10000, tfreq, potHighpassFrequency);
        }
        else 
        {
            inputMessage = "No message sent";
        }
        request->send(200, "text/plain", "OK");
    });

    server.on("/bass", HTTP_GET, [] (AsyncWebServerRequest *request) 
    {
        String inputMessage;
        // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
        if (request->hasParam("value")) 
        {
            inputMessage = request->getParam("value")->value();
            bass = inputMessage.toInt();
            //sliderValue = inputMessage;
            Serial.print("bass ");
            Serial.println(bass);

            preferences.begin("settings", false);
            preferences.putInt("bass",bass);
            preferences.end();

            setPotMeter(-100,100, bass, potLowpassLevel);
        }
        else 
        {
            inputMessage = "No message sent";
        }
        request->send(200, "text/plain", "OK");
    });

    server.on("/bfreq", HTTP_GET, [] (AsyncWebServerRequest *request) 
    {
        String inputMessage;
        // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
        if (request->hasParam("value")) 
        {
            inputMessage = request->getParam("value")->value();
            bfreq = inputMessage.toInt();
            //sliderValue = inputMessage;
            Serial.print("bfreq ");
            Serial.println(bfreq);

            preferences.begin("settings", false);
            preferences.putInt("bfreq",bfreq);
            preferences.end();

            setPotMeter(100,400, bfreq, potLowpassFrequency);
        }
        else 
        {
            inputMessage = "No message sent";
        }
        
        request->send(200, "text/plain", "OK");
    });

    server.on("/balance", HTTP_GET, [] (AsyncWebServerRequest *request) 
    {
        String inputMessage;
        // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
        if (request->hasParam("value")) 
        {
            inputMessage = request->getParam("value")->value();
            balance = inputMessage.toInt();

            //sliderValue = inputMessage;
            Serial.print("balance ");
            Serial.println(balance);

            preferences.begin("settings", false);
            preferences.putInt("balance",balance);
            preferences.end();
            
            setPotMeter(-100,100, balance, potPreAmp);
        }
        else 
        {
            inputMessage = "No message sent";
        }
        request->send(200, "text/plain", "OK");
    });
    
    server.on("/data", HTTP_GET, [] (AsyncWebServerRequest *request)
    {
        
        String response = String(volume) + "," + String(bass) + "," + String(treble) + "," + String(balance) + "," + String(bfreq) + "," + String(tfreq);
        Serial.print("Sending: ");
        Serial.println(response);
        request->send(200, "text/plain", response);
    });
}
void setPotMeter(int min, int max, int value, Digipot& pot)
{
    int potVal = 255 * (value - min)/(max - min);
    pot.write(potVal);
    Serial.printf("pot value = %d\n\n", potVal);
    
    csL.WriteSPI();
    csR.WriteSPI();
    Serial.println("");
}

void setupPots()
{
    pinMode(SPI_CSL, OUTPUT);
    pinMode(SPI_CSR, OUTPUT);
    digitalWrite(SPI_CSL, HIGH);
    digitalWrite(SPI_CSR, HIGH);


    Serial.println("PotSetupDone");
}

void setupMux()
{  
    pinMode(SW_PIR, OUTPUT);
    digitalWrite(SW_PIR, LOW);
    pinMode(SW_PIL, OUTPUT);
    digitalWrite(SW_PIL, LOW);
    pinMode(SW_AUXL, OUTPUT);
    digitalWrite(SW_AUXL, HIGH);
    pinMode(SW_AUXR, OUTPUT);
    digitalWrite(SW_AUXR, HIGH);

    Serial.println("Mux setup done");
}