// Demonstrates usage of the new udpServer feature.
//You can register the same function to multiple ports, and multiple functions to the same port.
//
// 2013-4-7 Brian Lee <cybexsoft@hotmail.com>

#include <EtherCard.h>
#include <IPAddress.h>
#include <FastLED.h>
#define COLOR_ORDER GRB
#define DATA_PIN_1    19
#define CLK_PIN_1   20
#define DATA_PIN_2    22
#define CLK_PIN_2   23
#define LED_TYPE     LPD8806
#define NUM_STRIPS   2
#define NUM_LEDS    1000
//CRGB leds[NUM_LEDS];
CRGB leds[NUM_STRIPS][NUM_LEDS];

#define STATIC 1  // set to 1 to disable DHCP (adjust myip/gwip values below)

#if STATIC
// ethernet interface ip address
static byte myip[] = { 192,168,2,102 };
// gateway ip address
static byte gwip[] = { 192,168,2,1 };
#endif

// ethernet mac address - must be unique on your network
static byte mymac[] = { 0x70,0x69,0x69,0x2D,0x30,0x31 };

byte Ethernet::buffer[5000]; // tcp/ip send and receive buffer



//callback that prints received packets to the serial port
void udpSerialPrint1(short unsigned int port, unsigned char ip[4], short unsigned int len1, const char *data, short unsigned int len2) {
  IPAddress src(ip[0], ip[1], ip[2], ip[3]);
  Serial.println(src);
  Serial.println(port);
  Serial.println(data);
  Serial.println(len2);
  int cntr=0;
  for (int i=0; i<len2; i+=3) {
    leds[0][cntr++] = CRGB(data[i], data[i+1], data[i+2]);
  }
 FastLED.show();
}

void udpSerialPrint2(short unsigned int port, unsigned char ip[4], short unsigned int len1, const char *data, short unsigned int len2) {
  IPAddress src(ip[0], ip[1], ip[2], ip[3]);
  Serial.println(src);
  Serial.println(port);
  Serial.println(data);
  Serial.println(len2);
  int cntr=0;
  for (int i=0; i<len2; i+=3) {
    leds[1][cntr++] = CRGB(data[i], data[i+1], data[i+2]);
  }
 FastLED.show();
}

void setup(){
  Serial.begin(57600);
  Serial.println(F("\n[backSoon]"));

  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0)
    Serial.println(F("Failed to access Ethernet controller"));
#if STATIC
  ether.staticSetup(myip, gwip);
#else
  if (!ether.dhcpSetup())
    Serial.println(F("DHCP failed"));
#endif
//  ether.printIp("IP:  ", ether.myip);
//  ether.printIp("GW:  ", ether.gwip);
//  ether.printIp("DNS: ", ether.dnsip);
  //register udpSerialPrint() to port 1337
  ether.udpServerListenOnPort(&udpSerialPrint1, 3000);
  ether.udpServerListenOnPort(&udpSerialPrint2, 3001);
  FastLED.addLeds<LED_TYPE,DATA_PIN_1,CLK_PIN_1,COLOR_ORDER>(leds[0], NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE,DATA_PIN_2,CLK_PIN_2,COLOR_ORDER>(leds[1], NUM_LEDS).setCorrection(TypicalLEDStrip);
}

void loop(){
  //this must be called for ethercard functions to work.
  ether.packetLoop(ether.packetReceive());
}


/*
//Processing sketch to send test UDP packets.

import hypermedia.net.*;

 UDP udp;  // define the UDP object


 void setup() {
 udp = new UDP( this, 6000 );  // create a new datagram connection on port 6000
 //udp.log( true );     // <-- printout the connection activity
 udp.listen( true );           // and wait for incoming message
 }

 void draw()
 {
 }

 void keyPressed() {
 String ip       = "192.168.0.200";  // the remote IP address
 int port        = 1337;    // the destination port

 udp.send("Greetings via UDP!", ip, port );   // the message to send

 }

 void receive( byte[] data ) {       // <-- default handler
 //void receive( byte[] data, String ip, int port ) {  // <-- extended handler

 for(int i=0; i < data.length; i++)
 print(char(data[i]));
 println();
 }
*/
