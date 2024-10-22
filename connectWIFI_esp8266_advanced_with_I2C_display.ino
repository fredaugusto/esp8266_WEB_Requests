/*
  Fred Augusto
  
  Este software foi desenvolvido para fins didáticos e os arquivos de associados são gratuitos a qualquer pessoa que obtenha uma cópia.

  O código é compatível com placas ESP8266 na versão 3.0.0 ou superior


  Informações importantes:
    O led irá piscar enquanto estiver procurando a rede WiFi.
    Quando encontrar e conectar, ele irá ficar acesso.
    Quando apagar em um espaço de tempo maior, significa que está enviando dados.
    Se ele piscar ao final deste período, é por que a requisisão falhou.
    Se ele piscar de forma mais rápida, foi criado um ponto de acesso para configuração de SSID e senha para conexão em uma rede
      O esp não possui DHCP, logo é necessário definir de forma manual um IP fixo dentro da faixa 192.168.1.0/24
      O endereço do ESP neste momento será 192.168.1.1
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <Wire.h>
#include "SSD1306Wire.h"

// Inicializa o display Oled
SSD1306Wire  display(0x3c, D1, D2);

const char* ssid = "VIVO-7738";
const char* password = "vivo12345";

const char* ssidConfigAP = "ESP Config AP"; // Nome da rede Wi-Fi
const char* passwordConfigAP = "00000000"; // Senha da rede Wi-Fi

ESP8266WebServer server(80);

// insira aqui apenas o endereço para realizar o request, não insira nenhum parâmetro ainda
String serverName = "https://fredaugusto.com.br/teste/enviar_dados.php";

// Insira aqui o tempo que será aguardado antes de enviar uma nova requisição
unsigned long timerDelay = 5000;
unsigned long lastTime = 0;
long randNumber;

IPAddress ip;

bool configured = false;



void handleRoot() {
  String html = "<html><body style='margin:0px;padding:0px;font-family:Calibri,Verdana,Arial'>";
  html += "<div style='position=fixed;top:0px;left:0px;width:100%;height:10vh;background-color:#000fb5;color:#dfe1f7;text-align:center;font-size:3em;display:flex'><div style='margin:auto'>ESP8266 Configuration interface</div></div>";
  html += "<div style='width:70%;text-align:center;margin:auto'>";
  html += "<h1>ESP8266</h1>";
  html += "<form action='/update' method='GET'>";
  html += "<div style='font-size:0.7em'>WiFi SSID</div>";
  html += "<input type='text' name='wifissid' style='width: 90%' value='"+String(ssid)+"'><br>";
  html += "<div style='font-size:0.7em'>WiFi Password</div>";
  html += "<input type='text' name='wifipassword' style='width: 90%' value='"+String(password)+"'><br>";
  
  html += "<div style='font-size:0.7em'>Endpoint para request (GET)</div>";
  html += "<input type='text' name='serverName' style='width: 90%' value='"+serverName+"'><br>";
  html += "<div style='font-size:0.7em'>Novo Timer Delay (ms)</div>";
  html += "<input type='number' min='100' name='timerDelay' style='width: 90%; text-align: center' value='"+String(timerDelay)+"'><br>";
  html += "<input type='submit' value='Atualizar' style='margin: 3px;padding:5px; background-color: #000fb5; color: #dfe1f7; border-radius: 3px; border-color: #00085e'>";
  html += "</form>";
  html += "</div>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleUpdate() {
  if (server.hasArg("wifissid")) {
    ssid = server.arg("wifissid").c_str();
    configured = true;
  }
  if (server.hasArg("wifipassword")) {
    password = server.arg("wifipassword").c_str();
    configured = true;
  }
  if (server.hasArg("serverName")) {
    serverName = server.arg("serverName");
  }
  if (server.hasArg("timerDelay")) {
    int timerUpdate = server.arg("timerDelay").toInt();
    if(timerUpdate > 100){
      timerDelay = timerUpdate;
    }
  }
  server.send(200, "text/html", "<h1>Dados Atualizados!</h1><a href=\"/\">Voltar</a>");
}


void setup() {
  // Mantenha em 115200, o uso de outros valores apresentou falhas em diversos sistemas
  Serial.begin(115200);

  display.init();
  display.flipScreenVertically();

  Serial.println();
  
  pinMode(LED_BUILTIN, OUTPUT); // LED_BUILTIN é normalmente o pino GPIO2


  for (int counter = 0; counter <= 100; counter++)
  {
    display.clear();
    //Desenha a barra de progresso
    display.drawProgressBar(0, 32, 120, 10, counter);
    //Atualiza a porcentagem completa
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 15, String(counter) + "%");
    display.display();
    delay(10);
  }
  delay(1000);


  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  //Seleciona a fonte
  //display.setFont(ArialMT_Plain_10);
  //display.setFont(ArialMT_Plain_16);
  //display.setFont(ArialMT_Plain_24);
  display.setFont(ArialMT_Plain_10);


  // Modelo de conexão ao Wi-Fi
  WiFi.mode(WIFI_STA);
  while (WiFi.status() != WL_CONNECTED) {
    bool connectedWiFi = false;
    // Tenta por 'k' vezes se conectar à rede especificada
    Serial.println("Conectando ao WiFi ("+ String(ssid)+") ..");
    
    //Apaga o display
    display.clear();
    display.drawString(63, 1, "Conectando ao WiFi");
    display.drawString(63, 26, String(ssid));
    display.display();
    for (int k = 0; k < 4; k++) {
    
      WiFi.begin(ssid, password);
      for (int i = 0; i < 30; i++) {
        if(WiFi.status() == WL_CONNECTED){
          connectedWiFi = true;
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

        //Apaga o display
        display.clear();
        display.drawString(63, 1, "A conexão está lenta,");
        display.drawString(63, 16, "verifique o SSID e senha.");
        display.drawString(63, 32, "Verifique o alcance da rede");
        display.drawString(63, 48, String(ssid));
        display.display();
      }
    }
     
    //Quando não conseguir conectar à rede, inicia um ponto de acesso para configuração
    if(!connectedWiFi){
      configured = false;
      display.clear();
      display.drawString(63, 1, "Criando ponto de acesso");
      display.drawString(63, 26, "para configuração.");
      display.display();
      Serial.println("Criando ponto de acesso para configuração.");
      delay(1000);
      // Configura o ESP como ponto de acesso
      WiFi.softAP(ssidConfigAP, passwordConfigAP);
      
      IPAddress local_ip(192, 168, 1, 1); // Endereço IP do ponto de acesso
      IPAddress gateway(192, 168, 1, 1); // Gateway
      IPAddress subnet(255, 255, 255, 0); // Máscara de sub-rede
      
      WiFi.softAPConfig(local_ip, gateway, subnet);
  
      // Obtém o endereço IP do ponto de acesso
      IPAddress ip = WiFi.softAPIP();
      Serial.print("Ponto de Acesso criado com IP: ");
      Serial.println(ip);
      
      display.clear();
      display.drawString(63, 1, "Ponto de Acesso: ");
      display.drawString(63, 24, ssidConfigAP);
      display.drawString(63, 46, ip.toString());
      display.display();
      
      // Inicia as configurações de um servidor WEB para o ESP
      server.on("/", handleRoot);
      server.on("/update", handleUpdate);
      server.begin();
      while(!configured){
        server.handleClient();
        digitalWrite(LED_BUILTIN, HIGH);
        delay(400);
        digitalWrite(LED_BUILTIN, LOW);
        delay(50);
      }
      Serial.println("O dispositivo reiniciará e tentará conectar na nova rede.");
      display.clear();
      display.drawString(63, 1, "O dispositivo reiniciará e tentará");
      display.drawString(63, 26, "conectar na nova rede.");
      display.display();
      delay(2000);
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    ip = WiFi.localIP();
    Serial.println();
    digitalWrite(LED_BUILTIN, LOW);
    Serial.print("Conectado (" + String(ssid) + ") - ");
    display.clear();
    display.drawString(63, 1, "Conectado");
    display.drawString(63, 14, String(ssid));
    display.display();
    Serial.println(ip);
  }

  // Gera um seed para geração de um número aleatório para testes
  randomSeed(analogRead(0));
}


void loop() {
  server.handleClient();
  digitalWrite(LED_BUILTIN, LOW);
  // Utiliza o milis para gerar um delay sem congelar o sistema
  if ((millis() - lastTime) > timerDelay) {
    // Mostra o IP do ESP para possíveis conexões
    Serial.print("Endereço IP: ");
    Serial.println(ip);

    display.clear();
    display.drawString(63, 1, "Conectado");
    display.drawString(63, 12, String(ssid));
    display.drawString(63, 24, ip.toString());
    display.display();
    
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

      Serial.println("[HTTP] Iniciando requisição \"" + serverPath + "\" [GET]");
      
      display.clear();
      display.drawString(63, 1, "Conectado");
      display.drawString(63, 12, String(ssid));
      display.drawString(63, 24, ip.toString());
      display.drawString(63, 36, "[HTTP] Request");
      display.display();
      
      if (https.begin(*client, serverPath)) {  // HTTPS
        digitalWrite(LED_BUILTIN, HIGH);

        int httpCode = https.GET();

        if (httpCode > 0) {

          Serial.println("[HTTP] GET. HTTP code: " + String(httpCode));
          
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            String payload = https.getString();
            Serial.println(payload);
          } else {
            digitalWrite(LED_BUILTIN, LOW);
            delay(200);
            digitalWrite(LED_BUILTIN, HIGH);
          }
        } else {
          
          display.clear();
          display.drawString(63, 1, "Conectado");
          display.drawString(63, 12, String(ssid));
          display.drawString(63, 24, ip.toString());
          display.drawString(63, 36, "[HTTP] Request Fail");
          display.drawString(63, 48, String(https.errorToString(httpCode).c_str()));
          display.display();
          
          Serial.println("[HTTP] GET... requisição falhou, erro: "+ String(https.errorToString(httpCode).c_str()));
          digitalWrite(LED_BUILTIN, LOW);
          delay(200);
          digitalWrite(LED_BUILTIN, HIGH);
        }
  
        https.end();
        digitalWrite(LED_BUILTIN, LOW);
      } else {
        display.clear();
        display.drawString(63, 1, "Conectado");
        display.drawString(63, 12, String(ssid));
        display.drawString(63, 24, ip.toString());
        display.drawString(63, 36, "[HTTP] Request Fail");
        display.display();
        Serial.println("[HTTP] Não foi possível realizar o request.");
      }
    }
    Serial.println();    
    lastTime = millis();
  }
}
