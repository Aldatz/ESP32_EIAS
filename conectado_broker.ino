#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>  // Librería para controlar el servo en ESP32

#define SS_PIN 5       
#define RST_PIN 22    
#define RED_LED_PIN 26      // Pin para la luz roja
#define GREEN_LED_PIN 25    // Pin para la luz verde
#define BUZZER_PIN 32
#define SERVO_PIN 27        // Pin para el servo

const char* ssid = "AEG-IKASLE";
const char* password = "Ea25dneAEG";
const char* mqttServer = "10.80.128.2";
const int mqttPort = 8883;

Servo servoMotor;

const char* certificate_request = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIF9TCCA92gAwIBAgIUTn+K4jmgqp3YxzM+grGiA87ZO1YwDQYJKoZIhvcNAQEL\n" \
"BQAwgYkxCzAJBgNVBAYTAkVVMREwDwYDVQQIDAhHSVBVWktPQTERMA8GA1UEBwwI\n" \
"RE9OT1NUSUExDDAKBgNVBAoMA0FFRzEQMA4GA1UECwwHS0FPVElLQTEQMA4GA1UE\n" \
"AwwHS0FPVElLQTEiMCAGCSqGSIb3DQEJARYTb3NrYXIuY2Fsdm9AYWVnLmV1czAe\n" \
"Fw0yNDExMDUxMTA3NDhaFw0zNDExMDMxMTA3NDhaMIGJMQswCQYDVQQGEwJFVTER\n" \
"MA8GA1UECAwIR0lQVVpLT0ExETAPBgNVBAcMCERPTk9TVElBMQwwCgYDVQQKDANB\n" \
"RUcxEDAOBgNVBAsMB0tBT1RJS0ExEDAOBgNVBAMMB0tBT1RJS0ExIjAgBgkqhkiG\n" \
"9w0BCQEWE29za2FyLmNhbHZvQGFlZy5ldXMwggIiMA0GCSqGSIb3DQEBAQUAA4IC\n" \
"DwAwggIKAoICAQDRHYLECpA1vHB4oBoqjGn2rJ1vwParc3FIGtBeDDTkKvQWT0rX\n" \
"p86a30WGIJW1/AugLqJNyNuU3aiFS/orK1CM2lPe7QP9TwbbW/Jyu5Hnff6f91ZW\n" \
"fmzUDsPTI8pCUE0GAz2bsxzA59XGSaMIllbX0cmMUfETU03QdIZOmx7v+fqkX3vY\n" \
"sRrDSZ1tuRo+t9MRfidEV17S61/kwDt9WfE56mtREXGr/ogTYnACiA3a5mcsUe84\n" \
"tYOKXsMFDV3Xh4iDotPS2pqPHnWhGbculHeFO3NVgsQkt83hHC6df98Tal1OI8cG\n" \
"BUTfno6viAa+fSRXVhxRlgVKPcSdqIH9PdU27tEemio7qPrbS3yvInSUt+XtXE0U\n" \
"UiPqHQAx8e1bosGFqaOqnGR98YtePo5AwRrl2nTJxbvyhoPg8D0DVAFjzv8UgCHN\n" \
"Obo52y1Qb8PqfucFQzltbOVihRmskQbRBg6XORg+hKstDWcZBm6PdgusK2l5lC+H\n" \
"gChnlaA22XcSjfqSv7iP5nnU15fXm1L33iZIY9qzKfBg2Zzd8Le4mvn7wGbnKY5/\n" \
"AfEyLyxVl/5JHeCnHDGjQayhh6ojZoMR+XgPYmdDqT7OcDAcLemZVPDQvHpeScSq\n" \
"ECrPN1J7tbwQsueF9T+DwvbvDPeOcfRAZxgQUico+6q+STWmCQDHaVlAtQIDAQAB\n" \
"o1MwUTAdBgNVHQ4EFgQU4olSyYbprVwO0bkiAM/9eRXSiXkwHwYDVR0jBBgwFoAU\n" \
"4olSyYbprVwO0bkiAM/9eRXSiXkwDwYDVR0TAQH/BAUwAwEB/zANBgkqhkiG9w0B\n" \
"AQsFAAOCAgEAkQMv07kbzRaG55T6Br0WbtUsDtuDc9m6JWBwaItksPcgDajTgZJc\n" \
"71x53Ie94X0JT/wW0MsprD+cUqR6MkHAJpKjSxon7+BxIxF3N/YQbs9dCVDddMqn\n" \
"EJ1PhFrmV9Mt/zhhV3p8u8BvuTdgKHtkLhHrwcmSTIl9Ed6YJ+a/dzqkbnTdiEaU\n" \
"WZZq7+20mM7xXUxc6uoCqAnyJtDKUV1Gltiff8arEiqJrSa5J6thvD2b3G5ErFID\n" \
"lXcj2pEAdH+Ml+Ggp6v0181aeGS/bYQTJXhloYOyudnoSt1X35dvT3HSCsD5T3iT\n" \
"cPtu5sX6HJxSrzt5c0Hgm2XHRxGW25XVao1dUn4GqNJ/qMrtqmT9SH4E8Nhse6d4\n" \
"H1E362x4RlmsaQbmYNh9toOXqLtBXTjw24qrnprMdSfymiJvJg+u9Itp++m+h87b\n" \
"VJPuiFmLeVhfXDHOxl/gvoCgDLfIbwfDQaefEz6WNO898jsciiZn66cCzGu4b+eC\n" \
"UGry77DBM08LgoQDBuieDHiMaJ3dgpmvyoWTaVt/oOo2rS1r+o7n9UFW3LgmUiGh\n" \
"XKvpsnMBXXMoKEljtahuYWOPqoi+B7wM9YB6wGYnoF8ML3b9XFo9EuJUoRro4JX1\n" \
"+F7Gra0qxN7kM9oc6BNlY3GQZwQpG4uRZXwBcp2doptJvSpX/o5F1zY=\n" \
"-----END CERTIFICATE-----\n";

const char* certificate = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIEeDCCAmACFF4Lkq2z01fU8SWRQfeG7x/CXwBpMA0GCSqGSIb3DQEBCwUAMIGJ\n" \
"MQswCQYDVQQGEwJFVTERMA8GA1UECAwIR0lQVVpLT0ExETAPBgNVBAcMCERPTk9T\n" \
"VElBMQwwCgYDVQQKDANBRUcxEDAOBgNVBAsMB0tBT1RJS0ExEDAOBgNVBAMMB0tB\n" \
"T1RJS0ExIjAgBgkqhkiG9w0BCQEWE29za2FyLmNhbHZvQGFlZy5ldXMwHhcNMjQx\n" \
"MTA2MDg0MDQ3WhcNMjUxMTA2MDg0MDQ3WjBnMQswCQYDVQQGEwJFUzEKMAgGA1UE\n" \
"CAwBRDEKMAgGA1UEBwwBRDENMAsGA1UECgwEZWlhczENMAsGA1UECwwEZWlhczEN\n" \
"MAsGA1UEAwwEZWlhczETMBEGCSqGSIb3DQEJARYEZWlhczCCASIwDQYJKoZIhvcN\n" \
"AQEBBQADggEPADCCAQoCggEBAK8vxkhnokbWk65fnPQSP6Cxihyt7U78sYlOtouI\n" \
"H+d74ZcIiral2cOspC8ytwStYBcM5VAxsf+HRocpR7xyfckcl6nTEnG2IutxhBiI\n" \
"yGdDNyq9NUYuMaEOtOTRYZ+uuXxTqqRsVg0QRvZ76Ef6o4zj/m4AOoot0hOVUtkz\n" \
"D/yO+foeX904LGDTHcTOaIconWIiwNDrZJJMLc4knObauHwRBaE2EFSwGBIa1oEg\n" \
"3aMYGkK+whO3SrfxQhY9dnqOjlpA39yF695J3WKMy/FITzJgPvy8YdIdkHKgmIMT\n" \
"a7CkaSmUmGeZxoRmFns7F3atSaSbVIcPJJ2wAzyAcOaG2j0CAwEAATANBgkqhkiG\n" \
"9w0BAQsFAAOCAgEAXvUF6GmJMQDne6Vo/9I9oekK8lCulP7doXu1myuS20Hk5XR4\n" \
"Oknt4I/IiYgTTAfRWH1zn0sCXmMvYi4+gIj14vVbZkjfTkOUpC/rbj+ULX+BcBDM\n" \
"y09ta7jIPl26jYYSS3WJFlZn0HbApG8KGtwZIwcjxFpXGivAxxWq+X75fNoCIT7V\n" \
"7l0mtGsJC/x5xU+Fdi/opNUUxU56m3sOiHaTXNfBhcsz7hOa1loj0HZ+DbOg903G\n" \
"7pN7s9+E15BjbKr46C2pvA3fZ5EZbctD9D1sxmPhmHf5sVA+6DYlEebAlu6+WHVB\n" \
"a6NWNoZzKNwySX4WA0mL5LSM2H6+TdKDKJTPrASYhnmUAHm39jWGbHoJojF7QSXb\n" \
"wwe683aPGWv/eNSN7G3b1pJQVDFXGC1PznNf6bGfoiBfXh9NMv+DPiGSYdYuL0Ur\n" \
"hrCXGIZ0UVN+BpB9uMSrVSXk9woV7O5FRjG3uOXXeDna/KGxmbUlngQGudERMbyu\n" \
"gZEglMLobWqi9AiXDq5mQTGBGlfvhxdj1acBVHN9JOa+gxP57792wOnanGov2np8\n" \
"85WwrLCjIAPAhHbWTKbjNBrXsEWwE9KlhDL7xq8WHAfSa1iPB7IGeXq+JVHoJxS7\n" \
"npxC9Qj1YUC/mwTpTilzK8TptVoeUCvbBy/d/r1TgY1lNiDTYqr6ikRqZS0=\n" \
"-----END CERTIFICATE-----\n";

const char* private_key = \
"-----BEGIN PRIVATE KEY-----\n" \
"MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQCvL8ZIZ6JG1pOu\n" \
"X5z0Ej+gsYocre1O/LGJTraLiB/ne+GXCIq2pdnDrKQvMrcErWAXDOVQMbH/h0aH\n" \
"KUe8cn3JHJep0xJxtiLrcYQYiMhnQzcqvTVGLjGhDrTk0WGfrrl8U6qkbFYNEEb2\n" \
"e+hH+qOM4/5uADqKLdITlVLZMw/8jvn6Hl/dOCxg0x3EzmiHKJ1iIsDQ62SSTC3O\n" \
"JJzm2rh8EQWhNhBUsBgSGtaBIN2jGBpCvsITt0q38UIWPXZ6jo5aQN/cheveSd1i\n" \
"jMvxSE8yYD78vGHSHZByoJiDE2uwpGkplJhnmcaEZhZ7Oxd2rUmkm1SHDySdsAM8\n" \
"gHDmhto9AgMBAAECggEAD6koWKCrE8TAPv5dJunatkrTcJeRIgEvqHlY6EWq/yPx\n" \
"RByHX1Hu6FyD8OmKYuinhmf+DUU5XFaDf4xj1VIxhyD/OIyd7CcUHWgbHvZbXp/W\n" \
"TAPOF2VZxRluImZ1YQkFm+T+2Y61wpqJ6CRQ3N5ocIKROX7bvNk2cSRlhkqCS1l2\n" \
"u63tZqeKp/XSjkI5qN4Owh8Mo7Q1qPNfn4VbKpiBI+pLDQ6Ayj/3GkO3Ut5PmbOE\n" \
"y4ogFHsKCHpzP8dkVKrtVtgeGVj/m7m9NZX3DfwCkpurTVAxw6qcwiT8tdqpB6+h\n" \
"QbEZVfuljPH4pYn/yaYu8fxUNcxaVf4uX4fDaEFuIQKBgQDmE+TDvSST0BazfNLe\n" \
"f4kxVqgICNAC9Fmf4BgtStSnTXnGbbjVkHkhEc5oNlMP1EfBi5tp46w/TQLZ1ThI\n" \
"jP3mShXO3nO/uPAczFgD02ANAKVsgdeqi+3L7E2UDUgpiqxL21+Jz1l7D/LA2MkW\n" \
"MFrYiGh2fN0kGo6/rvU4KR6uJwKBgQDC7KnGGzMdxbubKKmCyWl9DJ+2rjOsyBVe\n" \
"nJ1jnBip+6MPkmMbJ+zMI9TPLyzHSDXnQ3p2MeNCQzv3A2GEyv2EckZZhaBJxO12\n" \
"Rl22HcX6ArtDWPL40vzUKfZQxlarCFf4qOAYp+FoU5ywT3txjzy+H8jLTaMjt083\n" \
"TTA0qSdW+wKBgGaiSzNyBYYWlnnc3eg5NkcXI//phnk67VwfSEm0DubO7dqxiXlj\n" \
"aFVzLwNX8HeoN5ZEI8D5uhXG0dXAgsoCcySuucm9fB7zbdE66qxSN4+1URX1WfKP\n" \
"VzXXpMe9oQo+/RsdsiG0qHL4K08RpapLScs8FnV8v91iJ3mkn2k9Fp2TAoGBAITg\n" \
"2ItYF2yAUvferE+gBe1dIbNi5Y8vhoHMue4hhx65j9sZq73jZaVz96qVOi1PqI9S\n" \
"jLTuJUISm1o0T9I6nsAfNp3oXfBr8oTiSxfPDBr2qbmrdVbyXbA0Otmtm39UXMQC\n" \
"O2ITb7Gy7emzBS/QuVPSTwtojYHXgITudaIwfcTpAoGAXj9OwUhLmKvVYuHcX5tI\n" \
"62AGrNwtLwanJ7gw6UhjqSIkcVNuHU2iXDBkK5QAPdb9WZ7/UsOJiHfeLfucXmaV\n" \
"W1sN32KOjLvm5Y/19FwpXut4ubhUJVHCzc1Tp1yJyFOSk5GcYpIP2croQaxjiMZD\n" \
"tDqv9NapB7SMBxtspozpLe8=\n" \
"-----END PRIVATE KEY-----\n";

WiFiClientSecure ESP_EIAS;
PubSubClient client(ESP_EIAS);

MFRC522 rfid = MFRC522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

bool waitingForAccessResponse = false;
unsigned long waitingStartTime = 0;
String uidStr = "";

void reconnect() {
  // Conectar a WiFi y MQTT
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reconnecting to WiFi...");
    WiFi.begin(ssid, password);
    delay(5000);
  }
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32EIAS")) {
      Serial.println("connected");
      client.subscribe("EIASOpenDoor");
      client.subscribe("EIASOpenDoorDenied");
      client.subscribe("EIASAcolyteInside");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
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
    digitalWrite(GREEN_LED_PIN, HIGH); // Enciende la luz verde
    digitalWrite(RED_LED_PIN, LOW);    // Apaga la luz roja
    tone(BUZZER_PIN, 1000, 500);       // Tono alto (1 kHz) durante 0.5 segundos para acceso permitido
    servoMotor.attach(SERVO_PIN);      // Activa el control del servo
    servoMotor.write(180);             // Gira el servo a 180 grados para abrir la puerta
    delay(4000);                       // Espera 2 segundos con la puerta abierta

    digitalWrite(GREEN_LED_PIN, LOW);  // Apaga la luz verde después de abrir
    client.publish("EIASdoorOpened", uidStr.c_str());
  } else if (strcmp(topic, "EIASOpenDoorDenied") == 0) {
    Serial.println("Access denied. Showing message or alert...");
    digitalWrite(GREEN_LED_PIN, LOW);  // Apaga la luz verde
    digitalWrite(RED_LED_PIN, HIGH);   // Enciende la luz roja
    tone(BUZZER_PIN, 500, 500);        // Tono bajo (500 Hz) durante 0.5 segundos para acceso denegado
    delay(1000);
    digitalWrite(RED_LED_PIN, LOW);    // Apaga la luz roja después de mostrar el error
  }

  if (strcmp(topic, "EIASAcolyteInside") == 0) {
    Serial.println("Mensaje recibido en EIASAcolyteInside. Cerrando la puerta.");
    servoMotor.attach(SERVO_PIN);      // Activa el control del servo
    servoMotor.write(55);             // Gira el servo a 180 grados para abrir la puerta
    delay(2000);                       // Espera 2 segundos con la puerta abierta
    servoMotor.detach();
  }

  waitingForAccessResponse = false;
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("Initialize System"));

  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(RED_LED_PIN, LOW);    // Apaga las luces al iniciar
  digitalWrite(GREEN_LED_PIN, LOW);

  servoMotor.attach(SERVO_PIN, 1000, 2000);  // Ajuste del rango de pulsos entre 1000 y 2000 microsegundos
  servoMotor.write(20);
  delay(1000);
  servoMotor.detach();

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

  SPI.begin();
  rfid.PCD_Init();
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));

  ESP_EIAS.setCACert(certificate_request);
  ESP_EIAS.setCertificate(certificate);
  ESP_EIAS.setPrivateKey(private_key);

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  reconnect();
}

void loop() {
  readRFID();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (waitingForAccessResponse && (millis() - waitingStartTime > 10000)) {
    Serial.println("Waiting time exceeded. Restarting card reading...");
    waitingForAccessResponse = false;
  }
}

void readRFID(void) {
  if (waitingForAccessResponse) return;

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  uidStr = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uidStr += String(rfid.uid.uidByte[i], HEX);
    if (i < rfid.uid.size - 1) uidStr += ":";
  }

  Serial.print(F("UID in hexadecimal: "));
  Serial.println(uidStr);

  client.publish("EIASidCard", uidStr.c_str());

  waitingForAccessResponse = true;
  waitingStartTime = millis();

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}