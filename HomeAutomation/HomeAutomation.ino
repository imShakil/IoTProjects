#include <Pinger.h>
#include <ESP8266WebServer.h>
#include "./esppl_functions.h"

/*Wifi Router SSID & Password Required to connect with NodeMCU*/
const char* ssid = "ICT_Innovation_Lab";  // Enter SSID here
const char* password = "beinnovative#";  //Enter Password here

ESP8266WebServer server(80);
Pinger pinger;

int pin1 = 13;
bool LED1status = LOW;
int pin2 = 12;
bool LED2status = LOW;
bool st = false;
int cooldown = 0;

#define LIST_SIZE 1 // define list of client

// Stored Users Mac Addresses
uint8_t friendmac[LIST_SIZE][ESPPL_MAC_LEN] = {
   {0xec, 0x9b, 0xf3, 0x3c, 0xe2, 0xdf}
  };

// Define identity of devices
String friendname[LIST_SIZE] = {
   "Target 1"
  };

void setup() {
  Serial.begin(115200);
  delay(100);
  pinMode(pin1, OUTPUT);
  pinMode(pin2, OUTPUT);

  Serial.println("Connecting to ");
  Serial.println(ssid);

  //connect to your local wi-fi network
  WiFi.begin(ssid, password);

  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  
  Serial.println(WiFi.localIP());

  server.on("/", handle_OnConnect);
  server.on("/led1on", handle_led1on);
  server.on("/led1off", handle_led1off);
  server.on("/led2on", handle_led2on);
  server.on("/led2off", handle_led2off);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");


  // Is the device is received packet or not?
  pinger.OnReceive([] (const PingerResponse& response) {

    if (response.ReceivedResponse) {

       Serial.printf(
        "Reply from %s: bytes=%d time=%lums TTL=%d\n",
        response.DestIPAddress.toString().c_str(),
        response.EchoMessageSize - sizeof(struct icmp_echo_hdr),
        response.ResponseTime,
        response.TimeToLive);
      
      return true;
    }
    else {
      Serial.println("Requested time out");

      return false;
    }    
  });

  // After sending request checking the resonse rate
  pinger.OnEnd([] (const PingerResponse& response) {

    // Evaluate lost packet percentage
    float loss = 100;
    if(response.TotalReceivedResponses > 0)
    {
      if(st == false) {
        server.handleClient();
      }
      loss = (response.TotalSentRequests - response.TotalReceivedResponses) * 100 / response.TotalSentRequests;
      st = true;

      return true;
    }
    else {
      st = false;

      return false;
    }

    // Print packet trip data
    Serial.printf(
      "Ping statistics for %s:\n",
      response.DestIPAddress.toString().c_str());
    Serial.printf(
      "    Packets: Sent = %lu, Received = %lu, Lost = %lu (%.2f%% loss),\n",
      response.TotalSentRequests,
      response.TotalReceivedResponses,
      response.TotalSentRequests - response.TotalReceivedResponses,
      loss);

      // Print time information
    if(response.TotalReceivedResponses > 0)
    {
      Serial.printf("Approximate round trip times in milli-seconds:\n");
      Serial.printf(
        "    Minimum = %lums, Maximum = %lums, Average = %.2fms\n",
        response.MinResponseTime,
        response.MaxResponseTime,
        response.AvgResponseTime);
    }
    else {
      Serial.println("No Data recieved.");
      Serial.println("Device is out of bound");
    }
   // Print host data
    Serial.printf("Destination host data:\n");
    Serial.printf(
      "    IP address: %s\n",
      response.DestIPAddress.toString().c_str());
    if(response.DestMacAddress != nullptr)
    {
      Serial.printf(
        "    MAC address: " MACSTR "\n",
        MAC2STR(response.DestMacAddress->addr));
    }
    if(response.DestHostname != "")
    {
      Serial.printf(
        "    DNS name: %s\n",
        response.DestHostname.c_str());
    }
  });
  
}

void loop() {
  
   IsConnected();
   delay(30000);
}

// Comparing MAC Address to find the right one 
bool maccmp(uint8_t *mac1, uint8_t *mac2) {
  for (int i=0; i < ESPPL_MAC_LEN; i++) {
    if (mac1[i] != mac2[i]) {
      return false;
    }
  }
  return true;
}

// Collecting MAC Address to check target device is connected or not
void cb(esppl_frame_info *info) {
  for (int i=0; i<LIST_SIZE; i++) {
    if (maccmp(info->sourceaddr, friendmac[i]) || maccmp(info->receiveraddr, friendmac[i])) {
      Serial.printf("\n%s is here! :)", friendname[i].c_str());
      cooldown = 100; // here we set it to 1000 if we detect a packet that matches our list
      if (i == 0){
        Serial.println("Device is nearby");
        if(st == false) {

        }
        st = true;
       } // Here we turn on the blue LED until turnoff() is called
    }

      else { // this is for if the packet does not match any we are tracking
        if (cooldown > 0) {
          Serial.println("Device far away");
          cooldown--; } //subtract one from the cooldown timer if the value of "cooldown" is more than one
          else { // If the timer is at zero, then run the turnoff function to turn off any LED's that are on.
            st = false;
        } 
      }  
    } 
  }

bool IsConnected() {
  
  if(pinger.Ping(IPAddress(192, 168, 1, 199)  )) {
    server.handleClient();
  
    if(LED1status)
    {digitalWrite(pin1, HIGH);}
    else
    {digitalWrite(pin1, LOW);}
    
    if(LED2status)
    {digitalWrite(pin2, HIGH);}
    else
    {digitalWrite(pin2, LOW);}
    Serial.println("Device found");
  }
  else {
    // Collecting connected devices mac addresses.
    esppl_sniffing_start();
    while (true) {
        for (int i = ESPPL_CHANNEL_MIN; i <= ESPPL_CHANNEL_MAX; i++ ) {
          esppl_set_channel(i);
          while (esppl_process_frames()) {
            //
          }
       }
    } 
    Serial.println("Device not found");
  }
}

void handle_OnConnect() {
  LED1status = HIGH;
  LED2status = HIGH;
  Serial.println("GPIO7 Status: OFF | GPIO6 Status: OFF");
  server.send(200, "text/html", SendHTML(LED1status,LED2status)); 
}

void handle_DisConnect() {
  
}

void handle_led1on() {
  LED1status = HIGH;
  Serial.println("GPIO7 Status: ON");
  server.send(200, "text/html", SendHTML(true,LED2status)); 
}

void handle_led1off() {
  LED1status = LOW;
  Serial.println("GPIO7 Status: OFF");
  server.send(200, "text/html", SendHTML(false,LED2status)); 
}

void handle_led2on() {
  LED2status = HIGH;
  Serial.println("GPIO6 Status: ON");
  server.send(200, "text/html", SendHTML(LED1status,true)); 
}

void handle_led2off() {
  LED2status = LOW;
  Serial.println("GPIO6 Status: OFF");
  server.send(200, "text/html", SendHTML(LED1status,false)); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML(uint8_t led1stat,uint8_t led2stat){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>LED Control</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #1abc9c;}\n";
  ptr +=".button-on:active {background-color: #16a085;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>Manual Control Webserver</h1>\n";
    ptr +="<h3>Using Station(STA) Mode</h3>\n";
  
   if(led1stat)
  {ptr +="<p>LED1 Status: ON</p><a class=\"button button-off\" href=\"/led1off\">OFF</a>\n";}
  else
  {ptr +="<p>LED1 Status: OFF</p><a class=\"button button-on\" href=\"/led1on\">ON</a>\n";}

  if(led2stat)
  {ptr +="<p>LED2 Status: ON</p><a class=\"button button-off\" href=\"/led2off\">OFF</a>\n";}
  else
  {ptr +="<p>LED2 Status: OFF</p><a class=\"button button-on\" href=\"/led2on\">ON</a>\n";}

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}
