// Firmware for ESP-01 to transmit data received via UART to a server using a GET request.
// Data format: name1 value1 name2 value2 .. nameN valueN 0x0D - pairs of parameter name - data,
// separated by spaces, with a \r character at the end.
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "secrets.h"

#define LED1 2 // D4 BLUE LED "COM"

#define DEBUG_UART // output information to UART

// #define LED2 16 // D0 BLUE LED "WAKE"

// #define TXD 0 // D3 FLASH
// #define TXD 1 // TX
// #define TXD 2 // D4 BLUE LED "COM"
// #define TXD 3 // RX
// #define TXD 4 // D2
// #define TXD 5 // D1
// #define TXD 9 // SD2
// #define TXD 10 // SD3
// #define TXD 12 // D6
// #define TXD 13 // D7  RXD2
// #define TXD 14 // D5
// #define TXD 15 // D8  TXD2
// #define TXD 16 // D0  BLUE LED2 WAKE

String ServerAddress = "http://meteo.antresoldo.ru/"; // Server address
String serverPath = "";

// Secret data (ssid, pass, key) is moved to secrets.h
String StationID = "6"; // Station ID (channel)

WiFiClient wclient; // WiFi client
HTTPClient http;    // Create Object of HTTPClient
int httpCode;       // server response code
String payload;     // server response string
String error;       // error code description

long rssi; // WiFi router signal strength

unsigned long TimeToSend1;
unsigned long TimeToSend2;
unsigned long TimeToSend;    // time to send the packet
unsigned long TimeToConnect; // time to connect to the router
unsigned long TimeTotal1;
unsigned long TimeTotal2;
unsigned long TimeTotal; // total time to process the request

int countUartRx = 0; // number of received bytes in the packet

char uartRxBuffer[1024]; // receive buffer for hardware UART

int counterPacket = 0;

void OffLEDBLUE(void);
void OnLEDBLUE(void);

void parseUartRxBuffer(void);
void sendToServer(void);
void connectToWiFi(void);

void setup()
{
  pinMode(2, OUTPUT);
  OnLEDBLUE();
  Serial.begin(115200); // initialize UART interface at 115200 baud
  delay(10);
#ifdef DEBUG_UART
  Serial.println(); // new line
  Serial.println();
  Serial.println("RESET!");
  Serial.print("Setup ... ");
  Serial.println("OK");
#endif
  connectToWiFi();

  OffLEDBLUE();
  delay(1000);
  OffLEDBLUE();

  // serverPath = "&s0=1.12345&s1=1.00001&s2=2.00002&s3=3.00003&s4=4.00004&s5=5.00005&s6=6.00006$&s7=7.00007&v=8.00008"; // test string
  // sendToServer();
}

void loop()
{
  if (Serial.available() > 0) // one or more bytes received
  {
    OnLEDBLUE();

    uartRxBuffer[countUartRx] = Serial.read(); // save the character to the receive buffer
#ifdef DEBUG_UART
    Serial.printf("%c", uartRxBuffer[countUartRx]); // print the character to UART
#endif
    countUartRx++;

    if (uartRxBuffer[countUartRx - 1] == '\r') // newline character 0x0D detected
    {
      TimeTotal1 = millis();

      parseUartRxBuffer(); // разбор строки

#ifdef DEBUG_UART
      Serial.println("");
      Serial.printf("\rUART received: %d Bytes", countUartRx);
      Serial.println("");
      Serial.println("");
#endif
      countUartRx = 0; // reset buffer pointer to the 0th element

      sendToServer();

      serverPath = ""; // clear the string

      TimeTotal2 = millis();
      TimeTotal = TimeTotal2 - TimeTotal1; // total time to process the UART request
#ifdef DEBUG_UART
      Serial.print("totalTime: ");
      Serial.print(TimeTotal);
      Serial.println(" ms ");
      Serial.println("------------------------------------------------");
      Serial.println("");
#endif
      OffLEDBLUE();
    }
  }
}

void parseUartRxBuffer(void)
{
  int i = 0;

  while (i < (countUartRx - 1)) // iterate through all characters of the string
  {
    serverPath += '&';
    while (uartRxBuffer[i] != ' ') // until a space is found
    {
      serverPath += uartRxBuffer[i]; // append character to the string - building the name
      i++;
      // Serial.println(serverPath);
    }
    i++;

    serverPath += '=';
    while ((uartRxBuffer[i] != ' ')) // until a space is found or the end of the array is reached
    {
      serverPath += uartRxBuffer[i]; // append character to the string - building the value
      i++;
      // Serial.println(serverPath);
      if (i >= (countUartRx - 1)) // exit the function if the end of the string is reached and the second space is not found
        return;
    }
    i++;
  }
}
void connectToWiFi(void)
{
  int counterConnect = 100; // timeout 10 sec
  // connect to wi-fi if not connected to the router
  if (WiFi.status() != WL_CONNECTED)
  {
#ifdef DEBUG_UART
    Serial.print("\rConnecting to WiFi ");
    Serial.print(ssid);
    Serial.print(" ");
#endif
    TimeToSend1 = millis();
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(100);
      counterConnect--;
      if (counterConnect == 0)
      {
        Serial.println("status 999");

        OffLEDBLUE();
        delay(100);
        OnLEDBLUE();
        delay(200);
        OffLEDBLUE();
        delay(100);
        OnLEDBLUE();
        delay(200);
        OffLEDBLUE();
        delay(100);
        OnLEDBLUE();
        delay(200);
        OffLEDBLUE();
        return;
      }
#ifdef DEBUG_UART
      Serial.print(".");
#endif
    }
    TimeToSend2 = millis();
    TimeToConnect = TimeToSend2 - TimeToSend1;

    if (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
      Serial.println("status 999"); //------------------------------------------------------------------------------------------------

      OffLEDBLUE();
      delay(100);
      OnLEDBLUE();
      delay(200);
      OffLEDBLUE();
      delay(100);
      OnLEDBLUE();
      delay(200);
      OffLEDBLUE();
      delay(100);
      OnLEDBLUE();
      delay(200);
      OffLEDBLUE();

      return;
    }
    else
    {
#ifdef DEBUG_UART
      Serial.println(" OK");
      Serial.print("timeToConnect: ");
      Serial.print(TimeToConnect);
      Serial.print(" ms ");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
#endif
    }
  }

#ifdef DEBUG_UART
  Serial.print("\rWiFi connected. ");
#endif

  rssi = WiFi.RSSI();
#ifdef DEBUG_UART
  Serial.print("RSSI: ");
  Serial.print(rssi);
  Serial.println(" dBm");
#endif
}

void sendToServer(void)
{
  int counterConnect = 100; // timeout 10 sec
  // connect to wi-fi if not connected to the router
  if (WiFi.status() != WL_CONNECTED)
  {
#ifdef DEBUG_UART
    Serial.print("\rConnecting to WiFi ");
    Serial.print(ssid);
    Serial.print(" ");
#endif
    TimeToSend1 = millis();
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(100);
      counterConnect--;
      if (counterConnect == 0)
      {
        Serial.println("status 999");

        OffLEDBLUE();
        delay(100);
        OnLEDBLUE();
        delay(200);
        OffLEDBLUE();
        delay(100);
        OnLEDBLUE();
        delay(200);
        OffLEDBLUE();
        delay(100);
        OnLEDBLUE();
        delay(200);
        OffLEDBLUE();
        return;
      }
#ifdef DEBUG_UART
      Serial.print(".");
#endif
    }
    TimeToSend2 = millis();
    TimeToConnect = TimeToSend2 - TimeToSend1;

    if (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
      Serial.println("status 999"); //------------------------------------------------------------------------------------------------

      OffLEDBLUE();
      delay(100);
      OnLEDBLUE();
      delay(200);
      OffLEDBLUE();
      delay(100);
      OnLEDBLUE();
      delay(200);
      OffLEDBLUE();
      delay(100);
      OnLEDBLUE();
      delay(200);
      OffLEDBLUE();

      return;
    }
    else
    {
#ifdef DEBUG_UART
      Serial.println(" OK");
      Serial.print("timeToConnect: ");
      Serial.print(TimeToConnect);
      Serial.print(" ms ");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
#endif
    }
  }

#ifdef DEBUG_UART
  Serial.print("\rWiFi connected. ");
#endif

  rssi = WiFi.RSSI();
#ifdef DEBUG_UART
  Serial.print("RSSI: ");
  Serial.print(rssi);
  Serial.println(" dBm");
#endif

  String serverPath2 = ServerAddress + "?key=" + key + "&station=" + StationID + serverPath;

#ifdef DEBUG_UART
  Serial.print("GET: ");
  Serial.println(serverPath2);
#endif

  TimeToSend1 = millis();

  http.begin(wclient, serverPath2); //
  httpCode = http.GET();

  error = http.errorToString(httpCode).c_str();
  payload = http.getString(); // payload = Response from server

  // wclient.stop();
  http.end(); // Close Connection

  TimeToSend2 = millis();

  TimeToSend = TimeToSend2 - TimeToSend1;

  TimeTotal2 = millis();
  TimeTotal = TimeTotal2 - TimeTotal1; // total time to process the UART request

#ifdef DEBUG_UART
  Serial.print("timeToSend: ");
  Serial.print(TimeToSend);
  Serial.print(" ms ");
#endif
  Serial.print("status "); //------------------------------------------------------------------------------------------------
  Serial.print(httpCode);
  // Serial.print(" ");
  Serial.print(" rssi ");
  Serial.print(rssi);
  Serial.print(" time ");
  // Serial.print(" ");
  Serial.print(TimeTotal);

  if (httpCode > 0)
  {
#ifdef DEBUG_UART
    Serial.print("\thttpCode: ");
    Serial.print(httpCode);
    Serial.print("\t");
    Serial.print(error);
    if (httpCode == 200)
    {
      Serial.println(" [OK]\t");
    }
    // Serial.print("payload: ");
    // Serial.println(payload);
#endif
  }
  else
  {
#ifdef DEBUG_UART
    Serial.print("failed, error: "); // Display Error msg to PC
    Serial.print(httpCode);
    Serial.print(":\t");
    Serial.println(error);
#endif
  }
  counterPacket++;
#ifdef DEBUG_UART
  Serial.printf("send packets: %d\t", counterPacket);
#endif
}

void OffLEDBLUE(void) // Turn off LED1
{                     // turn the LED off (HIGH is the voltage level)
  digitalWrite(2, HIGH);
} // write 1 to the blue LED port
void OnLEDBLUE(void) // Turn on LED1
{                    // turn the LED on by making the voltage LOW
  digitalWrite(2, LOW);
} // write 0 to the blue LED port