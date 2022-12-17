#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h> // Importa a Biblioteca PubSubClient
#include "Timer.h"
#include <string.h> 
#include <stdlib.h> 


#ifndef STASSID
// #define STASSID "INTELBRAS" //"Cesar"
// #define STAPSK  "25588123" //"25588122"
#define STASSID "INTELBRAS" //"Cesar"
#define STAPSK  "Pbl-Sistemas-Digitais" //"25588122"
#endif

// MQTT
#define SUBSCRIBE_TOPIC_COMMAND "TP02G03/SBC/node0/comando"     //tópico MQTT de escuta dos comandos vindos da SBC.
#define SUBSCRIBE_TOPIC_TIME    "TP02G03/SBC/node0/tempo"       //tópico para alteração de tempo.
#define PUBLISH_TOPIC_RESPONSE  "TP02G03/SBC/node0/resposta"    //tópico MQTT de envio de códigos de resposta para o SBC.
#define PUBLISH_TOPIC_SENSORES   "TP02G03/SBC/node0/sensores"    //tópico MQTT de envio dos dados dos sensores.
#define CLIENTID "TP02G03-ESP"
#define USER "aluno"
#define PASSWORD "@luno*123"
#define QOS 2                                                   // A mensagem é sempre entregue exatamente uma vez.
                                 


//Mapeamento de pinos do NodeMCU

#define pinAnalog A0  // Entrada analógica.
 


// FIXME: Retirar o comentário. 
// Definições de rede
IPAddress local_IP(10, 0, 0, 109);
IPAddress gateway(10, 0, 0, 1);
IPAddress subnet(255, 255, 0, 0);

const char* host = "ESP-10.0.0.109";
const char* ssid = STASSID;
const char* password = STAPSK;


// Configuração MQTT
// const char* BROKER = "broker.emqx.io"; // FIXME: Trocar pelo broker do lab.
const char* BROKER = "10.0.0.101";
const int BROKER_PORT = 1883;

WiFiClient espClient; // Cria o objeto espClient
PubSubClient MQTT(espClient); // Instancia o Cliente MQTT passando o objeto espClient

// Mensagem que é enviada caso o NodeMCU perca a conexão(0X1F = NodeMCU com problema).
char WILL_MESSAGE[] = {0x1F};

int ledPin = LED_BUILTIN;     // Pino do LED.


int measurementTime = 30;     // valor padrão do tempo de medições: 30 segundos.
Timer t;                      // Para utilizar a biblioteca de tempo.

// Funções MQTT
void onMessage(char* topic, byte* payload, unsigned int length);
void publishData(char* topic, byte* message,  unsigned int length);
void publishString(char* topic, char* message);
void automaticMeasurements(void);


/**
 * Inicializa parâmetros de conexão MQTT (endereço do broker, porta e 
 * define a função executada ao chegar uma mensagem).
*/
void initMQTT() 
{
    MQTT.setServer(BROKER, BROKER_PORT);   //informa qual broker e porta deve ser conectado
    MQTT.setCallback(onMessage);            //atribui função de callback (função chamada quando qualquer informação de um dos tópicos subscritos chega)
}


/**
 * Reconecta-se ao broker MQTT (caso ainda não esteja conectado ou em caso de a conexão cair)
 * em caso de sucesso na conexão ou reconexão, o subscribe dos tópicos é refeito.
*/
void reconnectMQTT() 
{
    while (!MQTT.connected()) 
    {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER);
        // boolean connect (clientID, [username, password], [willTopic, willQoS, willRetain, willMessage], [cleanSession])
        // MQTT.connect(CLIENTID, USER, PASSWORD, SUBSCRIBE_TOPIC_COMMAND, QOS, false, WILL_MESSAGE)
        if (MQTT.connect(CLIENTID, USER, PASSWORD, PUBLISH_TOPIC_RESPONSE, QOS, false, WILL_MESSAGE))
        {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(SUBSCRIBE_TOPIC_COMMAND, 1); 
            MQTT.subscribe(SUBSCRIBE_TOPIC_TIME, 1); 
        } 
        else
        {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Tentando se conectar em 2s...");
            delay(2000);
        }
    }
}


/**
 * Caso a NodeMCU não esteja conectado ao WiFi, a conexão é restabelecida.
*/
void reconnectWiFi() 
{
    //se já está conectado a rede WI-FI, nada é feito. 
    //Caso contrário, são efetuadas tentativas de conexão
    if (WiFi.status() == WL_CONNECTED)
        return;
         
    WiFi.begin(ssid, password); // Conecta na rede WI-FI
     
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(100);
        Serial.print(".");
    }
   
    Serial.println();
    Serial.print("Conectado com sucesso na rede ");
    Serial.print(ssid);
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());
}
 


/**
 * Verifica se o cliente está conectado ao broker MQTT e ao WiFi.
 * Em caso de desconexão, a conexão é restabelecida.
*/
void checkMQTTConnection(void)
{
    reconnectWiFi();
    if (!MQTT.connected()) 
        reconnectMQTT(); //se não há conexão com o Broker, a conexão é refeita
}


/**
 * É chamada toda vez que uma informação de 
   um dos tópicos subscritos chega)
   @param topic Tópico inscrito.
   @param payload Conteúdo da mensagem recebida.
   @param length Tamanho da mensagem.
*/
void onMessage(char* topic, byte* payload, unsigned int length) 
{
    Serial.println("Mensagem recebida no tópico :");
    Serial.println(topic);
    if(length > 0) { // Verifica se há algo no payload da mensagem recebida.

        if(strcmp(topic, SUBSCRIBE_TOPIC_COMMAND) == 0) 
        {
          Serial.println("Mensagem de comando recebida.");
          Serial.println(payload[0]);
          switch(payload[0]) 
          {
            case 0x03: 
            {
             // Situação do NodeMCU
              byte response[1] = {0x00};
              publishData(PUBLISH_TOPIC_RESPONSE, response, 1);                     // NodeMCU funcionando OK.
              break;
            }
            case 0x04: 
            {
              // Valor da entrada analógica.
              int value = analogRead(pinAnalog);  // Ler o valor atual da entrada analógica. 
              byte firstByte = value / 256; // Pega o primeiro byte do valor.
              byte secondByte = value % 256; // Pega o segundo byte do valor.         
              Serial.println(value);
              byte response[] = {0x01, firstByte, secondByte};    
              publishData(PUBLISH_TOPIC_RESPONSE, response, sizeof(response));    
            }
              break;
            case 0x05: // Valor da entrada digital.
            {
              int valor = payload[1];                          // Pino que o sensor está pinado.
              int pino;
              switch (pino)
              {
              case 0:
                pino = D0;
                Serial.println("Pino D0!");
                break;
              case 1:
                pino = D1;
                break;
              case 2:
                pino = D2;
                break;
              case 3:
                pino = D3;
                break;
              case 4:
                pino = D4;
                break;
              case 5:
                pino = D5;
                break;
              case 6:
                pino = D6;
                break;
              case 7:
                pino = D7;
                break;
              default:
                break;
              }
              pinMode(pino, INPUT);                           // Define o pino como entrada.                     
              int value = digitalRead(pino);                // Ler o valor do sensor neste pino.
              Serial.println(value);
              byte response[] = {0x02, value};
              publishData(PUBLISH_TOPIC_RESPONSE, response, sizeof(response));   
            }
              break;
            case 0x06: //Controla o LED, ligando se estiver desligado ou desligando se estiver ligado.
              {
                int ligado = digitalRead(ledPin);
                digitalWrite(ledPin, !ligado);
                byte response[1];
                if(!ligado) // LED desligado
                {
                  response[0] = {0x07};   
                }
                else {   // Led ligado
                  response[0] = {0x08}; 
                }
                publishData(PUBLISH_TOPIC_RESPONSE, response, 1);  
                break;
              }
            default:
                //Problema na requisicao ao NodeMCU.
                byte response[1] = {0x1F};
                publishData(PUBLISH_TOPIC_RESPONSE, response, 1);
                break;
          }
        }
        else if(strcmp(topic, SUBSCRIBE_TOPIC_TIME) == 0){

                Serial.println("Alteracao do tempo recebida.");
                String str = " ";
                for (int i = 0; i< length; i++){
                  char c = (char) payload[i];
                  str+= c;
                }

                int newTime = str.toInt();
                measurementTime = newTime;
                Serial.print("Novo tempo: ");
                Serial.println(measurementTime);
                byte response[1] = {0x0A};
                publishData(PUBLISH_TOPIC_RESPONSE, response, 1);
                t.stop(0);
                t.every(measurementTime * 1000, automaticMeasurements);
        }
    }
    else{
      Serial.println("Mensagem vazia!");
    }
}


/**
 * Envia dados para o SBC via o tópico de publicação.
 * @param topic Tópico que será publicado a mensagem.
 * @param message Mensagem a ser enviada.
 * @param length Quantidade de bytes a ser enviado.
*/
void publishData(char* topic, byte* message,  unsigned int length) 
{
    // boolean publish (topic, payload, [length], [retained])
    // Envia a mensagem ao broker para ser enviada aos subscribers(SBC) e indica que essa mensagem deve ser armazenada no broker.
    // Desse modo, permite que os subscribers possam ter a informação mais atualizada sem ter que esperar o publisher enviar a nova informação.
    MQTT.publish(topic, message, length, true);  
    Serial.println("Mensagem enviada ao SBC!");
}

/**
 * Envia dados ce string para o SBC via o tópico de publicação.
 * @param topic Tópico que será publicado a mensagem.
 * @param message Mensagem a ser enviada.
*/
void publishString(char* topic, char* message) 
{
    // boolean publish (topic, payload, [length], [retained])
    // Envia a mensagem ao broker para ser enviada aos subscribers(SBC) e indica que essa mensagem deve ser armazenada no broker.
    // Desse modo, permite que os subscribers possam ter a informação mais atualizada sem ter que esperar o publisher enviar a nova informação.
    MQTT.publish(topic, message, true);  
    Serial.println("Mensagem enviada ao SBC!");
}


/**
 * Realiza as medições automáticas dos sensores em um tempo definido.
*/
void automaticMeasurements(void)
{
  byte sensor1 = {digitalRead(D0)};
  byte sensor2 = {digitalRead(D1)};
  int analogValue= analogRead(pinAnalog);
  byte firstByteSensor3 = analogValue / 256; // Pega o primeiro byte do valor.
  byte secondByteSensor3 = analogValue % 256; // Pega o segundo byte do valor.

  byte values[] = {sensor1, sensor2, firstByteSensor3, secondByteSensor3};
  
  publishData(PUBLISH_TOPIC_SENSORES, values, sizeof(values));
}


void code_uploaded(){
  for(int i=0;i<2;i++){
    digitalWrite(LED_BUILTIN,LOW);
    delay(150);
    digitalWrite(LED_BUILTIN,HIGH);
    delay(150);
  }
}

void OTA_setup(){
  
  Serial.begin(115200);
  Serial.println("Booting");

//   // Configuração do IP fixo no roteador, se não conectado, imprime mensagem de falha
//   if (!WiFi.config(local_IP, gateway, subnet)) {
//     Serial.println("STA Failed to configure");
//   }

  Serial.println("------Conexao WI-FI------");
  Serial.print("Conectando-se na rede: ");
  Serial.println(ssid);
  Serial.println("Aguarde");


  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(host);

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  
}


void setup() {
  code_uploaded();
  OTA_setup(); 
  pinMode(ledPin,OUTPUT);
  initMQTT();
  t.every(measurementTime * 1000, automaticMeasurements);  // Define que a função de medições automáticas deve ser chamada a cada X segundos.
  digitalWrite(ledPin, HIGH);
  Serial.begin(9600); 
}

void loop() {
    ArduinoOTA.handle();
    checkMQTTConnection();
    MQTT.loop(); //This should be called regularly to allow the client to process incoming messages and maintain its connection to the server.
    t.update();
}
