/*
  Web Server
  Need to install Ethernet2 arduino library
  If new arduino esp32 or make error, need go to c:\Program Files (x86)\Arduino\hardware\espressif\arduino-esp32\cores\esp32\Server.h 
  Change virtual void begin(uint16_t port = 0) = 0; to virtual void begin() = 0;
  Other example can see https://github.com/adafruit/Ethernet2
 */
#include <M5Stack.h>
#include <SPI.h>
#include <Ethernet2.h>
#include "UNIT_4RELAY.h"
UNIT_4RELAY unit_4relay;
//#include <M5GFX.h>

//M5GFX display;
//M5Canvas canvas(&display);
#define SCK 18
#define MISO 19
#define MOSI 23
#define CS 26
#define COUNT 100
#define TIMES 50
const char *STOP = "StopSteam";
const char *START = "StartSteam";
const char *VERSION = "get_versions";
static uint8_t count;
static uint8_t stop_count;
static uint8_t start_count;
static uint8_t last_stop_count;
static uint8_t last_start_count;
static uint8_t stop_send_times;
static uint8_t start_send_times;
static int64_t last_get_time;
const char stop_buf[8] = {0x1B, 0x2A, 0x02, 0x15, 0x00, 0x00, 0x00, 0x5C};
const char start_buf[8] = {0x1B, 0x2A, 0x02, 0x15, 0x01, 0x00, 0x00, 0x5D};
uint8_t stream_switch = 0;
// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 1, 187);
unsigned int localPort = 8888; // local port to listen on

// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; //buffer to hold incoming packet,
char ReplyBuffer[] = "acknowledged";       // a string to send back
EthernetUDP Udp;
void setup()
{
    // Open serial communications and wait for port to open:
    M5.begin(true, false, true, true);
    unit_4relay.Init(0); //Set the lamp and relay to asynchronous mode(Async = 0,Sync = 1).  将灯和继电器设为非同步模式
    SPI.begin(SCK, MISO, MOSI, -1);
    Ethernet.init(CS);
    // start the Ethernet connection and the server:
    Ethernet.begin(mac, ip);
    Udp.begin(localPort);
    Serial.begin(9600);
    M5.Lcd.println("M5Stack W5500 Test");
    M5.Lcd.println(" ");
    M5.Lcd.print(Ethernet.localIP());
    M5.Lcd.println(" ");
    M5.Lcd.print(localPort);
}

void loop()
{
    // if there's data available, read a packet
    int packetSize = Udp.parsePacket();
    count++;
    if (packetSize)
    {
        //Serial.print("Received packet of size ");
        //Serial.println(packetSize);
        //Serial.print("From ");
        IPAddress remote = Udp.remoteIP();
        for (int i = 0; i < 4; i++)
        {
            // Serial.print(remote[i], DEC);
            if (i < 3)
            {
                //Serial.print(".");
            }
        }
        //Serial.print(", port ");
        //Serial.println(Udp.remotePort());

        // read the packet into packetBufffer
        Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
        //Serial.println("Contents:");
        // Serial.println(packetBuffer);
        if (strncmp("StartSteam", packetBuffer, strlen("StartSteam")) == 0)
        {
            //TODO 开蒸汽
            stream_switch = 1;
            start_count++;
            last_get_time = esp_timer_get_time();
            //            Serial.write(start_buf, 8);
            unit_4relay.LEDWrite(0, 1);
            unit_4relay.relayWrite(0, 1);
            delay(500);
            unit_4relay.LEDWrite(0, 0);
            unit_4relay.relayWrite(0, 0);

            // send a reply, to the IP address and port that sent us the packet we received
            Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
            Udp.write(START);
            Udp.endPacket();
        }
        else if ((strncmp("StopSteam", packetBuffer, strlen("StopSteam")) == 0))
        {
            //TODO 关蒸汽
            stream_switch = 0;
            stop_count++;
            //           Serial.write(stop_buf, 8);
            unit_4relay.LEDWrite(0, 1);
            unit_4relay.relayWrite(0, 1);
            delay(500);
            unit_4relay.LEDWrite(0, 0);
            unit_4relay.relayWrite(0, 0);
            // send a reply, to the IP address and port that sent us the packet we received
            Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
            Udp.write(STOP);
            Udp.endPacket();
            M5.Lcd.print((esp_timer_get_time() - last_get_time));
        }
        else if (strncmp(VERSION, packetBuffer, sizeof(*VERSION)) == 0)
        {
            Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
            Udp.write("version_01");
            Udp.endPacket();
        }
        else
        {
            // Serial.println("packetBuffer error");
        }
    }
    if ((esp_timer_get_time() - last_get_time > 40000000)&&(stream_switch == 1))
    {
        //TODO 关蒸汽
        stream_switch = 0;
        stop_count++;
        //        Serial.write(stop_buf, 8);
        unit_4relay.LEDWrite(0, 1);
        unit_4relay.relayWrite(0, 1);
        delay(500);
        unit_4relay.LEDWrite(0, 0);
        unit_4relay.relayWrite(0, 0);
    }
}
