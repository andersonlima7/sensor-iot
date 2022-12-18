# Problema #3 – IoT: A Internet das Coisas

Este projeto tem por objetivo o aprimoramento do sistema descrito em [Problema #2 – Interfaces de Entrada e Sáida](https://github.com/andersonlima7/serial-sensor/), substituindo a comunicação que antes era feita por meio da UART pelo protocolo MQTT (<i>Message Queue Telemetry Transport<i/>).

## Sumário

- [Instalação](#instalação)
- [Introdução](#introdução)
- [Objetivo](#objetivo)
- [Metodologia](#metodologia)
- [Desenvolvimento](#desenvolvimento)
- [Testes e resultados](#testes)
- [Referências](#referências)

## Instalação

Para utilizar o sistema, deve-se possuir uma Orange Pi para a execução do código presente em [SBC](/SBC/main.c), pode ser utilizado em uma Raspberry Pi, porém os pinos para se utilizar o display LCD devem ser alterados para os correspondentes, além disso deve-se possuir uma ESP8266 para a execução do código que está em [NodeMCU](/NodeMCU/main/main.ino) e o [Arduino IDE](https://www.arduino.cc/en/software) para o upload do código nessa segunda placa.

Para clonar o projeto, utilize o comando

```git
git clone https://github.com/andersonlima7/sensor-iot.git
```

### SBC

Para executar o código da SBC, acesse a pasta SBC e rode os seguintes comandos:

```git
make
sudo ./main
```

### NodeMCU

Para executar o código do NodeMCU, utilize o Arduino IDE para fazer upload do código [main](/NodeMCU/main/main.ino). Ao faze-lo, o código já será executado automaticamente.

### Interface WEB

A interface WEB bem como suas instruções de uso pode ser acessada nesse [repositório](https://github.com/antonyaraujo/sensor-iot-app).

## Introdução

O ramo da Internet das Coisas (IoT) está em constante expansão, cada vez mais diversos equipamentos e produtos do dia a dia estão sendo inseridos em um ambiente de rede, por essa razão faz-se necessário entender como os equipamentos utilizados para propiciar a comunicação são programados e como inseri-lós no ambiente em rede, considerando a necessidade de integração com sistemas de controle, que normalmente são gerenciados por entes centrais, como SBC (Single Board Computer).

MQTT (Message Queuing Telemetry Transport) é um protocolo de comunicação máquina para máquina (M2M - Machine to Machine) com foco em IoT que funciona em cima do protocolo TCP/IP. Um sistema MQTT se baseia no paradigma Publish-Subscribe, em que clientes podem tanto publicar postagens para outros clientes e se inscrever em postagens de outros clientes.

O padrão Publish-Subscribe é muito parecido com o padrão Observer, porém nesse caso adicionamos o papel do Broker, que é responsável por filtrar as mensagens e saber exatamente para quem enviar. Dessa forma, o publisher e subscriber não precisam se conhecer diretamente e apenas precisam conhecer o Broker, que é quem fará a notificação da mudança de estados e enviará essa informação para aqueles que tiverem inscritos no tópico referenciado. Por fim, o publish precisa se preocupar apenas com enviar as informações e estabelecer a conexão exclusivamente com o Broker.

O sistema construído utiliza o protocolo MQTT para integrar o NodeMCU (ESP2866) com a SBC (Orange Pi), e a aplicação WEB feita em react para a exibição dos dados dos sensores.

## Objetivo

Criar um protótipo de um sistema IoT, por meio do protocolo MQTT, que integre e sincronize três sistemas diferentes. O NodeMCU realiza as medições dos sensores e os envia para a SBC, essa tem a responsabilidade de armazenar as últimas 10 medições de cada sensor e exibir os dados no display LCD, ademais, a SBC envia os dados para a exibição na interface WEB.

## Metodologia

Como esse novo problema é um aprimoramento do [Problema #2 – Interfaces de Entrada e Sáida](https://github.com/andersonlima7/serial-sensor/), a estrutura básica da comunicação SBC-NodeMCU já estava construída, o que tinha que ser feito era a substituição da comunicação via UART pela comunicação através do MQTT. Desse modo, o primeiro passo foi fazer essa substituição de formas de transmissão de dados, os tópicos utilizados para a troca de mensagens dos sistemas são descritos em [protocolo](#protocolo).

Posteriormente, com a comunicação SBC-NodeMCU já funcionando, o foco passou em desenvolver em simultâneo a interface local e a interface WEB. E por fim foi feita a integração dos três sistemas, com toda a comunicação sendo feita pelo MQTT.

## Desenvolvimento

### Protocolo

A implementação do protocolo MQTT é o grande cerne de todo o sistema, desse modo fez-se necessário uma estruturação precisa dos tópicos utilizados para a comunicação das entidades presentes.

Os tópicos presentes em Subscriber são os que a entidade se inscreve, isto é, são os tópicos que ela aguardará por mensagens. Por outro lado, os tópicos na seção Publisher são os tópicos em que o sistema envia mensagens para outros sistemas que sejam inscritos nesse tópico.

Todos os tópicos utilizados possuem o prefixo `TP02G03/` para assegurar que o sistema de outras pessoas não interferiam na comunicação do sistema construído.

#### Comunicação SBC ↔️ NodeMCU

Os tópicos utilizados para realizar a transmissão de dados entre a SBC e o NodeMCU são descritos abaixo.

Visão: SBC

Subscriber:
<b>TP02G03/SBC/node0/resposta</b>
<b>TP02G03/SBC/node0/sensores</b>

Publisher:
<b>TP02G03/SBC/node0/comando</b>
<b>TP02G03/SBC/node0/tempo</b>

NodeMCU

Subscriber:
<b>TP02G03/SBC/node0/comando</b>
<b>TP02G03/SBC/node0/tempo</b>

Publisher:
<b>TP02G03/SBC/node0/resposta</b>
<b>TP02G03/SBC/node0/sensores</b>

A SBC publica comandos para a NodeMCU no tópico `/comando`, estes comandos podem ser vistos na tabela abaixo. Além desse tópico, a SBC altera o tempo em que a NodeMCU realiza as medições dos sensores por meio do tópico `/tempo`. O NodeMCU se inscreve nesses tópicos e os responde para o SBC via tópico `/resposta` A SBC se inscreve no tópico `resposta` para aguardar os comandos de resposta do NodeMCU. Já o tópico `/sensores` é usado para a SBC receber os dados das medições automáticas dos sensores que são publicados pela NodeMCU.

<table id='tabela'>
<thead>
  <tr>
    <th>Comando (SBC -&gt; NODEMCU)</th>
    <th>Descrição</th>
    <th>Resposta (NODEMCU -&gt; SBC)</th>
    <th>Descrição</th>
  </tr>
</thead>
<tbody>
  <tr>
    <td rowspan="2">0x03</td>
    <td rowspan="2">Solicita a situação atual do NodeMCU</td>
    <td>0x00</td>
    <td>NodeMCU OK</td>
  </tr>
  <tr>
    <td>0x1F</td>
    <td>NodeMCU ERRO</td>
  </tr>
  <tr>
    <td>0x04</td>
    <td>Solicita o valor da entrada analógica</td>
    <td>0x01 + Dado (~0 a 1024)</td>
    <td>Valor da entrada analógica</td>
  </tr>
  <tr>
    <td>0x05 + Endereço do Sensor</td>
    <td>Solicita o valor da entrada digital</td>
    <td>0x02 + Dado(0 ou 1)</td>
    <td>Valor da entrada digital</td>
  </tr>
  <tr>
    <td rowspan="2">0x06</td>
    <td rowspan="2">Controla o LED(Liga/Desliga)</td>
    <td>0x07</td>
    <td>LED ligado.</td>
  </tr>
  <tr>
    <td>0x08</td>
    <td>LED desligado.</td>
  </tr>
</tbody>
</table>

Deve-se destacar que o código de erro foi configurado para ser enviado quando o NodeMCU perde a conexão com broker, dessa forma a SBC recebe a informação que o NodeMCU está desconectado e não pode receber e enviar mensagens. Isso foi configurado na conexão do NodeMCU com o broker, as linha de código abaixo, que estão presentes no arquivo [main.ino](/NodeMCU/main/main.ino) são responsável por fazer essa configuração.

```c
#define PUBLISH_TOPIC_RESPONSE  "TP02G03/SBC/node0/resposta"

char WILL_MESSAGE[] = {0x1F};

MQTT.connect(CLIENTID, USER, PASSWORD, PUBLISH_TOPIC_RESPONSE, QOS, false, WILL_MESSAGE`

```

Isto é, se a NodeMCU perder a conexão, a mensagem `0x1F` é enviada para os inscritos no tópico `/resposta`, ou seja, é enviada para SBC.

#### Comunicação SBC ↔️ Interface WEB

Os tópicos utilizados para realizar a transmissão de dados entre a SBC e a interface WEB são descritos a seguir.

Visão: SBC

Subscriber:
<b>TP02G03/SBC/aplicacao/tempo_local
</b>

Publisher:
<b>TP02G03/SBC/aplicacao/tempo_remoto</b>
<b>TP02G03/SBC/aplicacao/historico</b>

Interface WEB

Subscriber:
<b>TP02G03/SBC/aplicacao/tempo_remoto</b>
<b>TP02G03/SBC/aplicacao/historico</b>

Publisher:
<b>TP02G03/SBC/aplicacao/tempo_local</b>

Agora se tratando da comunicação entre a SBC e a interface remota, a SBC como centralizadora dos dados no sistema proposto envia o histórico das últimas 10 medições de cada sensor para a aplicação WEB por meio do tópico `/historico`. Além disso, caso o tempo de medições dos sensores seja alterado, como esse valor é exibido na interface WEB, para manter a sincronização e unidade dos dados, a SBC envia o novo valor do tempo de medição para a interface remova através do tópico `/tempo_remoto`, atualizando o valor exibido ao usuário. Do mesmo modo, caso o tempo de medições seja alterado na interface WEB, a aplicação envia pelo tópico `/tempo_local` o novo valor do tempo para a SBC e esta, por sua vez, envia para o NodeMCU o novo valor do tempo de medições.

### Interface Local

Após a implementação do protocolo, a interface de interação entre a SBC e o usuário foi construída. A interface baseia-se em utilizar uma espécie de máquina de estados para a exibição das opções de uso do sistema no display LCD. Os botões pinados na SBC foram usados para o usuário poder escolher as opções desejadas, basicamente o botão do pino 19 retrocede uma opção, o botão do pino 23 confirma a opção escolhida e o botão do pino 25 avança uma opção.

Os estados definidos são:

- MENU
- ESCOLHER_SENSOR
- ALTERAR_TEMPO
- HISTORICO
- EXIBIR_HISTORICO
- ESPERA

Durante o estado de menu, as opções que o usuário pode selecionar são exibidas, sendo elas:

1. Status ESP
2. Analogico
3. Digital
4. Controlar LED
5. Tempo medicao
6. Historico
7. Sair

A cinco primeiras mandam mensagens para a SBC solicitando os comandos correspondentes, comando já detalhados na [tabela de comandos](#tabela). A opção **Digital** altera o estado da interface para o estado **ESCOLHER_SENSOR**, nesse estado uma lista de sensores são exibidas para o usuário poder escolher qual sensor quer saber a medição do valor digital. A opção **Tempo medicao** altera o estado para **ALTERAR_TEMPO**, nesse estado o usuário pode alterar o tempo de medição dos sensores utilizando os botões conectados na SBC, o botão do pino 19 é usado para diminuir o valor do tempo e do pino 25 é usado para aumentar o tempo. A opção **Historico** é única que não manda uma mensagem para o NodeMCU, pois corresponde a exibir o histórico de 10 medições dos sensores. Essa opção altera o estado da interface para **HISTORICO**, nesse estado o usuário pode escolher qual dos sensores ela quer visualizar o histórico. Após escolher o sensor que se quer ver o histórico, o estado altera-se para EXIBIR_HISTORICO, nesse estado, as linhas do histórico do sensor escolhido deveriam ser exibidas, porém por falta de tempo <span style="color:#DC143C">essa função não funcionou corretamente</span>, desse modo o histórico é exibido no terminal, sendo assim, esse estado se faz desnecessário nessa condição. Após selecionar cada uma das opções, para a exibição das respostas vindo da NodeMCU, a interface vai para o estado de espera, nesse estado a interface exibe os dados de resposta até que o usuário pressione o botão de avançar ou de retroceder, caso o usuário pressione o botão de confirmação uma nova requisição é feita para a NodeMCU. Por fim, foi incluída a opção **Sair** que finaliza corretamente a execução do programa.

## Testes

Alguns testes que foram feitos para verificar o funcionamento do sistema são descritos a seguir:

- A comunicação entre SBC e NodeMCU via protocolo MQTT foi testada juntamente com os requisitos do [Problema #2 – Interfaces de Entrada e Sáida](https://github.com/andersonlima7/serial-sensor/). O funcionamento foi de 100%, nenhum entrave nessa questão, as duas entidades trocam mensagens entre si e cada um dos quesitos solicitados foram devidamente atendidos.

- A interface local foi testada, foi verificado se as opções eram exibidas corretamente, na ordem correta e se as respostas das requisições eram apresentadas ao usuários de forma precisa. Nessa questão o problema pecou ao exibir o histórico do sensor escolhido para o usuário, como já foi dito anteriormente para cumprir esse requisito o histórico teve que ser exibido no terminal. Desse modo o funcionamento da interface local se dá em 90%.

- A conexão com a interface remota foi igualmente testada juntamente com seu funcionamento. Infelizmente, alguns problemas foram notados nesse caso: A interface remota apresenta problemas de receber dados dos sensores e de enviar o novo tempo de medição que o usuário escolhe para a SBC. Como as vezes ela recebe os dados dos sensores e as vezes não, essa questão fica bem comprometida, porém, ao selecionar um novo tempo de medição na SBC, esse tempo é atualizado de forma correta na interface remota. Dessa forma, o funcionamento desse caso é de cerca de 50%, sendo o principal ponto de aprimoramento desse sistema.

## Referências

NERI, Renan. LOMBA, Matheus. BULHÕES, Gabriel. MQTT. 2019. Disponível em: <https://www.gta.ufrj.br/ensino/eel878/redes1-2019-1/vf/mqtt/>. Acesso em 18 Dez 2022.

```

```
