#include <WiFiS3.h>
#include <SPI.h>
#include <MFRC522.h>

const char* ssid = "";
const char* password = "";
String lineNotifyToken = "";

String message = "Your attendance has been checked!!!";

const char* server = "notify-api.line.me";
const int port = 443;

WiFiSSLClient client;
MFRC522 rfid(10, 9);

const char* googleScriptHost = "script.google.com";
String googleScriptPath = "";
const int httpsPort = 443;

unsigned long previousMillis = 0;
const long interval = 1000;

void setup() {
  Serial.begin(115200);
  SPI.begin();
  rfid.PCD_Init();
  
  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to Wi-Fi...");
  }
  Serial.println("Connected to Wi-Fi");

  Serial.println("Scan RFID card to send Attendance check...");
}

void loop() {
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    sendLineNotify();

    // Get the card ID
    String cardID = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
      cardID += String(rfid.uid.uidByte[i], HEX);
    }

    sendToGoogleSheet(cardID);
    rfid.PICC_HaltA();
  }

  delay(1000);
}

void sendLineNotify() {
  if (!client.connect(server, port)) {
    Serial.println("Connection to server failed");
    return;
  }

  String postRequest = String("POST /api/notify HTTP/1.1\r\n") +
                       "Host: " + server + "\r\n" +
                       "Authorization: Bearer " + lineNotifyToken + "\r\n" +
                       "Content-Type: application/x-www-form-urlencoded\r\n" +
                       "Connection: close\r\n" +
                       "Content-Length: " + String(message.length() + 8) + "\r\n\r\n" +
                       "message=" + message;

  client.print(postRequest);

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }

  while (client.available()) {
    String response = client.readString();
    Serial.println(response);
  }

  client.stop();
}

void sendToGoogleSheet(String cardID) {
  if (!client.connect(googleScriptHost, httpsPort)) {
    Serial.println("Connection to Google Sheets failed");
    return;
  }

  String payload = "{\"cardID\":\"" + cardID + "\"}";

  String postRequest = String("POST ") + googleScriptPath + " HTTP/1.1\r\n" +
                       "Host: " + googleScriptHost + "\r\n" +
                       "Content-Type: application/json\r\n" +
                       "Content-Length: " + payload.length() + "\r\n" +
                       "Connection: close\r\n\r\n" + payload;

  client.print(postRequest);

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }

  while (client.available()) {
    String response = client.readString();
    Serial.println(response);
  }

  client.stop();
}
