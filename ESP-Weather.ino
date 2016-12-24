#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <SHT21.h>
#include <vector>
#include <WebSocketServer.h>
#include <WiFiServer.h>
#include <EEPROM.h>
#include <DNSServer.h>
#include <WiFiManager.h>

#define NTP_PORT 2392
#define MAX_WAIT_TIME 550
#define EEPROM_SIZE 512

// NTP-Client
IPAddress timeServerIP; 
const char* ntpServerName = "time.nist.gov";
const int NTP_PACKET_SIZE = 48; 
byte ntpPacketBuffer[NTP_PACKET_SIZE]; 
WiFiUDP ntp;
unsigned long timestamp = 0;
unsigned long timeReceived = 0;
unsigned long lastStorageTime = 0;
byte storeAtBoot = 1;
unsigned long lastNTP = 0;

SHT21 SHT21;
ESP8266WebServer server(80);
WiFiServer serverSocket(2391);
WebSocketServer webSocketServer;

#define DEFAULT_SSID "ESP-Weather"
#define DEFAULT_PASS "testtest"

IPAddress broadcastIP;

struct __attribute__((__packed__)) Measurement {
  float temperature;
  float humidity;
};

struct __attribute__((__packed__)) Header {
  uint16_t count;
  uint16_t checksum;
};

#define MAX_STORAGE (EEPROM_SIZE - sizeof(Header)) / sizeof(Measurement)

struct __attribute__((__packed__)) PersistentStorage {
  Measurement data[MAX_STORAGE];
  Header header;
};

PersistentStorage storage;

void writeMemory(PersistentStorage &s) {
  Serial.println("write Memory");
  unsigned char* r = (unsigned char*) &s;
  uint16_t a = 0, b = 0;
  for(int i = 0; i < sizeof(PersistentStorage) - 2; i++) {
    a = (a + r[i]) % 255;
    b = (b + a) % 255; 
  }
  s.header.checksum = (b << 8) | a;
  for(int i = 0; i < sizeof(PersistentStorage); i++) {
    EEPROM.write(i, r[i]);
  }
  EEPROM.commit();
}

PersistentStorage readMemory() {
  PersistentStorage s;
  unsigned char* r = (unsigned char*) &s;
  for(int i = 0; i < sizeof(PersistentStorage); i++) {
    r[i] = EEPROM.read(i);
  }
  uint16_t a = 0, b = 0;
  for(int i = 0; i < sizeof(PersistentStorage) - 2; i++) {
    a = (a + r[i]) % 255;
    b = (b + a) % 255; 
  }
  if(s.header.checksum != ((b << 8) | a)) {
    Serial.print("Checksum error ");
    Serial.print(s.header.checksum);
    Serial.print(" ");
    Serial.println((b << 8) | a);
    s.header.count = 0;
  } else {
    Serial.println("Checksum ok");
  }
  return s;
}

std::vector<IPAddress> vecClients; 

// UDP-Config
unsigned int localPort = 2390;
char packetBuffer[255];                                       
char pingBuffer[] = "pingESP8266v0.1";
char echoBuffer[] = "echoESP8266v0.1";
WiFiUDP Udp;

unsigned long lastTime;
bool waitingForReplies = false;

const char* htmlBegin = "<html>\
  <head>\
    <title>Sysadmin</title>\
    <script src=\"http://hoegerle-home.de/sysAdmin/js/jquery-3.1.1.min.js\"></script>\
    <script src=\"http://hoegerle-home.de/sysAdmin/js/bootstrap.min.js\"></script>\
    <script src=\"http://hoegerle-home.de/sysAdmin/js/Chart.bundle.min.js\"></script>\
    <script src=\"http://hoegerle-home.de/sysAdmin/js/script.js\"></script>\
    <link rel=\"stylesheet\" href=\"http://hoegerle-home.de/sysAdmin/css/bootstrap.min.css\" />\
  </head>\
  <body>\
    <script type=\"text/javascript\">";
const char* htmlEnd = "</script>\
  </body>\
</html>";

// root-URL
void handleRoot() {
  // send a reply, to the IP address and port that sent us the packet we received
  Udp.beginPacket(broadcastIP, 2390);
  Udp.write(pingBuffer);
  Udp.endPacket();

  // Timer starten
  lastTime = millis();
  waitingForReplies = true;
  
  Serial.println("Sending UDP Broadcast...");
}

// URL nicht vorhanden
void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup(void) {
  EEPROM.begin(EEPROM_SIZE);
   
  // Debugging
  Serial.begin(115200);
  Serial.println();
  Serial.println("ESP-Weather init...");

  //SHT21.begin();
  // The SHT library is simpy calling Wire.begin(), but the default
  // config does not match the pins i'm using (sda - 2; scl - 0)
  Wire.begin(2, 0);
  
  WiFiManager wifiManager;
  // use one or the other, never both!
  wifiManager.autoConnect(DEFAULT_SSID, DEFAULT_PASS);
  //wifiManager.startConfigPortal(DEFAULT_SSID, DEFAULT_PASS);

  storage = readMemory();

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  broadcastIP = ~WiFi.subnetMask() | WiFi.gatewayIP();
  
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();

  serverSocket.begin();

  // NTP-Client
  ntp.begin(NTP_PORT);
  WiFi.hostByName(ntpServerName, timeServerIP); 
  lastNTP = millis();
  sendNTPpacket(timeServerIP); 
  
  Udp.begin(localPort);
  Serial.println("ESP-Weather ready!");
}

void loop(void){
  server.handleClient();

  // Websocket fuer Browser
  WiFiClient client = serverSocket.available();
  if (client.connected() && webSocketServer.handshake(client)) {
    Serial.println("Building WebSocket Response...");
    
    String json = "{\"H\":";
    json += String(SHT21.getHumidity());
    json += ",";
    json += "\"T\":";
    json += String(SHT21.getTemperature());
    json += ", \"EEPROM\" : [";
    for (int i = 0; i < storage.header.count; i++) {
      json += "{\"H\":";
      json += String(storage.data[i].humidity);
      json += ",";
      json += "\"T\":";
      json += String(storage.data[i].temperature);
      json += "}";
      if (i < storage.header.count - 1) {
        json += ",";
      }
    }
    json += "]}";

    Serial.println("WebSocket Response:");
    Serial.println(json);
    
    webSocketServer.sendData(json);
    client.flush();
    client.stop();
  }

  // NTP wiederholen falls keine Antwort
  if ((timestamp == 0) && ((millis() - lastNTP) > 2000)) {
    Serial.println("NTP packet retry...");
    WiFi.hostByName(ntpServerName, timeServerIP); 
    lastNTP = millis();
    sendNTPpacket(timeServerIP); 
  }
  
  // NTP Paket vom Server erhalten
  if (ntp.parsePacket() >= NTP_PACKET_SIZE) {
    ntp.read(ntpPacketBuffer, NTP_PACKET_SIZE);   
    unsigned long highWord = word(ntpPacketBuffer[40], ntpPacketBuffer[41]);
    unsigned long lowWord = word(ntpPacketBuffer[42], ntpPacketBuffer[43]);
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    const unsigned long seventyYears = 2208988800UL;
    unsigned long epoch = secsSince1900 - seventyYears;
    timestamp = epoch;
    timeReceived = millis();
    Serial.print("Got NTP time: ");
    Serial.println(epoch);
  }

  // EEPROM-Schreiben jede Stunde
  if ((((((millis() - timeReceived) / 1000) + timestamp) % 3600) == 0)
          && (timestamp != 0) && (((millis() - lastStorageTime) > 100000) || storeAtBoot) ) {
      Serial.println("Storing new data packet...");
      lastStorageTime = millis();
      storeAtBoot = 0;
      if (storage.header.count < MAX_STORAGE) {
        storage.header.count++;
      } else {
        for(int i = 0; i < MAX_STORAGE - 1; i++) {
          storage.data[i] = storage.data[i+1];
        }
      }
      storage.data[storage.header.count-1].temperature = SHT21.getTemperature();
      storage.data[storage.header.count-1].humidity = SHT21.getHumidity(); 
      writeMemory(storage); 
  }
  
  // UDP
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    IPAddress remoteIp = Udp.remoteIP();
    // read the packet into packetBufffer
    int len = Udp.read(packetBuffer, 255);
    if (len > 0) {
      packetBuffer[len] = 0;
    }

    Serial.print("Got UDP packet: ");
    Serial.println(packetBuffer);
       
    if (strcmp(packetBuffer, pingBuffer) == 0) {
      Serial.println("Broadcast");
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      Udp.print(echoBuffer);
      Udp.endPacket();
    } else if((strcmp(packetBuffer, echoBuffer) == 0) && (waitingForReplies == true)) {
      vecClients.push_back(Udp.remoteIP());
    }
  }
  
  if (((millis() - lastTime) >= MAX_WAIT_TIME) && (waitingForReplies == true)) {
    Serial.println("Timeout, sending response...");
    waitingForReplies = false;
    String message = htmlBegin;
    message += "var clients = Array(";
    message += "\"" + WiFi.localIP().toString() + "\"";
    if(vecClients.size() > 0) {
      message += ", ";
    }
    for(int i = 0; i < vecClients.size(); i++) {
      message += "\"" + vecClients[i].toString() + "\"";
      if(i < (vecClients.size() - 1)) {
        message += ", ";
      }
    }
    message += ");";
    message += htmlEnd;

    vecClients.clear();
    
    server.send(200, "text/html", message);
  }
}

void sendNTPpacket(IPAddress& address) {
  Serial.println("Sending NTP packet...");
  memset(ntpPacketBuffer, 0, NTP_PACKET_SIZE);
  ntpPacketBuffer[0] = 0b11100011;   // LI, Version, Mode
  ntpPacketBuffer[1] = 0;    
  ntpPacketBuffer[2] = 6;     
  ntpPacketBuffer[3] = 0xEC; 
  ntpPacketBuffer[12]  = 49;
  ntpPacketBuffer[13]  = 0x4E;
  ntpPacketBuffer[14]  = 49;
  ntpPacketBuffer[15]  = 52;

  ntp.beginPacket(address, 123);
  ntp.write(ntpPacketBuffer, NTP_PACKET_SIZE);
  ntp.endPacket();
}

