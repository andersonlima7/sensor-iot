#include <stdio.h>
#include <string.h>
#include <wiringPi.h>
#include <lcd.h>
#include "MQTTClient.h"
#include <stdlib.h>



/*
* Configuração MQTT
*/

#define ADDRESS "tcp://10.0.0.101:1883"
#define CLIENTID "alunoSBC"
#define USERNAME "TP02-G03"
#define PASSWORD "@luno*123"
#define SUBSCRIBE_TOPIC "sensores/#"
#define SUBSCRIBE_TOPIC_NODEMCU "NodeMCU/sensores/#"
#define SUBSCRIBE_TOPIC_WEB "SBC/tempo/"
#define PAYLOAD "maçã"
#define QOS 2
#define TIMEOUT 10000L


// Pinagem LCD
#define RS 13
#define E  18
#define D4 21
#define D5 24
#define D6 26
#define D7 27   

// Botões
#define btn1 19                                              
#define btn2 23                                              
#define btn3 25

int lcd;
MQTTClient client;

/*
* Prototipos de funcao
*/
void publish(MQTTClient client, char* topic, char* payload);
int on_message(void *context, char *topicName, int topicLen, MQTTClient_message *message);
void writeLCD(char *string1, char *string2);


/**
 * Publica uma mensagem no tópico especificado.
 * @param client Cliente MQTT
 * @param topic Tópico que a mensagem será publicada.
 * @param payload Mensagem.
*/
void publish(MQTTClient client, char* topic, char* payload) {
    MQTTClient_message message = MQTTClient_message_initializer;
    
    message.payload = payload;
    message.payloadlen = strlen(message.payload);
    message.qos = QOS;
    message.retained = 0;

    MQTTClient_deliveryToken token;
    MQTTClient_publishMessage(client, topic, &message, &token);
    MQTTClient_waitForCompletion(client, token, 1000L);
    printf("Mensagem com o token %d enviada\n", token);
}


/**
 * Função que é executada ao receber uma mensagem de um dado tópico.
*/
int on_message(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    char* payload = message->payload;
 
    /* Mostra a mensagem recebida */
    printf("Mensagem recebida! \n\rTopico: %s Mensagem: %s\n", topicName, payload);
 
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;

}

/**
 * Escreve no display LCD.
 * @param string1 Mensagem da primeira linha.
 * @param string2 Mensagem da segunda linha.
*/
void writeLCD(char *string1, char *string2) {
    lcdHome(lcd);
    lcdClear(lcd);
    lcdPosition(lcd, 0, 0);
    lcdPuts(lcd, string1);
    lcdPosition(lcd, 0, 1);
    lcdPuts(lcd, string2);
}

int main()
{

    wiringPiSetup();


    pinMode(btn1, INPUT);
    pinMode(btn2, INPUT);
    pinMode(btn3, INPUT);

    /**
     * Inicializa o display em modo 4 bits e 2 linhas.
    */
    lcd = lcdInit(2, 16, 4, RS, E, D4, D5, D6, D7, 0, 0, 0, 0) ;
    lcdHome(lcd);
    lcdClear(lcd);

    writeLCD("  Problema #03  ", "   IoT - MQTT   ");
    
    
    int rc;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_create(&client, ADDRESS, CLIENTID,
                      MQTTCLIENT_PERSISTENCE_NONE, NULL);

    // MQTT Connection parameters
    conn_opts.username = USERNAME;
    conn_opts.password = PASSWORD;
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Falha o conectar no MQTT, return code %d\n", rc);
        writeLCD("Erro:", "Conexão MQTT");
        exit(-1);
    }

    MQTTClient_subscribe(client, SUBSCRIBE_TOPIC, 0);

    while(1)
   {
       /*
        * o exemplo opera por "interrupcao" no callback de recepcao de 
        * mensagens MQTT. Portanto, neste laco principal eh preciso fazer
        * nada.
        */
   }

    
}