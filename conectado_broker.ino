#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <WiFiClientSecure.h> // Biblioteca para conexión segura a WiFi
#include <PubSubClient.h>     // Biblioteca para gestión de MQTT
#include <ESP32Servo.h>       // Biblioteca para control de servo

// Pines de configuración para RFID y componentes adicionales
#define SS_PIN 5           // Pin para SS (Slave Select) del lector RFID
#define RST_PIN 22         // Pin de reset del lector RFID
#define GREEN_LED_PIN 14   // Pin del LED verde
#define RED_LED_PIN 15     // Pin del LED rojo
#define SERVO_PIN 27       // Pin del servo

// Credenciales de WiFi y configuración del servidor MQTT
const char *ssid = "AEG-IKASLE";        // Nombre de la red WiFi
const char *password = "Ea25dneAEG";    // Contraseña de la red WiFi
const char *mqttServer = "10.80.128.2"; // Dirección del servidor MQTT
const int mqttPort = 8883;              // Puerto seguro para el servidor MQTT

// Certificados de seguridad para SSL
const char *certificate_request = "-----BEGIN CERTIFICATE-----\n..."; // Certificado raíz del servidor
const char *certificate = "-----BEGIN CERTIFICATE-----\n..."; // Certificado del dispositivo
const char *private_key = "-----BEGIN PRIVATE KEY-----\n..."; // Clave privada del dispositivo

// Objetos para WiFi seguro y MQTT
WiFiClientSecure ESP_EIAS;
PubSubClient client(ESP_EIAS);

// Configuración de RFID
MFRC522 rfid = MFRC522(SS_PIN, RST_PIN); // Objeto para lector RFID
MFRC522::MIFARE_Key key;                 // Llave para autenticación en RFID

// Variables de control de estado
bool waitingForAccessResponse = false; // Indica si se espera respuesta de acceso
unsigned long waitingStartTime = 0;    // Tiempo de inicio de espera
String uidStr = "";                    // Almacena el UID de la tarjeta

// Función para reconectar WiFi y MQTT
void reconnect() {
  // Conexión a WiFi
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reconnecting to WiFi...");
    WiFi.begin(ssid, password);
    delay(5000);
  }

  // Conexión al servidor MQTT
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32EIAS")) {
      Serial.println("connected");
      client.subscribe("EIASOpenDoor");       // Suscribirse al tema de apertura de puerta
      client.subscribe("EIASOpenDoorDenied"); // Suscribirse al tema de acceso denegado
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// Función para manejar los mensajes recibidos de MQTT
void callback(char *topic, byte *payload, unsigned int length) {
  String receivedMessage = "";
  for (unsigned int i = 0; i < length; i++) {
    receivedMessage += (char)payload[i];
  }

  Serial.print("Message received on topic ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(receivedMessage);

  if (strcmp(topic, "EIASOpenDoor") == 0) {
    Serial.println("Access granted. Opening door...");
    openDoor();
    delay(1000);
    Serial.println("The door is open");
    client.publish("EIASdoorOpened", uidStr.c_str()); // Publicar evento de apertura
  } else if (strcmp(topic, "EIASOpenDoorDenied") == 0) {
    Serial.println("Access denied. Showing message or alert...");
    blinkLED(RED_LED_PIN, 2); // Parpadea el LED rojo dos veces
    delay(200);               // Pequeña demora para evitar rebotes
  }

  waitingForAccessResponse = false;
}

// Configuración inicial del sistema
void setup() {
  // Configuración de pines
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  servo.attach(SERVO_PIN);
  servo.write(0);                   // Posición inicial del servo en 0 grados
  digitalWrite(GREEN_LED_PIN, LOW); // LED verde apagado inicialmente
  digitalWrite(RED_LED_PIN, LOW);   // LED rojo apagado inicialmente

  Serial.begin(115200);
  Serial.println(F("Initialize System"));

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }

  Serial.println();
  Serial.print("Connected to:\t");
  Serial.println(WiFi.SSID());
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());

  SPI.begin();       // Inicializar comunicación SPI
  rfid.PCD_Init();   // Inicializar el lector RFID
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));

  // Configuración de seguridad SSL
  ESP_EIAS.setCACert(certificate_request);
  ESP_EIAS.setCertificate(certificate);
  ESP_EIAS.setPrivateKey(private_key);

  client.setServer(mqttServer, mqttPort); // Configurar el servidor MQTT
  client.setCallback(callback);           // Configurar la función de manejo de mensajes

  reconnect(); // Conectar a WiFi y MQTT
}

// Función principal
void loop() {
  if (!client.connected()) {
    reconnect(); // Intentar reconexión si se pierde la conexión
  }
  client.loop();

  if (waitingForAccessResponse && (millis() - waitingStartTime > 10000)) {
    Serial.println("Waiting time exceeded. Restarting card reading...");
    waitingForAccessResponse = false;
  }

  readRFID(); // Leer tarjeta RFID si es posible
}

// Función para leer tarjeta RFID
void readRFID(void) {
  if (waitingForAccessResponse)
    return;

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF; // Clave por defecto
  }

  if (!rfid.PICC_IsNewCardPresent()) return; // Verificar nueva tarjeta
  if (!rfid.PICC_ReadCardSerial()) return;   // Leer serial de la tarjeta

  uidStr = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uidStr += String(rfid.uid.uidByte[i], HEX);
    if (i < rfid.uid.size - 1)
      uidStr += ":";
  }

  Serial.print(F("UID in hexadecimal: "));
  Serial.println(uidStr);

  client.publish("EIASidCard", uidStr.c_str()); // Publicar UID en MQTT

  waitingForAccessResponse = true;
  waitingStartTime = millis();

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

// Función para abrir la puerta
void openDoor() {
  blinkLED(GREEN_LED_PIN, 2); // Parpadea el LED verde dos veces
  servo.write(90);            // Mueve el servo a 90 grados
}

// Función para cerrar la puerta
void closeDoor() {
  servo.write(0); // Mueve el servo a 0 grados
}

// Función para hacer parpadear un LED
void blinkLED(int ledPin, int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(ledPin, HIGH);
    delay(200);
    digitalWrite(ledPin, LOW);
    delay(200);
  }
}
