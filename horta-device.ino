#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>


//=========================================== D H T =============================================
#define DHTPIN 26  //Pino one está o DHT22

DHT dht(26, DHT11); //Objeto que faz a leitura da temperatura e umidade

float temperature = 0;
float humidity = 0;

void configDHT() {
  dht.begin();
}
void dhtsensor() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  Serial.print("temperature: ");
  Serial.print(temperature);
  Serial.println(" C");
  Serial.print("Umidade: ");
  Serial.println(humidity);
  Serial.println(" %");
  
  Serial.println("----------------------------");
}
//=========================================== U M I D A D E  DO  S O L O =============================================
//=========================================== T E M P E R A T U RA  DO  S O L O =============================================
//=========================================== W I F I =============================================
void configWIFI() {
  // Inicializa a conexão com a rede Wi-Fi
  WiFi.begin("Lorena", "L12345678");


  // Espera a conexão estar estabelecida
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }
  
}
void postHTTP() {
   if(WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    http.begin("https://jsonplaceholder.typicode.com/posts/1");

    int statusCode = http.GET();

    if (statusCode > 0) {
      Serial.print("HTTP status code: ");
      Serial.println(http.GET());
      Serial.print("HTTP response: ");
      Serial.println(http.getString());
    } else {
      Serial.println("HTTP ERRo: ");
      Serial.println(http.GET());
      Serial.println(http.getString());
    }

    http.end();
    delay(2000);
  }
}

void setup() {
  delay(100);
  configWIFI();
  configDHT();
  delay(2000);

  
  Serial.begin(115200);
}

void loop() {
  
  dhtsensor();
  //postHTTP();

}
