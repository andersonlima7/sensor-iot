#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h> // Importa a Biblioteca PubSubClient


#ifndef STASSID
#define STASSID "Cesar" //INTELBRAS
#define STAPSK  "25588122" //Pbl-Sistemas-Digitais
#endif

// MQTT
#define SUBSCRIBE_TOPIC "NodeMCU/comando"     //tópico MQTT de escuta
#define PUBLISH_TOPIC   "NodeMCU/resposta"    //tópico MQTT de envio de informações para Broker
#define CLIENTID "TP02-G03-ESP"


// Definições de rede
// IPAddress local_IP(10, 0, 0, 109);
// IPAddress gateway(10, 0, 0, 1);
// IPAddress subnet(255, 255, 0, 0);

const char* host = "ESP-10.0.0.109";
const char* ssid = STASSID;
const char* password = STAPSK;


// Configuração MQTT
const char* BROKER = "broker.emqx.io"; // FIXME: Trocar pelo broker do lab.
const int BROKER_PORT = 1883;

WiFiClient espClient; // Cria o objeto espClient
PubSubClient MQTT(espClient); // Instancia o Cliente MQTT passando o objeto espClient


int ledPin = LED_BUILTIN; // Pino do LED.
const int sensorAnalog = A0; // Entrada analógica.

//Variáveis que guardam o comando e o endereço do sensor na requisição.
byte command;
byte address;

// Variáveis para guardar os valores dos sensores.
int analogValue = 0; // Valor lido da entrada analógica.
int digitalValue = 0; // Valor lido dos sensores digitais.



// Funções MQTT

void onMessage(char* topic, byte* payload, unsigned int length);
void handleRequest(int command, int address);


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
        if (MQTT.connect(CLIENTID)) 
        {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(SUBSCRIBE_TOPIC); 
        } 
        else
        {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Tentando se conectar em 2s...");
            delay(2000);
        }
    }
}



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
 * Verifica se o cliente está conectado ao broker MQTT.
 * Em caso de desconexão, a conexão é restabelecida.
*/
void checkMQTTConnection(void)
{
    if (!MQTT.connected()) 
        reconnectMQTT(); //se não há conexão com o Broker, a conexão é refeita
    reconnectWiFi();
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
    String msg;
 
    //obtem a string do payload recebido
    for(int i = 0; i < length; i++) 
    {
       char c = (char)payload[i];
       msg += c;
    }

    Serial.println(msg);
   
    //toma ação dependendo da string recebida:
    //verifica se deve colocar nivel alto de tensão na saída D0:
    //IMPORTANTE: o Led já contido na placa é acionado com lógica invertida (ou seja,
    //enviar HIGH para o output faz o Led apagar / enviar LOW faz o Led acender)
    if (msg.equals("L"))
    {
        digitalWrite(ledPin, LOW);
        digitalValue = 1;
    }
 
    //verifica se deve colocar nivel alto de tensão na saída D0:
    if (msg.equals("D"))
    {
        digitalWrite(ledPin, HIGH);
        digitalValue = 0;
    }
     
}

/**
 * Envia dados para o SBC via o tópico de publicação.
*/
void sendData(void)
{
    if (digitalValue == 0)
      MQTT.publish(PUBLISH_TOPIC, "Desligado");
 
    if (digitalValue == 1)
      MQTT.publish(PUBLISH_TOPIC, "Ligado");
 
    Serial.println("- Estado da saida D0 enviado ao broker!");
    delay(1000);
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
  command = -1; // Comando requisitado
  address = -1; // Endereço requisitado
  initMQTT();
  digitalWrite(ledPin,HIGH);
  Serial.begin(9600); //Inicia a uart com um baudrate de 9600.
}

void loop() {
    ArduinoOTA.handle();
    checkMQTTConnection();
    sendData();
    MQTT.loop();
}
