#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <PubSubClient.h>

char jsonOutput[2100];
//=========================================== U M I D A D E  DO  S O L O =============================================
#define sensorUmidadeSolo 36
#define pinSensorD 8

float Umidade;
float PorcentagemUmidade;

float loopUmidadeSolo() {
  Umidade = analogRead(sensorUmidadeSolo);
  if (Umidade > 4095) {
    Umidade = 4095;
  } else if (Umidade < 2160) {
    Umidade = 2160;
  }
  
  PorcentagemUmidade = map(Umidade, 2160, 4095, 100, 0);

  if (PorcentagemUmidade > 100) {
    PorcentagemUmidade = 100;
  } else if (PorcentagemUmidade < 0) {
    PorcentagemUmidade = 0;
  }

  return PorcentagemUmidade;
}
void statusUmidadeSolo() {
  Serial.print("Umidade do solo:");
  Serial.print(PorcentagemUmidade);
  Serial.println("%");
  Serial.print("amperagem recebida:");
  Serial.println(Umidade);
}
//=========================================== T E M P E R A T U R A  DO  S O L O =============================================
#define ONE_WIRE_BUS 25

float temperaturaSolo;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setupTempSolo() {
  sensors.begin();
}
float loopTempSolo() {
  sensors.requestTemperatures();
  temperaturaSolo = sensors.getTempCByIndex(0);

  return temperaturaSolo;
}
void statusTempSolo() {
  Serial.print("Temperatura do solo: ");
  Serial.println(temperaturaSolo);
}
//=========================================== D H T =============================================
#define DHTPIN 26  //Pino one está o DHT22

DHT dht(26, DHT11); //Objeto que faz a leitura da temperatura e umidade

float temperaturaAr = 0;
float umidadeAr = 0;

void setupDHT() {
  dht.begin();
}
float loopDHT() {
  temperaturaAr = dht.readTemperature();
  umidadeAr = dht.readHumidity();

  return temperaturaAr, umidadeAr;
}
void statusDHT() {
  Serial.print("Temperatura do ar: ");
  Serial.print(temperaturaAr);
  Serial.println(" C");
  Serial.print("Umidade do ar: ");
  Serial.print(umidadeAr);
  Serial.println("%");
}

//=========================================== W I F I =============================================
const char* SSID = "Lorena";
const char* PASSWD = "L12345678";
void setupWIFI() {
  // Inicializa a conexão com a rede Wi-Fi
  WiFi.begin("Lorena", "L12345678");


  // Espera a conexão estar estabelecida
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Wifi Conectado!");
  }
  
}

//=========================================== H T T P =============================================
const char* URL = "http://192.168.1.41:3333/api-hortas/createLeitura";
// const char* URL = "https://api-hortas-service.onrender.com/api-hortas/createLeitura";

void postHTTP() {
   if(WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    http.begin(URL);
    http.addHeader("Content-Type", "application/json");

    DynamicJsonDocument doc(1024);

    doc["temperaturaSolo"] = temperaturaSolo;
    doc["temperaturaAmbiente"] = temperaturaAr;
    doc["umidadeAtmosfera"]   = umidadeAr;
    doc["umidadeSolo"] = PorcentagemUmidade;
    doc["phSolo"] = "0";
    doc["condutividadeEletricaSolo"] = 0;
    doc["luminosidade"] = 0;

    serializeJson(doc, jsonOutput);

    Serial.println(jsonOutput);

    int statusCode = http.POST(jsonOutput);

    if (statusCode > 0) {
      Serial.print("HTTP status code: ");
      Serial.println(statusCode);

      String response = http.getString();
      Serial.print("HTTP response: ");
      Serial.println(response);
    } else {
      Serial.println("HTTP ERRo: ");
      Serial.println(http.GET());
      Serial.println(http.getString());
    }

    http.end();
  }
}

//=========================================== M Q T T  =============================================
const char* mqtt_server = "maqiatto.com";
const char* mqtt_user = "lucashouse0@gmail.com";
const char* mqtt_password = "hortas";
const char* mqtt_topic = "lucashouse0@gmail.com/tempsolo";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(256)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  // if ((char)payload[0] == '1') {
  //   digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
  //   // but actually the LED is on; this is because
  //   // it is active low on the ESP-01)
  // } else {
  //   digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  // }

}
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(mqtt_topic, "hellosss world");
      // ... and resubscribe
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void setupMQTT() {
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}
void loopMQTT() {
  float tempSolo = loopTempSolo();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    // snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
    snprintf (msg, MSG_BUFFER_SIZE, "%f, %f, %f, %f ", temperaturaSolo, PorcentagemUmidade, temperaturaAr, umidadeAr);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish(mqtt_topic, msg);
  }
}
//========================================================================================


void setup() {
  delay(100);
  setupDHT();
  setupTempSolo();
  setupWIFI();
  setupMQTT();
  delay(2000);

  
  Serial.begin(115200);
}



void loop() {
  
  loopDHT();
  loopTempSolo();
  loopUmidadeSolo();
  statusUmidadeSolo();
  Serial.println("----------------------------");
  loopMQTT();
  postHTTP();
  delay(10000);

}
