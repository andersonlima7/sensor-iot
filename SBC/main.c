#include <stdio.h>
#include <string.h>
// #include <wiringPi.h>
// #include <lcd.h>
#include <MQTTClient.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * Configuração MQTT
 */
// #define ADDRESS "mqtt://10.0.0.101:1883"
// #define CLIENTID "alunoSBC"
#define USERNAME ""
#define PASSWORD ""
#define ADDRESS "mqtt://broker.mqttdashboard.com:1883"
#define CLIENTID "alunoSBC123"
// #define USERNAME "aluno"
// #define PASSWORD "@luno*123"
#define QOS 0
#define TIMEOUT 10000L

/*
 * Comandos de Solicitação
 */
const int situacao_atual = 0x03;    // código que requisita a situacao atual do NodeMCU.
const int entrada_analogica = 0x04; // código que requisita o valor da entrada analogica.
const int sensor_digital = 0x05;    // código que requisita o valor da entrada digital.
const int controlar_led = 0x06;     // código que requisita ligar/desligar o led.
const int alterar_tempo = 0x09;     // código que requisita alterar o tempo de medição dos sensores.
int sensor = 0;                     // Armazena a opcao do sensor selecionado pelo usuário.

/**
 * Valores atuais dos sensores
 */
int historico_sensor1[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int historico_sensor2[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int historico_sensor3[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int sensor1 = 0;
int sensor2 = 0;
int sensor3 = 0;
int tempo = 0;
FILE *arquivo_historico;

/*
 * Comandos de resposta
 */

const int node_ok = 0x00;
const int node_erro = 0x1F;
const int resposta_analogico = 0x01;
const int resposta_digital = 0x02;
const int led_on = 0x08;
const int led_off = 0x07;
const int tempo_alterado = 0x0A;

/*
 * Tópicos - Receptor
 */
#define NODE0 "TP02G03/SBC/node0/#"
#define SENSORES "TP02G03/SBC/node0/sensores"
#define NODE0_RESPOSTA "TP02G03/SBC/node0/resposta"
#define APLICACAO_TEMPO_MEDICAO "TP02G03/SBC/aplicacao/tempo"

/*
 * Tópicos - Publicador
 */
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

/*void slice(const char *str, char *result, size_t start, size_t end)
{
    strncpy(result, str + start, end - start);
}*/

/*
 * Prototipos de funcao
 */
void publish(MQTTClient client, char *topic, char *payload);
int on_message(void *context, char *topicName, int topicLen, MQTTClient_message *message);
void writeLCD(char *string1, char *string2);
void atualizar_historico_sensor(int *historico_sensor, int sensor, int valor);

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
    message.payloadlen = strlen(message.payload);
    message.qos = QOS;
    message.retained = 0;

    MQTTClient_deliveryToken token;
    MQTTClient_publishMessage(client, topic, &message, &token);
    MQTTClient_waitForCompletion(client, token, 1000L);
    printf("\nMensagem enviada para o topico %s enviada\n", topic);
}

void enviar_valores_sensores_aplicacao()
{
    char msg_json[256];
    sprintf(msg_json, "{sensor1: %i, sensor2: %i, sensor3: %i, tempo: %i}", sensor1, sensor2, sensor3, tempo);
    publish(client, SBC_ENVIAR_DADOS_SENSORES, msg_json);
}

/**
 * Função que é executada ao receber uma mensagem de um dado tópico.
 */
int on_message(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    char *payload = message->payload;
    if (strcmp(topicName, NODE0_RESPOSTA) == 0)
    {
        // print("Resposta: %i")

        if (*payload == node_ok)
        {
            writeLCD("NodeMCU: OK", " ");
        }
        else if (*payload == node_erro)
        {
            writeLCD("NodeMCU: ERRO", " ");
        }
        else if (payload[0] == resposta_analogico)
        {
            char msg[16];
            unsigned char highByte = payload[1];
            unsigned char lowByte = payload[2];
            sensor3 = (highByte * 256) + lowByte; // Converte os dois bytes em um único valor.
            sprintf(msg, "%d", sensor3);
            atualizar_historico_sensor(historico_sensor3, 3, sensor3);
            writeLCD("SENSOR ANALOGICO", msg);
        }
        else if (payload[0] == resposta_digital)
        {
            char msg[16];
            if (sensor == 16)
            {
                sensor1 = payload[1];
                atualizar_historico_sensor(historico_sensor1, 1, sensor1);
            }
            else if (sensor == 5)
            {
                sensor2 = payload[1];
                atualizar_historico_sensor(historico_sensor2, 2, sensor2);
            }
            sprintf(msg, "Sensor%d:%d", sensor, sensor1);
            writeLCD(msg, " ");
        }
        else if (*payload == led_on)
        {
            writeLCD("LED: ON", " ");
        }
        else if (*payload == led_off)
        {
            writeLCD("LED: OFF", " ");
        }
        else if (*payload == tempo_alterado)
        {
            char msg[16];
            sprintf(msg, "%d segundos", tempo);
            writeLCD("NOVO TEMPO:", msg);
        }
    }
    else if (strcmp(topicName, SENSORES) == 0)
    {
        sensor1 = payload[0];
        sensor2 = payload[1];
        unsigned char highByte = payload[2];
        unsigned char lowByte = payload[3];
        sensor3 = (highByte * 256) + lowByte;
        char msg1[16];
        char msg2[16];
        sprintf(msg1, "s1:%d   s2:%d", sensor1, sensor2);
        ;
        sprintf(msg2, "s3:%d", sensor3);
        ;
        writeLCD(msg1, msg2);
        atualizar_historico_sensor(historico_sensor1, 1, sensor1);
        atualizar_historico_sensor(historico_sensor2, 2, sensor2);
        atualizar_historico_sensor(historico_sensor3, 3, sensor3);
    }

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

/**
 * Escreve no display LCD.
 * @param string1 Mensagem da primeira linha.
 * @param string2 Mensagem da segunda linha.
 **/
void writeLCD(char *string1, char *string2)
{
    // lcdHome(lcd);
    // lcdClear(lcd);
    // lcdPosition(lcd, 0, 0);
    // lcdPuts(lcd, string1);
    // lcdPosition(lcd, 0, 1);
    // lcdPuts(lcd, string2);
    printf("\n%s\n", string1);
    printf("\n%s\n", string2);
}

void atualizar_historico_sensor(int *historico_sensor, int sensor, int valor)
{
    int aux;
    for (int i = 9; i >= 0; i--)
    {
        int aux_interno = 0;
        if (i == 9)
        {

            aux = historico_sensor[i];
            historico_sensor[i] = valor;
        }
        else
        {
            aux_interno = historico_sensor[i];
            historico_sensor[i] = aux;
            aux = aux_interno;
        }
    }
    printf("\n Histórico Sensor %d: \n", sensor);
    for (int i = 9; i >= 0; i--)
    {
        printf("Medição %i - valor: %i \n", abs(9 - i), historico_sensor[i]);
    }
}

void atualizar_historico()
{
    char msg[300] = {"\0"};
    char sensor1[100];
    char sensor2[100];
    char sensor3[100];
    sprintf(sensor1, "{\"sensor1\": {");
    for (int i = 0; i <= 9; i++)
    {
        char valor[10];
        if (i == 9)
            sprintf(valor, "\"%i\": %i ", (i + 1), historico_sensor1[i]);
        else
            sprintf(valor, "\"%i\": %i, ", (i + 1), historico_sensor1[i]);
        strcat(sensor1, valor);
    }
    sprintf(sensor2, "}, \"sensor2\": {");
    for (int i = 0; i <= 9; i++)
    {
        char valor[10];
        if (i == 9)
            sprintf(valor, "\"%i\": %i ", (i + 1), historico_sensor2[i]);
        else
            sprintf(valor, "\"%i\": %i, ", (i + 1), historico_sensor2[i]);
        strcat(sensor2, valor);
    }
    sprintf(sensor3, "}, \"sensor3\": {");
    for (int i = 0; i <= 9; i++)
    {
        char valor[10];

        if (i == 9)
            sprintf(valor, "\"%i\": %i ", (i + 1), historico_sensor3[i]);
        else
            sprintf(valor, "\"%i\": %i, ", (i + 1), historico_sensor3[i]);
        strcat(sensor3, valor);
    }
    strcat(msg, sensor1);
    strcat(msg, sensor2);
    strcat(msg, sensor3);
    char aux[15];
    sprintf(aux, "}, \"tempo\": %i}", tempo);
    strcat(msg, aux);
    printf("\n\n %s \n", msg);
    publish(client, SBC_ENVIAR_HISTORICO, msg);
}

/**
 * Reconecta-se ao broker MQTT (caso ainda não esteja conectado ou em caso de a conexão cair)
 * em caso de sucesso na conexão ou reconexão, o subscribe dos tópicos é refeito.
 */
void reconnectMQTT()
{

    while (!MQTTClient_isConnected(client))
    {
        printf("Tentando se conectar ao Broker MQTT:\n %s", ADDRESS);
        int returnCode;
        MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
        MQTTClient_create(&client, ADDRESS, CLIENTID,
                          MQTTCLIENT_PERSISTENCE_NONE, NULL);
        // Define as funções que devem ser chamadas na perca de conexão e no recebimento de mensagens.
        MQTTClient_setCallbacks(client, NULL, reconnectMQTT, on_message, NULL);

        // MQTT Connection parameters
        conn_opts.username = USERNAME;
        conn_opts.password = PASSWORD;
        conn_opts.keepAliveInterval = 20;
        conn_opts.cleansession = 1;
        returnCode = MQTTClient_connect(client, &conn_opts); // Tenha se conectar ao broker.
        if (returnCode == MQTTCLIENT_SUCCESS)
        {
            printf("Conectado com sucesso ao BROKER");
            MQTTClient_subscribe(client, NODE0_RESPOSTA, QOS);
            MQTTClient_subscribe(client, SENSORES, QOS);
            //    MQTTClient_subscribe(client, APLICACAO_TEMPO_MEDICAO, 0);
        }
        else
        {
            printf("Falha o conectar no MQTT, codigo %d\n", returnCode);
            writeLCD("Erro:", "Conexão MQTT");
            printf("Tentando se reconectar em 2s...");
            sleep(2);
        }
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

    reconnectMQTT();

    int opcao = 1;
    while (opcao != 0)
    {
        reconnectMQTT();
        // Solicitação de dados para criar a requisição ao NodeMCU.
        printf("\nSelecione a requisição que deseja realizar");
        printf("\n1 - Solicitar a situação atual do NodeMCU\n2 - Solicitar o valor da entrada analógica\n3 - Solicitar o valor de uma das entradas digitais.\n4 - Controlar led da NodeMCU\n5 - Definir tempo de leitura da Node MCU\n--> ");
        scanf("%i", &opcao);
        sensor = 0;

        if (opcao == 3) // Comando da entrada digital.
        {
            printf("\nSelecione o sensor que deseja saber a medição\n"); // De 1 a 3
            scanf("%i", &sensor);
        }

        unsigned char message[10]; // Array da mensagem a ser enviada.
        unsigned char *p_message;
        p_message = NULL;
        p_message = &message[0];
        // Envia o código do comando requisitado para o NodeMCU.
        switch (opcao)
        {
        case 1:
            // Envia o código da requisição de situação atual do sensor.
            // writeLCD("Situaçao atual");
            *p_message++ = situacao_atual;
            break;

        case 2:
            // Envia o código da requisição do valor da entrada analógica.
            // writeLCD("Valor analogico");
            *p_message++ = entrada_analogica;
            break;
        case 3:
            // Envia o código da requisição do valor de uma entrada digital.
            // writeLCD("Sensor ");
            *p_message++ = sensor_digital; // Primeiro define-se o comando da requisição.
            *p_message++ = sensor;         // Depois, define-se o endereço do sensor requisitado.
            break;
        case 4:
            // Envia o código da requisição para acender/apagar o led.
            // writeLCD("Controlar LED");
            *p_message++ = controlar_led;
            break;
        case 5:
            printf("\nInforme o novo tempo de medição (em segundos):\n");
            scanf("%i", &tempo);
            *p_message++ = alterar_tempo;
            *p_message++ = tempo;
            enviar_valores_sensores_aplicacao();
            break;
        default:
            continue;
        }
        publish(client, SBC_ENVIAR_COMANDO, &message);
    }
}
