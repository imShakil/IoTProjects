#define BLYNK_PRINT Serial


#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
// char auth[] = "VeLFwlMz7vx0dk-W83hgHRzlA80iiDTo";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "ICT_Innovation_Lab";
char pass[] = "beinnovative#";

// Set server port
WiFiServer server(80);

String header;
String ob1 = "OFF";
String ob2 = "OFF";

const int op1 = 13;
const int op2 = 12;

unsigned long CurrentTime = millis();
unsigned long PreviousTime = 0;
const long TimeOut = 2000;

void setup()
{
  // Debug consoles
  Serial.begin(9600);
  Serial.println("Hello World");
  // WiFiServer server(80);
  
  pinMode(op1, OUTPUT);
  pinMode(op2, OUTPUT);

  digitalWrite(op1, LOW);
  digitalWrite(op2, LOW);

 // Blynk.begin(auth, ssid, pass);

 Serial.print("Connecting to");
 Serial.print(ssid);
 WiFi.begin(ssid, pass);

 while(WiFi.status() != WL_CONNECTED) {
  delay(500);
  Serial.print(".");
 }

 Serial.println("\nWifi Connected");
 Serial.print("IP Address: ");
 Serial.print(WiFi.localIP());
 Serial.println();
 server.begin();
 
}

void loop()
{
  
 // digitalWrite(13, HIGH);
 // delay(2000);
 // digitalWrite(13, LOW);
 // delay(1000);
 // Blynk.run();

  WiFiClient Client = server.available();    // Finding available Clients 

  if (Client) {
    Serial.println("New Client: ");
    String CurrentLine = "";
    CurrentTime = millis();
    PreviousTime = CurrentTime;

    while (Client.connected() && CurrentTime - PreviousTime <= TimeOut) {
      CurrentTime = millis();

      if(Client.available()){
        char ch = Client.read();
        Serial.write(ch);
        header += ch;

        if(ch == '\n')
        {
          if (CurrentLine.length() == 0) {
            Client.println("HTTP/1.1 200 OK");
            Client.println("Content-type:text/html");
            Client.println("Connection: close");
            Client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /5/on") >= 0) {
              Serial.println("GPIO 5 on");
              ob1 = "ON";
              digitalWrite(op1, HIGH);
            } else if (header.indexOf("GET /5/off") >= 0) {
              Serial.println("GPIO 5 off");
              ob1 = "OFF";
              digitalWrite(op1, LOW);
            } else if (header.indexOf("GET /4/on") >= 0) {
              Serial.println("GPIO 4 on");
              ob2 = "ON";
              digitalWrite(op2, HIGH);
            } else if (header.indexOf("GET /4/off") >= 0) {
              Serial.println("GPIO 4 off");
              ob2 = "OFF";
              digitalWrite(op2, LOW);
            }

            // Display the HTML web page
            Client.println("<!DOCTYPE html><html>");
            Client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            Client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            Client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            Client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            Client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            Client.println(".button2 {background-color: #77878A;}</style></head>");
            
            // Web Page Heading
            Client.println("<body><h1>ESP8266 Web Server</h1>");
            
            // Display current state, and ON/OFF buttons for GPIO 5  
            Client.println("<p>GPIO 5 - State " + ob1 + "</p>");
            // If the output5State is off, it displays the ON button       
            if (ob1=="OFF") {
              Client.println("<p><a href=\"/5/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              Client.println("<p><a href=\"/5/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
               
            // Display current state, and ON/OFF buttons for GPIO 4  
            Client.println("<p>GPIO 4 - State " + ob2 + "</p>");
            // If the output4State is off, it displays the ON button       
            if (ob2=="OFF") {
              Client.println("<p><a href=\"/4/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              Client.println("<p><a href=\"/4/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            Client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            Client.println();
            // Break out of the while loop
            break;
          } else{
            CurrentLine = "";
          }
           
        } else if (ch != '\r'){
          CurrentLine += ch;
        }
        
      }
    }

    header = "";
    Client.stop();
    Serial.println("Client Disconnected.\n");
    
  } 
}
