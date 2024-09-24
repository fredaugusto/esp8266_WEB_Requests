/*
  Fred Augusto
  
  Este software foi desenvolvido para fins didáticos e os arquivos de associados são gratuitos a qualquer pessoa que obtenha uma cópia.

  O código é compatível com placas ESP8266 na versão 3.0.0 ou superior


  Informações importantes:
    O led irá piscar enquanto estiver procurando a rede WiFi.
    Quando encontrar e conectar, ele irá ficar acesso.
    Quando apagar em um espaço de tempo maior, significa que está enviando dados.
    Se ele piscar ao final deste período, é por que a requisisão falhou.

  
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

const char* ssid = "VIVO-7738";
const char* password = "vivo123456";

ESP8266WebServer server(80);

// insira aqui apenas o endereço para realizar o request, não insira nenhum parâmetro ainda
String serverName = "https://fredaugusto.com.br/teste/enviar_dados.php";

// Insira aqui o tempo que será aguardado antes de enviar uma nova requisição
unsigned long timerDelay = 5000;
unsigned long lastTime = 0;
long randNumber;

IPAddress ip;

void setup() {
  // Mantenha em 115200, o uso de outros valores apresentou falhas em diversos sistemas
  Serial.begin(115200);

  Serial.println();
  
  pinMode(LED_BUILTIN, OUTPUT); // LED_BUILTIN é normalmente o pino GPIO2

  // Inicia as configurações de um servidor WEB para o ESP
  server.on("/", handleRoot);
  server.on("/update", handleUpdate);
  server.begin();

  // Modelo de conexão ao Wi-Fi
  WiFi.mode(WIFI_STA);
  Serial.print("Conectando ao WiFi ("+ String(ssid)+") ..");
  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    for (int i = 0; i < 30; i++) {
      if(WiFi.status() == WL_CONNECTED){
        i=30;
      }
      Serial.print('.');
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(300);
    }
    if(WiFi.status() != WL_CONNECTED){
      Serial.println();
      Serial.println("A conexão está demorando um pouco, verifique se o SSID da rede e sua senha estão corretos.");
      Serial.println("Verifique também se a rede ("+String(ssid)+") está no alcance.");
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    ip = WiFi.localIP();
    Serial.println();
    digitalWrite(LED_BUILTIN, LOW);
    Serial.print("Conectado (" + String(ssid) + ") - ");
    Serial.println(ip);
  }

  // Gera um seed para geração de um número aleatório para testes
  randomSeed(analogRead(0));
}


void handleRoot() {
  String html = "<html><body style='margin:0px;padding:0px;font-family:Calibri,Verdana,Arial'>";
  html += "<div style='position=fixed;top:0px;left:0px;width:100%;height:10vh;background-color:#000fb5;color:#dfe1f7;text-align:center;font-size:3em;display:flex'><div style='margin:auto'>ESP8266 Configuration interface</div></div>";
  html += "<div style='width:70%;text-align:center;margin:auto'>";
  html += "<h1>ESP8266</h1>";
  html += "<form action='/update' method='GET'>";
  html += "<div style='font-size:0.7em'>Endpoint para request (GET)</div>";
  html += "<input type='text' name='serverName' style='width: 90%'><br>";
  html += "<div style='font-size:0.7em'>Novo Timer Delay (ms)</div>";
  html += "<input type='number' min='100' name='timerDelay' style='width: 90%; text-align: center'><br>";
  html += "<input type='submit' value='Atualizar' style='margin: 3px;padding:5px; background-color: #000fb5; color: #dfe1f7; border-radius: 3px; border-color: #00085e'>";
  html += "</form>";
  html += "</div>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleUpdate() {
  if (server.hasArg("serverName")) {
    serverName = server.arg("serverName");
  }
  if (server.hasArg("timerDelay")) {
    int timerUpdate = server.arg("timerDelay").toInt();
    if(timerUpdate > 100){
      timerDelay = timerUpdate;
    }
  }
  server.send(200, "text/html", "<h1>Configurações Atualizadas!</h1><a href=\"/\">Voltar</a>");
}


void loop() {
  server.handleClient();
  digitalWrite(LED_BUILTIN, LOW);
  // Utiliza o milis para gerar um delay sem congelar o sistema
  if ((millis() - lastTime) > timerDelay) {
    // Mostra o IP do ESP para possíveis conexões
    Serial.print("Endereço IP: ");
    Serial.println(ip);
    
    // Aguarda a conexão Wi-Fi
    if ((WiFi.status() == WL_CONNECTED)) {

      // O uso da biblioteca foi necessário para as requisições via https
      std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
  
      // Recurso para ignorar os certificados https
      client->setInsecure();
      
      HTTPClient https;


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

      Serial.print("[HTTPS] Iniciando requisição \"" + serverPath + "\"");
      
      if (https.begin(*client, serverPath)) {  // HTTPS
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.println("[HTTPS] GET");

        int httpCode = https.GET();

        if (httpCode > 0) {

          Serial.println("[HTTPS] GET. HTTP code: " + String(httpCode));
          
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            String payload = https.getString();
            Serial.println(payload);
          } else {
            digitalWrite(LED_BUILTIN, LOW);
            delay(200);
            digitalWrite(LED_BUILTIN, HIGH);
          }
        } else {
          Serial.println("[HTTPS] GET... requisição falhou, erro: "+ String(https.errorToString(httpCode).c_str()));
          digitalWrite(LED_BUILTIN, LOW);
          delay(200);
          digitalWrite(LED_BUILTIN, HIGH);
        }
  
        https.end();
        digitalWrite(LED_BUILTIN, LOW);
      } else {
        Serial.println("[HTTPS] Não foi possível realizar o request.");
      }
    }
    Serial.println();    
    lastTime = millis();
  }
}
