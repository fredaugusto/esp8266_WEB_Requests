# ESP8266_WEB_Requests

Este projeto é uma implementação simples de requisições HTTP usando o ESP8266. O código foi desenvolvido com fins didáticos e é gratuito para qualquer pessoa que o obtenha.

## Descrição

Este software permite que um dispositivo ESP8266 se conecte a uma rede Wi-Fi e envie requisições HTTP GET para um servidor. Os dados são enviados periodicamente, utilizando um número aleatório como exemplo. A implementação faz uso de conexões HTTPS para garantir a segurança nas comunicações.

## Funcionalidades

- Conexão com Wi-Fi
- Envio de requisições HTTP GET
- Geração de números aleatórios para testes
- Suporte a HTTPS

## Requisitos

- Placa ESP8266 (versão 3.0.0 ou superior)
- Biblioteca `ESP8266WiFi`
- Biblioteca `ESP8266HTTPClient`
- Biblioteca `WiFiClientSecureBearSSL`

## Como Usar

1. **Configuração do Wi-Fi**: 
   - Altere as variáveis `ssid` e `password` com os detalhes da sua rede Wi-Fi.

2. **Defina o Endereço do Servidor**:
   - Modifique a variável `serverName` com o endereço do seu endpoint. O exemplo atual é `https://fredaugusto.com.br/teste/enviar_dados.php`.

3. **Ajuste o Intervalo de Requisições**:
   - A variável `timerDelay` define o tempo em milissegundos entre cada requisição. O padrão é 5000 ms (5 segundos).

4. **Parâmetros da Requisição**:
   - Você pode adicionar parâmetros à requisição GET na string `serverPath`, seguindo o formato: `?chave=valor`.

5. **Compilar e Carregar**:
   - Compile e faça o upload do código para sua placa ESP8266 usando o Arduino IDE.

## Código

```cpp
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

String serverName = "https://fredaugusto.com.br/teste/enviar_dados.php";

unsigned long timerDelay = 5000;
unsigned long lastTime = 0;
long randNumber;

void setup() {
  Serial.begin(115200);
  Serial.println();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao WiFi ("+ssid+") ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }

  randomSeed(analogRead(0));
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    if ((WiFi.status() == WL_CONNECTED)) {
      std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
      client->setInsecure();
      
      HTTPClient https;
      Serial.print("[HTTPS] begin...\n");

      randNumber = random(100, 900);
      String serverPath = serverName + "?valor=" + String(int(randNumber));

      Serial.println(serverPath);
      
      if (https.begin(*client, serverPath)) {
        Serial.print("[HTTPS] GET");
        int httpCode = https.GET();

        if (httpCode > 0) {
          Serial.printf("[HTTPS] GET. HTTP code: %d\n", httpCode);
          
          if (httpCode == HTTP_CODE_OK `` httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
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
``

## Contribuições

Contribuições são bem-vindas! Sinta-se à vontade para abrir issues ou enviar pull requests.

## Licença

Este projeto é de domínio público e pode ser utilizado livremente.
