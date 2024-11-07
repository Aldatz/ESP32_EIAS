#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <WiFiClientSecure.h> // Incluye esta biblioteca para conexión segura a WiFi
#include <PubSubClient.h>     // Biblioteca para gestionar la comunicación MQTT
#include <ESP32Servo.h>       // Biblioteca para servo

#define SS_PIN 5   // Pin para SS (Slave Select) del lector RFID
#define RST_PIN 22 // Pin de reset del lector RFID

// Servo y leds
#define GREEN_LED_PIN 14 // Pin del LED verde
#define RED_LED_PIN 15   // Pin del LED rojo
#define SERVO_PIN 27     // Pin del Servo

// Credenciales WiFi y configuración MQTT
const char *ssid = "AEG-IKASLE";        // Nombre de la red WiFi
const char *password = "Ea25dneAEG";    // Contraseña de la red WiFi
const char *mqttServer = "10.80.128.2"; // Dirección del servidor MQTT
const int mqttPort = 8883;              // Puerto seguro para el servidor MQTT

// Certificados de seguridad para la conexión SSL
const char *certificate_request =
    "-----BEGIN CERTIFICATE-----\n"
    "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\n"
    "-----END CERTIFICATE-----\n";

// Definición del certificado y clave privada
const char *certificate =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIEeDCCAmACFF4Lkq2z01fU8SWRQfeG7x/CXwBpMA0GCSqGSIb3DQEBCwUAMIGJ\n"
    "MQswCQYDVQQGEwJFVTERMA8GA1UECAwIR0lQVVpLT0ExETAPBgNVBAcMCERPTk9T\n"
    "VElBMQwwCgYDVQQKDANBRUcxEDAOBgNVBAsMB0tBT1RJS0ExEDAOBgNVBAMMB0tB\n"
    "T1RJS0ExIjAgBgkqhkiG9w0BCQEWE29za2FyLmNhbHZvQGFlZy5ldXMwHhcNMjQx\n"
    "MTA2MDg0MDQ3WhcNMjUxMTA2MDg0MDQ3WjBnMQswCQYDVQQGEwJFUzEKMAgGA1UE\n"
    "CAwBRDEKMAgGA1UEBwwBRDENMAsGA1UECgwEZWlhczENMAsGA1UECwwEZWlhczEN\n"
    "MAsGA1UEAwwEZWlhczETMBEGCSqGSIb3DQEJARYEZWlhczCCASIwDQYJKoZIhvcN\n"
    "AQEBBQADggEPADCCAQoCggEBAK8vxkhnokbWk65fnPQSP6Cxihyt7U78sYlOtouI\n"
    "H+d74ZcIiral2cOspC8ytwStYBcM5VAxsf+HRocpR7xyfckcl6nTEnG2IutxhBiI\n"
    "yGdDNyq9NUYuMaEOtOTRYZ+uuXxTqqRsVg0QRvZ76Ef6o4zj/m4AOoot0hOVUtkz\n"
    "D/yO+foeX904LGDTHcTOaIconWIiwNDrZJJMLc4knObauHwRBaE2EFSwGBIa1oEg\n"
    "3aMYGkK+whO3SrfxQhY9dnqOjlpA39yF695J3WKMy/FITzJgPvy8YdIdkHKgmIMT\n"
    "a7CkaSmUmGeZxoRmFns7F3atSaSbVIcPJJ2wAzyAcOaG2j0CAwEAATANBgkqhkiG\n"
    "9w0BAQsFAAOCAgEAXvUF6GmJMQDne6Vo/9I9oekK8lCulP7doXu1myuS20Hk5XR4\n"
    "Oknt4I/IiYgTTAfRWH1zn0sCXmMvYi4+gIj14vVbZkjfTkOUpC/rbj+ULX+BcBDM\n"
    "y09ta7jIPl26jYYSS3WJFlZn0HbApG8KGtwZIwcjxFpXGivAxxWq+X75fNoCIT7V\n"
    "7l0mtGsJC/x5xU+Fdi/opNUUxU56m3sOiHaTXNfBhcsz7hOa1loj0HZ+DbOg903G\n"
    "7pN7s9+E15BjbKr46C2pvA3fZ5EZbctD9D1sxmPhmHf5sVA+6DYlEebAlu6+WHVB\n"
    "a6NWNoZzKNwySX4WA0mL5LSM2H6+TdKDKJTPrASYhnmUAHm39jWGbHoJojF7QSXb\n"
    "wwe683aPGWv/eNSN7G3b1pJQVDFXGC1PznNf6bGfoiBfXh9NMv+DPiGSYdYuL0Ur\n"
    "hrCXGIZ0UVN+BpB9uMSrVSXk9woV7O5FRjG3uOXXeDna/KGxmbUlngQGudERMbyu\n"
    "gZEglMLobWqi9AiXDq5mQTGBGlfvhxdj1acBVHN9JOa+gxP57792wOnanGov2np8\n"
    "85WwrLCjIAPAhHbWTKbjNBrXsEWwE9KlhDL7xq8WHAfSa1iPB7IGeXq+JVHoJxS7\n"
    "npxC9Qj1YUC/mwTpTilzK8TptVoeUCvbBy/d/r1TgY1lNiDTYqr6ikRqZS0=\n"
    "-----END CERTIFICATE-----\n";

const char *private_key =
    "-----BEGIN PRIVATE KEY-----\n"
    "MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQCvL8ZIZ6JG1pOu\n"
    "X5z0Ej+gsYocre1O/LGJTraLiB/ne+GXCIq2pdnDrKQvMrcErWAXDOVQMbH/h0aH\n"
    "KUe8cn3JHJep0xJxtiLrcYQYiMhnQzcqvTVGLjGhDrTk0WGfrrl8U6qkbFYNEEb2\n"
    "e+hH+qOM4/5uADqKLdITlVLZMw/8jvn6Hl/dOCxg0x3EzmiHKJ1iIsDQ62SSTC3O\n"
    "JJzm2rh8EQWhNhBUsBgSGtaBIN2jGBpCvsITt0q38UIWPXZ6jo5aQN/cheveSd1i\n"
    "jMvxSE8yYD78vGHSHZByoJiDE2uwpGkplJhnmcaEZhZ7Oxd2rUmkm1SHDySdsAM8\n"
    "gHDmhto9AgMBAAECggEAD6koWKCrE8TAPv5dJunatkrTcJeRIgEvqHlY6EWq/yPx\n"
    "RByHX1Hu6FyD8OmKYuinhmf+DUU5XFaDf4xj1VIxhyD/OIyd7CcUHWgbHvZbXp/W\n"
    "TAPOF2VZxRluImZ1YQkFm+T+2Y61wpqJ6CRQ3N5ocIKROX7bvNk2cSRlhkqCS1l2\n"
    "u63tZqeKp/XSjkI5qN4Owh8Mo7Q1qPNfn4VbKpiBI+pLDQ6Ayj/3GkO3Ut5PmbOE\n"
    "y4ogFHsKCHpzP8dkVKrtVtgeGVj/m7m9NZX3DfwCkpurTVAxw6qcwiT8tdqpB6+h\n"
    "QbEZVfuljPH4pYn/yaYu8fxUNcxaVf4uX4fDaEFuIQKBgQDmE+TDvSST0BazfNLe\n"
    "f4kxVqgICNAC9Fmf4BgtStSnTXnGbbjVkHkhEc5oNlMP1EfBi5tp46w/TQLZ1ThI\n"
    "jP3mShXO3nO/uPAczFgD02ANAKVsgdeqi+3L7E2UDUgpiqxL21+Jz1l7D/LA2MkW\n"
    "MFrYiGh2fN0kGo6/rvU4KR6uJwKBgQDC7KnGGzMdxbubKKmCyWl9DJ+2rjOsyBVe\n"
    "nJ1jnBip+6MPkmMbJ+zMI9TPLyzHSDXnQ3p2MeNCQzv3A2GEyv2EckZZhaBJxO12\n"
    "Rl22HcX6ArtDWPL40vzUKfZQxlarCFf4qOAYp+FoU5ywT3txjzy+H8jLTaMjt083\n"
    "TTA0qSdW+wKBgGaiSzNyBYYWlnnc3eg5NkcXI//phnk67VwfSEm0DubO7dqxiXlj\n"
    "aFVzLwNX8HeoN5ZEI8D5uhXG0dXAgsoCcySuucm9fB7zbdE66qxSN4+1URX1WfKP\n"
    "VzXXpMe9oQo+/RsdsiG0qHL4K08RpapLScs8FnV8v91iJ3mkn2k9Fp2TAoGBAITg\n"
    "2ItYF2yAUvferE+gBe1dIbNi5Y8vhoHMue4hhx65j9sZq73jZaVz96qVOi1PqI9S\n"
    "jLTuJUISm1o0T9I6nsAfNp3oXfBr8oTiSxfPDBr2qbmrdVbyXbA0Otmtm39UXMQC\n"
    "O2ITb7Gy7emzBS/QuVPSTwtojYHXgITudaIwfcTpAoGAXj9OwUhLmKvVYuHcX5tI\n"
    "62AGrNwtLwanJ7gw6UhjqSIkcVNuHU2iXDBkK5QAPdb9WZ7/UsOJiHfeLfucXmaV\n"
    "W1sN32KOjLvm5Y/19FwpXut4ubhUJVHCzc1Tp1yJyFOSk5GcYpIP2croQaxjiMZD\n"
    "tDqv9NapB7SMBxtspozpLe8=\n"
    "-----END PRIVATE KEY-----\n";

// Declaración de objetos de cliente seguro y MQTT
WiFiClientSecure ESP_EIAS;
PubSubClient client(ESP_EIAS);

// Configuración de RFID
MFRC522 rfid = MFRC522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

bool waitingForAccessResponse = false; // Bandera para esperar respuesta de acceso
unsigned long waitingStartTime = 0;    // Tiempo de inicio de espera
String uidStr = "";                    // Almacena el UID de la tarjeta

// Función para reconectar a WiFi y MQTT en caso de desconexión
void reconnect()
{
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Reconnecting to WiFi...");
    WiFi.begin(ssid, password);
    delay(5000);
  }

  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32EIAS"))
    {
      Serial.println("connected");
      client.subscribe("EIASOpenDoor"); // Suscribirse a temas para recibir comandos
      client.subscribe("EIASOpenDoorDenied");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// Función para manejar mensajes MQTT recibidos
void callback(char *topic, byte *payload, unsigned int length)
{
  String receivedMessage = "";
  for (unsigned int i = 0; i < length; i++)
  {
    receivedMessage += (char)payload[i];
  }

  Serial.print("Message received on topic ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(receivedMessage);

  if (strcmp(topic, "EIASOpenDoor") == 0)
  {
    Serial.println("Access granted. Opening door...");
    // led y abrir puerta
    openDoor()

        delay(1000);
    Serial.println("The door is open");
    client.publish("EIASdoorOpened", uidStr.c_str()); // Publicar evento de apertura
  }
  else if (strcmp(topic, "EIASOpenDoorDenied") == 0)
  {
    Serial.println("Access denied. Showing message or alert...");
    blinkLED(redLedPin, 2); // Parpadea el LED rojo dos veces
    delay(200);             // Pequeña demora para evitar rebotes
  }

  waitingForAccessResponse = false;
}

// Configuración inicial del sistema
void setup()
{

  // config servo y leds
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  servo.attach(SERVO_PIN);
  servo.write(0);                   // Posición inicial del servo a 0 grados
  digitalWrite(GREEN_LED_PIN, LOW); // LED verde apagado inicialmente
  digitalWrite(RED_LED_PIN, LOW);   // LED rojo apagado inicialmente

  Serial.begin(115200);
  Serial.println(F("Initialize System"));

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print('.');
  }

  Serial.println();
  Serial.print("Connected to:\t");
  Serial.println(WiFi.SSID());
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());

  SPI.begin();     // Inicializar comunicación SPI
  rfid.PCD_Init(); // Inicializar el lector RFID
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));

  ESP_EIAS.setCACert(certificate_request); // Cargar certificados para conexión segura
  ESP_EIAS.setCertificate(certificate);
  ESP_EIAS.setPrivateKey(private_key);

  client.setServer(mqttServer, mqttPort); // Configurar servidor MQTT
  client.setCallback(callback);           // Configurar función de manejo de mensajes MQTT

  reconnect(); // Conectar a WiFi y MQTT
}

// Función principal del sistema
void loop()
{
  if (!client.connected())
  {
    reconnect(); // Intentar reconexión si se pierde la conexión
  }
  client.loop();

  if (waitingForAccessResponse && (millis() - waitingStartTime > 10000))
  {
    Serial.println("Waiting time exceeded. Restarting card reading...");
    waitingForAccessResponse = false;
  }

  readRFID(); // Leer tarjeta RFID si es posible
}

// Función para leer tarjeta RFID
void readRFID(void)
{
  if (waitingForAccessResponse)
    return;

  for (byte i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF; // Configuración clave por defecto para RFID
  }

  if (!rfid.PICC_IsNewCardPresent())
    return; // Detectar nueva tarjeta
  if (!rfid.PICC_ReadCardSerial())
    return; // Leer serial de tarjeta

  uidStr = "";
  for (byte i = 0; i < rfid.uid.size; i++)
  {
    uidStr += String(rfid.uid.uidByte[i], HEX);
    if (i < rfid.uid.size - 1)
      uidStr += ":";
  }

  Serial.print(F("UID in hexadecimal: "));
  Serial.println(uidStr);

  client.publish("EIASidCard", uidStr.c_str()); // Publicar UID en el tema MQTT

  waitingForAccessResponse = true;
  waitingStartTime = millis();

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

// Acción al abrir la puerta
void openDoor()
{
  blinkLED(GREEN_LED_PIN, 2); // Parpadea el LED verde dos veces
  servo.write(90);            // Mueve el servo a 90 grados
}

// Acción al abrir la puerta
void closeDoor()
{
  servo.write(0); // Mueve el servo a 0 grados
}

// Función para hacer parpadear un LED
void blinkLED(int ledPin, int times)
{
  for (int i = 0; i < times; i++)
  {
    digitalWrite(ledPin, HIGH);
    delay(200);
    digitalWrite(ledPin, LOW);
    delay(200);
  }
}