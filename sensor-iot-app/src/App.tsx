import React, { useEffect, useState } from "react";
import "./App.css";
import {
  Box,
  Card,
  CardBody,
  Text,
  Select,
  CardHeader,
  TableContainer,
  Table,
  TableCaption,
  Thead,
  Tr,
  Th,
  Td,
  Tbody,
  Tfoot,
  Input,
  Button,
  InputRightElement,
  InputGroup,
  Flex,
  HStack,
  Tag,
  TagLabel,
  TagCloseButton,
} from "@chakra-ui/react";
import { Container, Content, Title } from "../src/globalStyles";
import mqtt, { IClientOptions } from "mqtt/dist/mqtt";
import { Menu, MenuItem, Sidebar, SubMenu } from "react-pro-sidebar";
import { forEach } from "lodash";
import SensorsChart from "../src/SensorsChart";
import { ClientRequest } from "http";

//const host = "ws://broker.mqttdashboard.com:8000/mqtt";
const host = "ws://10.0.0.101:9001/mqtt";
const clientId = "mqttjs_" + Math.random().toString(16).substr(2, 8);
const options: IClientOptions = {
  clientId: clientId,
  protocolId: "MQTT",
  keepalive: 6000,
  username: "aluno",
  password: "@luno*123",
};
const optionsPublicador: IClientOptions = {
  clientId: clientId + 1,
  protocolId: "MQTT",
  keepalive: 600,
  username: "aluno",
  password: "@luno*123",
};

function App() {
  // App variables
  const [selectedSensor, setSelectedSensor] = useState("1");
  const [connectionStatus, setConnectionStatus] = useState(false);
  const [menu, setMenu] = useState("1");

  const [msg, setMsg] = useState("");
  const [historico, setHistorico] = useState([]);
  const [historico1, setHistorico1] = useState([]);
  const [historico2, setHistorico2] = useState([]);
  const [historico3, setHistorico3] = useState([]);
  const [tempoHistorico1, setTempoHistorico1] = useState([]);
  const [tempoHistorico2, setTempoHistorico2] = useState([]);
  const [tempoHistorico3, setTempoHistorico3] = useState([]);
  const [chart, setChart] = useState([]);
  const [tempo, setTempo] = useState(30);
  const [novoTempo, setNovoTempo] = useState(0);
  const [conectado, setConectado] = useState(false);
  const mymsg = { msg: "minha mensagem" };
  const [lock, setLock] = useState(true);

  useEffect(() => {
    if (lock) {
      let tempoA: any = [];
      for (let i = 0; i < 10; i += 1) {
        tempoA[i] = new Date().toLocaleString();
      }
      setTempoHistorico1(tempoA);
      setTempoHistorico2(tempoA);
      setTempoHistorico3(tempoA);
      setLock(false);
    }
  }, []);

  useEffect(() => {
    const client: mqtt.MqttClient = mqtt.connect(host, options);
    client.on("connect", () => setConnectionStatus(true));
    client.on("connect", () => setConectado(true));
    client.on("offline", () => setConectado(false));
    client.on("disconnect", () => setConectado(false));
    client.on("close", () => setConectado(false));
    client.on("error", () => setConectado(false));

    console.log("chegcando useEffect");
    client.on("connect", function () {
      client.subscribe("TP02G03/SBC/aplicacao/historico");
      client.subscribe("TP02G03/SBC/aplicacao/tempo_remoto");
      //client.publish("TP02G03/SBC/aplicacao/historico", JSON.stringify(mymsg));
    });

    client.on("message", (topic: any, message: any, packet: any) => {
      console.log("chegou algo");
      console.log(topic);
      if (topic === "TP02G03/SBC/aplicacao/historico") {
        let new_msg = new TextDecoder("utf-8").decode(message);
        setMsg(new_msg);
        //console.log(new_msg);
        //console.log(msg);
        try {
          let sensor1 = JSON.parse(new_msg)["sensor1"];
          let sensor2 = JSON.parse(new_msg)["sensor2"];
          let sensor3 = JSON.parse(new_msg)["sensorA"];
          let sensor1JSON = sensor1 && Object.values(sensor1);
          let sensor2JSON = sensor2 && Object.values(sensor2);
          let sensor3JSON = sensor3 && Object.values(sensor3);

          let tempoA: any = tempoHistorico1;

          let aux = 0;
          for (let i = 9; i >= 0; i -= 1) {
            let aux_interno = 0;
            if (i === 9) {
              aux = tempoA[i];
              tempoA[i] = new Date().toLocaleString();
            } else {
              aux_interno = tempoA[i];
              tempoA[i] = aux;
              aux = aux_interno;
            }
          }

          tempoA[9] = new Date().toLocaleString();
          setTempoHistorico1(tempoA);

          tempoA = tempoHistorico2;

          aux = 0;
          for (let i = 9; i >= 0; i -= 1) {
            let aux_interno = 0;
            if (i === 9) {
              aux = tempoA[i];
              tempoA[i] = new Date().toLocaleString();
            } else {
              aux_interno = tempoA[i];
              tempoA[i] = aux;
              aux = aux_interno;
            }
          }

          tempoA[9] = new Date().toLocaleString();
          setTempoHistorico2(tempoA);

          tempoA = tempoHistorico3;

          aux = 0;
          for (let i = 9; i >= 0; i -= 1) {
            let aux_interno = 0;
            if (i === 9) {
              aux = tempoA[i];
              tempoA[i] = new Date().toLocaleString();
            } else {
              aux_interno = tempoA[i];
              tempoA[i] = aux;
              aux = aux_interno;
            }
          }

          tempoA[9] = new Date().toLocaleString();
          setTempoHistorico3(tempoA);

          setHistorico1(sensor1JSON);
          setHistorico2(sensor2JSON);
          setHistorico3(sensor3JSON);
          //setHistorico1(array);
          //console.log(new_msg);
        } catch (err) {
          console.log("Deu erro amigao");
          console.log(err);
        }
      }
      if (topic === "TP02G03/SBC/aplicacao/tempo_remoto") {
        console.log("chegou tempo");
        let new_msg = new TextDecoder("utf-8").decode(message);
        let tempoValor = JSON.parse(new_msg)["tempo"];
        setMsg(new_msg);
        console.log(msg);
        try {
          setTempo(tempoValor);
        } catch (er) {
          console.log("Erro de: ", er);
        }
      }
    });
  }, []);

  function enviarTempo(novoTempo: number) {
    const publicador: mqtt.MqttClient = mqtt.connect(host, optionsPublicador);
    publicador.on("connect", function () {
      publicador.publish(
        "TP02G03/SBC/aplicacao/tempo_local",
        novoTempo.toString()
      );
      publicador.end();
    });
  }

  const onMessage = (callBack: any) => {};

  return (
    <Container>
      <Box
        left="0"
        display="grid"
        gridColumn="2"
        gridTemplateColumns="25fr 75fr"
        top="0"
        width="100%"
        gap="48px"
      >
        <Content>
          <Text
            fontFamily="Monospace"
            fontSize="24px"
            color="lightblue"
            ml="12px"
            padding="16px 16px"
          >
            Sensor IoT
          </Text>
          <Sidebar width="100%">
            <Menu
              menuItemStyles={{
                button: ({ level, active, disabled }) => {
                  // only apply styles on first level elements of the tree
                  if (level === 0)
                    return {
                      color: active ? "#282c34" : "lightblue",
                      backgroundColor: active ? "white" : "#282c34",
                      _hover: {
                        color: "#282c34",
                        bg: "white",
                      },
                    };
                },
              }}
            >
              <MenuItem onClick={() => setMenu("1")}> Sensores </MenuItem>
              <MenuItem onClick={() => setMenu("2")}> Tempo </MenuItem>
              <MenuItem onClick={() => setMenu("3")}> Sobre </MenuItem>
            </Menu>
          </Sidebar>
        </Content>
        <Box width="90%" display={menu === "1" ? "block" : "none"}>
          <HStack spacing={4} justifyItems="center">
            <Title fontFamily="Monospace">Sensor IoT</Title>
            <Tag
              size="lg"
              key="lg"
              borderRadius="full"
              variant="solid"
              colorScheme={conectado ? "green" : "red"}
            >
              <TagLabel>{conectado ? "Conexão OK!" : "Não Conectado"}</TagLabel>
            </Tag>
          </HStack>
          <Text color="white">Selecione um sensor abaixo</Text>
          <Select
            value={selectedSensor}
            onChange={(e) => setSelectedSensor(e.target.value)}
            bg="whiteAlpha.900"
          >
            <option value="1">Sensor 1</option>
            <option value="2">Sensor 2</option>
            <option value="3">Sensor 3</option>
            <option value="4">Sensor 4</option>
            <option value="5">Sensor 5</option>
            <option value="6">Sensor 6</option>
            <option value="7">Sensor 7</option>
            <option value="8">Sensor 8</option>
            <option value="0">Sensor Analógico</option>
          </Select>
          <Card
            display={selectedSensor === "1" ? "block" : "none"}
            bg="whiteAlpha.900"
            mt="1em"
          >
            <CardHeader>Sensor 1</CardHeader>
            <hr />
            <CardBody>
              <TableContainer width="100%" pl="6em" pr="6em">
                <Table variant="striped">
                  <TableCaption>Valores de Medição dos Sensores</TableCaption>
                  <Thead>
                    <Tr alignItems="left">
                      <Th> Medição</Th>
                      <Th> Valor </Th>
                      <Th> Horário</Th>
                    </Tr>
                  </Thead>
                  <Tbody textAlign="right">
                    {historico1 !== null
                      ? historico1.map((sensor, key) => (
                          <Tr>
                            <Td>{key + 1}</Td>
                            <Td>{sensor}</Td>
                            <Td>{tempoHistorico1[key]}</Td>
                          </Tr>
                        ))
                      : "Os dados dos sensores não foram carregados"}
                  </Tbody>
                </Table>
                <SensorsChart chartData={historico1} />
              </TableContainer>
            </CardBody>
          </Card>

          <Card
            display={selectedSensor === "2" ? "block" : "none"}
            bg="whiteAlpha.900"
            mt="1em"
          >
            <CardHeader>Sensor 2</CardHeader>
            <hr />
            <CardBody>
              <TableContainer width="100%" pl="6em" pr="6em">
                <Table variant="striped">
                  <TableCaption>Valores de Medição dos Sensores</TableCaption>
                  <Thead>
                    <Tr alignItems="left">
                      <Th> Medição</Th>
                      <Th> Valor </Th>
                      <Th> Horário</Th>
                    </Tr>
                  </Thead>
                  <Tbody textAlign="right">
                    {historico2 !== null
                      ? historico2.map((sensor, key) => (
                          <Tr>
                            <Td>{key + 1}</Td>
                            <Td>{sensor}</Td>
                            <Td>{tempoHistorico2[key]}</Td>
                          </Tr>
                        ))
                      : "Os dados dos sensores não foram carregados"}
                  </Tbody>
                </Table>
                <SensorsChart chartData={historico2} />
              </TableContainer>
            </CardBody>
          </Card>

          <Card
            display={selectedSensor === "3" ? "block" : "none"}
            bg="whiteAlpha.900"
            mt="1em"
          >
            <CardHeader>Sensor 3</CardHeader>
            <hr />
            <CardBody>
              <TableContainer width="100%" pl="6em" pr="6em">
                <Table variant="striped">
                  <TableCaption>Valores de Medição dos Sensores</TableCaption>
                  <Thead>
                    <Tr alignItems="left">
                      <Th> Medição</Th>
                      <Th> Valor </Th>
                      <Th> Horário</Th>
                    </Tr>
                  </Thead>
                  <Tbody textAlign="right">
                    {historico3 !== null
                      ? historico3.map((sensor, key) => (
                          <Tr>
                            <Td>{key + 1}</Td>
                            <Td>{sensor}</Td>
                            <Td>{tempoHistorico3[key]}</Td>
                          </Tr>
                        ))
                      : "Os dados dos sensores não foram carregados"}
                  </Tbody>
                </Table>
                <SensorsChart chartData={historico3} />
              </TableContainer>
            </CardBody>
          </Card>
        </Box>
        <Box display={menu === "2" ? "block" : "none"}>
          <Card bg="whiteAlpha.900" mt="3em" w="80%">
            <CardHeader>
              <Text>Alteração do tempo de captura de dados</Text>
            </CardHeader>
            <CardBody>
              <Text>Tempo atual: {tempo.toString()}</Text>
              <Text mt="30px" fontSize="14px">
                Tempo de captura de dados:
              </Text>
              <Flex width="50%" alignItems="right">
                <InputGroup mr="2em">
                  <InputRightElement>s</InputRightElement>
                  <Input
                    id="inputTime"
                    name="inputTime"
                    className="inputTime"
                    type="number"
                    onChange={(e) => setNovoTempo(e.target.valueAsNumber)}
                  />
                </InputGroup>
                <Button
                  colorScheme="red"
                  w="10em"
                  onClick={() => enviarTempo(novoTempo)}
                >
                  Alterar tempo
                </Button>
              </Flex>
            </CardBody>
          </Card>
        </Box>
      </Box>
    </Container>
  );
}

export default App;
