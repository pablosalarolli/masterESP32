// Código do MESTRE
/*Autores:
  Leonardo Gonçalves
  Lucas Mantuan Ayres
  Pablo France Salarolli
*/
#include "gainProtocol.h"

#define MASTER_ADDR 0x0F
#define SLAVE_1 0x01
#define SLAVE_1 0x02
#define SERIAL_PORT_PC 0
#define SERIAL_PORT_BUS 2

#define LED_SPVP1 22
#define LED_SPVP2 23

#define RX_PIN 16
#define TX_PIN 17
#define SERIAL_CTRL 2


enum masterStates {
  AGUARDANDO,
  RECEBE_MSG_PC,
  RESPONDE_PC,
  ENVIA_MSG_SLAVE,
  AGUARDA_RESPOSTA_SLAVE,
  TRATA_RESPOSTA_SLAVE,
  ATUALIZA_LEDS,
  AQUISICAO_CONTINUA,
  REPORTA_ERRO
};

byte addr = 0, opcode = 0, opcode_slave = 0;
int dado = 0;


enum masterStates estado = AGUARDANDO, estado_anterior = AGUARDANDO;
bool ambos = 0; // flag para operações em ambos escravos
long dt = 1000, timeout = 3000;
unsigned long tAnt = 0, tAgora = 0, tAntTimeOut = 0, tAgoraTimeOut = 0;

void setup() {
  // Note the format for setting a serial port is as follows: Serial2.begin(baud-rate, protocol, RX pin, TX pin);
  pinMode(2, OUTPUT);
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
  digitalWrite(2, 1);
}

void loop() {


  switch (estado) {
    case AGUARDANDO:

      if (Serial.available())
        estado = RECEBE_MSG_PC;

      tAgora = millis();
      if ((tAgora - tAnt) >= dt) {
        tAnt = tAgora;
        opcode = 0b0100; //verifica se SP = VP ATUALIZAR OPCODE!!!
        addr = SLAVE_1;
        ambos = 1;
        estado = ENVIA_MSG_SLAVE;
        proximo_estado = ATUALIZA_LEDS;
      }
      break;

    case RECEBE_MSG_PC:
      flag = recebeMensagem(SERIAL_PORT_PC, &addr, &opcode, &dado);
      if (flag)
        estado = REPORTA_ERRO;
      else
        estado = trataMSGPC();
      break;

    case ENVIA_MSG_SLAVE:
      enviaMensagem(SERIAL_PORT_BUS, addr, opcode, dado);
      estado = AGUARDA_RESPOSTA_SLAVE;
      break;

    case AGUARDA_RESPOSTA_SLAVE:
      if (Serial2.available()) {
        estado = TRATA_RESPOSTA_SLAVE;
      }

      tAgoraTimeOut = millis();
      if ((tAgoraTimeOut - tAntTimeOut) >= TimeOut) {
        tAntTimeOut = tAgoraTimeOut;
        flag = 5; // flag de timeout
        estado = REPORTA_ERRO;
      }
      break;

    case TRATA_RESPOSTA_SLAVE:
      flag = recebeMensagem(SERIAL_PORT_BUS, &addr, &opcode, &dado);
      if (!flag) {
        estado = proximo_estado;
      }
      else
        estado = REPORTA_ERRO;
      break;

    case ATUALIZA_LEDS:
      atualizaLEDSP(addr, dado); // Atualiza o Led do escravo em addr com o valor de dado
      if (ambos) {
        ambos = 0;
        addr = SLAVE_2;
        estado = ENVIA_MSG_SLAVE;
      }
      else
        estado = AGUARDANDO;
      break;
    default:
      estado = AGUARDANDO;
      break;
  }

  //  enviaMensagem(0x02, 0x01, 0x02F5);
  delay(1000);
}
