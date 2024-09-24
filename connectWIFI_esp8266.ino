/*
  Fred Augusto
  
  Este software foi desenvolvido para fins didáticos e os arquivos de associados são gratuitos a qualquer pessoa que obtenha uma cópia.

  O código é compatível com placas ESP8266 na versão 3.0.0 ou superior
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

const char* ssid = "VIVO-7738";
const char* password = "vivo123456";

// insira aqui apenas o endereço para realizar o request, não insira nenhum parâmetro ainda
String serverName = "https://fredaugusto.com.br/teste/enviar_dados.php";

// Insira aqui o tempo que será aguardado antes de enviar uma nova requisição
unsigned long timerDelay = 5000;
unsigned long lastTime = 0;
long randNumber;

void setup() {
  // Mantenha em 115200, o uso de outros valores apresentou falhas em diversos sistemas
  Serial.begin(115200);

  Serial.println();

  // Modelo de conexão ao Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao WiFi ("+ String(ssid)+") ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }

  // Gera um seed para geração de um número aleatório para testes
  randomSeed(analogRead(0));
}

void loop() {
  // Utiliza o milis para gerar um delay sem congelar o sistema
  if ((millis() - lastTime) > timerDelay) {
    
    // Aguarda a conexão Wi-Fi
    if ((WiFi.status() == WL_CONNECTED)) {

      // O uso da biblioteca foi necessário para as requisições via https
      std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
  
      // Recurso para ignorar os certificados https
      client->setInsecure();
      
      HTTPClient https;
      Serial.print("[HTTPS] begin...\n");


      randNumber = random(100, 900);

      // Adicione aqui os parâmetros para a requisição via GET
      // Siga o padrão: 
      //  ex: https://fredaugusto.com.br/teste/?valor=123&valor2=456
      //  '?' => indica o limitador entre o endereço e os parâmetros a serem enviados
      //  '=' => indica que a váriavel da esquerda receberá o valor da direita (atribuição)
      //  '&' => indica que será enviada mais uma varíavel, ou seja, delimitador para
      //         o envio de várias variáveis
      //  
      //  Em resumo, será enviado ao endereço "https://fredaugusto.com.br/teste/" as
      //  variáveis "valor" e "valor2" com os valores "123" e "456" respectivamente.
      String serverPath = serverName + "?valor=" + String(int(randNumber));

      Serial.println(serverPath);
      
      if (https.begin(*client, serverPath)) {  // HTTPS
        Serial.println("[HTTPS] GET");

        int httpCode = https.GET();

        if (httpCode > 0) {

          Serial.printf("[HTTPS] GET. HTTP code: %d\n", httpCode);
          
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            String payload = https.getString();
            Serial.println(payload);
          }
        } else {
          Serial.printf("[HTTPS] GET... requisição falhou, erro: %s\n", https.errorToString(httpCode).c_str());
        }
  
        https.end();
      } else {
        Serial.printf("[HTTPS] Não foi possível realizar o request\n");
      }
    }
    Serial.println();    
    lastTime = millis();
  }
}
