#include <stdio.h>
#include <string.h>
//#include <wiringPi.h>
//#include <lcd.h>
#include "MQTTClient.h"
#include <stdlib.h>
#include <string.h>

/*
 * Configuração MQTT
 */
#define ADDRESS "mqtt://broker.emqx.io:1883"
#define CLIENTID "alunoSBC"
#define USERNAME ""
#define PASSWORD ""
#define QOS 0
#define TIMEOUT 10000L

/*
 * Comandos de Solicitação
 */
const int situacao_atual = 0x03;    // código que requisita a situacao atual do NodeMCU.
const int entrada_analogica = 0x04; // código que requisita o valor da entrada analogica.
const int sensor_digital = 0x05;    // código que requisita o valor da entrada digital.
const int controlar_led = 0x06;     // código que requisita ligar/desligar o led.
int sensor = 0;                     // Armazena a opcao do sensor selecionado pelo usuário.

/**
 * Valores atuais dos sensores
 */
int historico_sensores1[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int historico_sensores2[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int historico_sensores3[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int node0_sensor1 = 0;
int node0_sensor2 = 0;
int node0_sensor3 = 0;
int tempo = 0;
FILE *arquivo_historico;

/*
 * Comandos de resposta
 */

const int node_ok = 0x00;
const int node_erro = 0x1F;
const int led_on = 0x07;
const int led_off = 0x08;

/*
 * Tópicos - Receptor
 */
#define NODE0 "TP02G03/SBC/node0/#"
#define NODE0_SENSOR1 "TP02G03/SBC/node0/sensor1"
#define NODE0_SENSOR2 "TP02G03/SBC/node0/sensor2"
#define NODE0_SENSOR3 "TP02G03/SBC/node0/sensor3"
#define NODE0_RESPOSTA "TP02G03/SBC/node0/resposta"
#define APLICACAO_TEMPO_MEDICAO "TP02G03/SBC/aplicacao/tempo"

/*
 * Tópicos - Publicador
 */
#define SBC_DEFINIR_TEMPO "TP02G03/SBC/node0/tempo"
#define SBC_ENVIAR_COMANDO "TP02G03/SBC/node0/comando"
#define SBC_ENVIAR_DADOS_SENSORES "TP02G03/SBC/aplicacao/sensores"
#define SBC_ENVIAR_HISTORICO "TP02G03/SBC/aplicacao/historico"

// Pinagem LCD
#define RS 13
#define E 18
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

void slice(const char *str, char *result, size_t start, size_t end)
{
    strncpy(result, str + start, end - start);
}

/*
 * Prototipos de funcao
 */
void publish(MQTTClient client, char *topic, char *payload);
int on_message(void *context, char *topicName, int topicLen, MQTTClient_message *message);
// void writeLCD(char *string1, char *string2);

/**
 * Publica uma mensagem no tópico especificado.
 * @param client Cliente MQTT
 * @param topic Tópico que a mensagem será publicada.
 * @param payload Mensagem.
 */
void publish(MQTTClient client, char *topic, char *payload)
{
    MQTTClient_message message = MQTTClient_message_initializer;
    message.payload = payload;
    if (strcmp(topic, SBC_DEFINIR_TEMPO) == 0)
    {
        sprintf(message.payload, "%i", *payload);
    }
    message.payloadlen = strlen(message.payload);
    message.qos = QOS;
    message.retained = 1;

    MQTTClient_deliveryToken token;
    MQTTClient_publishMessage(client, topic, &message, &token);
    MQTTClient_waitForCompletion(client, token, 1000L);
    printf("Mensagem com o token %d enviada\n", token);
    message.payload = malloc(sizeof(message.payload));
}

void enviar_valores_sensores_aplicacao()
{
    char msg_json[256];
    sprintf(msg_json, "{sensor1: %i, sensor2: %i, sensor3: %i, tempo: %i}", node0_sensor1, node0_sensor2, node0_sensor3, tempo);
    publish(client, SBC_ENVIAR_DADOS_SENSORES, msg_json);
}

/**
 * Função que é executada ao receber uma mensagem de um dado tópico.
 */
int on_message(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    char *payload = message->payload;
    if (strcmp(topicName, NODE0_SENSOR1) == 0)
    {
        node0_sensor1 = *payload;
        printf("Valor sensor 1: %i", node0_sensor1);
    }
    if (strcmp(topicName, NODE0_SENSOR2) == 0)
    {
        node0_sensor2 = *payload;
        printf("Valor sensor 2: %i", node0_sensor2);
    }
    if (strcmp(topicName, NODE0_SENSOR3) == 0)
    {
        node0_sensor3 = *payload;
        printf("Valor sensor 3: %i", node0_sensor3);
    }
    printf("%i", *payload);
    if (strcmp(topicName, NODE0_RESPOSTA) == 0)
    {
        // print("Resposta: %i")
        if (*payload == node_ok)
        {
            printf("NodeMCU OK!\n");
        }
        if (*payload == node_erro)
        {
            printf("NodeMCU ERRO!\n");
        }
        if (*payload == led_on)
        {
            printf("LED ligado.\n");
        }
        if (*payload == led_off)
        {
            printf("LED desligado.\n");
        }
    }

    enviar_valores_sensores_aplicacao();
    /* Mostra a mensagem recebida */
    // printf("Mensagem recebida! \n\rTopico: %s Mensagem: %s\n", topicName, payload);

    // publish(client, SUBSCRIBE_TOPIC, cat);

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

/**
 * Escreve no display LCD.
 * @param string1 Mensagem da primeira linha.
 * @param string2 Mensagem da segunda linha.

void writeLCD(char *string1, char *string2)
{
    lcdHome(lcd);
    lcdClear(lcd);
    lcdPosition(lcd, 0, 0);
    lcdPuts(lcd, string1);
    lcdPosition(lcd, 0, 1);
    lcdPuts(lcd, string2);
}
*/

void atualizar_valor_sensor(int *historico_sensor, int valor)
{
    int aux;
    for (int i = 9; i < 0; i--)
    {
        if (i == 9)
        {
            aux = historico_sensor[i];
            historico_sensor[i] = valor;
        }
        else
            historico_sensor[i] = aux;
        aux = historico_sensor[i];
    }
}

void atualizar_historico()
{
    arquivo_historico = fopen("/arquivo_historico.txt", "w");
    if (arquivo_historico == NULL)
    {
        printf("Erro na leitura do arquivo");
    }
    else
    {
        char msg_json1[256];
        sprintf(msg_json1, "sensor1: {}");
        char msg_json2[256];
        char msg_json3[256];
    }
}

int main()
{

    // wiringPiSetup();

    // pinMode(btn1, INPUT);
    // pinMode(btn2, INPUT);
    // pinMode(btn3, INPUT);

    /**
     * Inicializa o display em modo 4 bits e 2 linhas.
     */
    // lcd = lcdInit(2, 16, 4, RS, E, D4, D5, D6, D7, 0, 0, 0, 0);
    // lcdHome(lcd);
    // lcdClear(lcd);

    // writeLCD("  Problema #03  ", "   IoT - MQTT   ");

    int rc;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_create(&client, ADDRESS, CLIENTID,
                      MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTClient_setCallbacks(client, NULL, NULL, on_message, NULL);

    // MQTT Connection parameters
    conn_opts.username = USERNAME;
    conn_opts.password = PASSWORD;
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Falha o conectar no MQTT, return code %d\n", rc);
        // writeLCD("Erro:", "Conexão MQTT");
        exit(-1);
    }

    // publish(client, SUBSCRIBE_TOPIC, "0x01", "0x04");
    MQTTClient_subscribe(client, NODE0, 0);
    MQTTClient_subscribe(client, NODE0_RESPOSTA, 0);
    //    MQTTClient_subscribe(client, APLICACAO_TEMPO_MEDICAO, 0);

    int opcao = 1;
    while (opcao != 0)
    {
        // Solicitação de dados para criar a requisição ao NodeMCU.
        printf("\nSelecione a requisição que deseja realizar");
        printf("\n1 - Solicitar a situação atual do NodeMCU\n2 - Solicitar o valor da entrada analógica\n3 - Solicitar o valor de uma das entradas digitais.\n4 - Controlar led da NodeMCU\n5 - Definir tempo de leitura da Node MCU\n--> ");
        scanf("%i", &opcao);
        sensor = 0;

        if (opcao < 1 || opcao > 5)
            continue;

        if (opcao == 3) // Comando da entrada digital.
        {
            printf("\nSelecione o sensor que deseja saber a medição\n"); // De 1 a 3
            scanf("%i", &sensor);
        }

        unsigned char tx_buffer[10]; // Array da mensagem a ser enviada.
        unsigned char *p_tx_buffer;
        p_tx_buffer = NULL;
        p_tx_buffer = &tx_buffer[0];
        // Envia o código do comando requisitado para o NodeMCU.
        switch (opcao)
        {
        case 1:
            // Envia o código da requisição de situação atual do sensor.
            // writeLCD("Situaçao atual");
            *p_tx_buffer++ = situacao_atual;
            break;

        case 2:
            // Envia o código da requisição do valor da entrada analógica.
            // writeLCD("Valor analogico");
            *p_tx_buffer++ = entrada_analogica;
            break;
        case 3:
            // Envia o código da requisição do valor de uma entrada digital.
            // writeLCD("Sensor ");
            *p_tx_buffer++ = sensor_digital; // Primeiro define-se o comando da requisição.
            *p_tx_buffer++ = sensor;         // Depois, define-se o endereço do sensor requisitado.
            break;
        case 4:
            // Envia o código da requisição para acender/apagar o led.
            // writeLCD("Controlar LED");
            *p_tx_buffer++ = controlar_led;
            break;
        case 5:
            printf("\nInforme o novo tempo de medição (em segundos):\n");
            scanf("%i", &tempo);
            publish(client, SBC_DEFINIR_TEMPO, &tempo);
            enviar_valores_sensores_aplicacao();
        default:
            continue;
        }
        if (opcao != 5)
        {
            publish(client, SBC_ENVIAR_COMANDO, &tx_buffer[0]);
        }
    }
}