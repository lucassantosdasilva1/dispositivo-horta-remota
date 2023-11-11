#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>

char jsonOutput[2100];
//=========================================== U M I D A D E  DO  S O L O =============================================
#define sensorUmidadeSolo 36
#define pinSensorD 8

float Umidade;
float PorcentagemUmidade;

float loopUmidadeSolo() {
  Umidade = analogRead(sensorUmidadeSolo);
  PorcentagemUmidade = map(Umidade, 1660, 4095, 100, 0);
  Serial.print("Umidade do solo:");
  Serial.print(PorcentagemUmidade);
  Serial.println("%");
  Serial.print("amperagem recebida:");
  Serial.println(Umidade);

  return PorcentagemUmidade;
}
//=========================================== T E M P E R A T U R A  DO  S O L O =============================================
#define ONE_WIRE_BUS 25

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setupTempSolo() {
  sensors.begin();
}
float loopTempSolo() {
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  Serial.print("Temperatura do solo: ");
  Serial.println(tempC);

  return tempC;
}
//=========================================== D H T =============================================
#define DHTPIN 26  //Pino one está o DHT22

DHT dht(26, DHT11); //Objeto que faz a leitura da temperatura e umidade

float temperature = 0;
float humidity = 0;

void setupDHT() {
  dht.begin();
}
float loopDHT() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  Serial.println("----------------------------");

  Serial.print("Temperatura do ar: ");
  Serial.print(temperature);
  Serial.println(" C");
  Serial.print("Umidade do ar: ");
  Serial.print(humidity);
  Serial.println("%");
  
  Serial.println("----------------------------");

  return temperature, humidity;
}

//=========================================== W I F I =============================================
void setupWIFI() {
  // Inicializa a conexão com a rede Wi-Fi
  WiFi.begin("Lorena", "L12345678");


  // Espera a conexão estar estabelecida
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }
  
}

//=========================================== H T T P =============================================
void postHTTP() {
   if(WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    http.begin("https://api-hortas-service.onrender.com/api-hortas/createLeitura");
    http.addHeader("Content-Type", "application/json");

    DynamicJsonDocument doc(1024);

    doc["temperaturaSolo"] = 0;
    doc["temperaturaAmbiente"] = 0;
    doc["umidadeAtmosfera"]   = 0;
    doc["umidadeSolo"] = 0;
    doc["phSolo"] = "0";
    doc["condutividadeEletricaSolo"] = 0;
    doc["luminosidade"] = 0;

    serializeJson(doc, jsonOutput);

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

void setup() {
  delay(100);
  setupWIFI();
  setupDHT();
  setupTempSolo();
  delay(2000);

  
  Serial.begin(115200);
}

void loop() {
  
  loopDHT();
  loopTempSolo();
  loopUmidadeSolo();
  Serial.println("----------------------------");
  //postHTTP();
  delay(10000);

}
