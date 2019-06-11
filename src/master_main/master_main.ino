// Código do MESTRE
/*Autores:
  Leonardo Gonçalves
  Lucas Mantuan Ayres
  Pablo France Salarolli
*/
#include "gainProtocolMaster.h"

#define MASTER_ADDR 0x0F
#define SLAVE_1 0x01
#define SLAVE_2 0x02
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
  RETRANSMISSAO,
  REPORTA_ERRO
};

byte addr = 0, opcode = 0, opcode_slave = 0;
int dado = 0, flag = 0;
byte addrBackup = 0, opcodeBackup = 0;
int dadoBackup = 0, contRetransmissao = 0;

enum masterStates estado = AGUARDANDO, proximo_estado = AGUARDANDO;
bool ambos = 0; // flag para operações em ambos escravos
bool aquisicao_continua_flag = 0;
long dt = 1000, timeout = 3000;
unsigned long tAnt = 0, tAgora = 0, tAntTimeOut = 0, tAgoraTimeOut = 0;

void setup() {
  // Note the format for setting a serial port is as follows: Serial2.begin(baud-rate, protocol, RX pin, TX pin);
  pinMode(2, OUTPUT);
  pinMode(MAX485_RE_NEG, OUTPUT);
  habilitaReceberDoBarramento();
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
  digitalWrite(2, 1);
}

void loop() {


  switch (estado) {
    case AGUARDANDO:
      tAgora = millis();
      if (Serial.available()) {
        estado = RECEBE_MSG_PC;
      }
      else if ((tAgora - tAnt) >= dt) {
        tAnt = tAgora;
        opcode = 0b0100; //verifica se SP = VP ATUALIZAR OPCODE!!!
        addr = SLAVE_1;
        ambos = 1;
        estado = ENVIA_MSG_SLAVE;
        proximo_estado = ATUALIZA_LEDS;
      }
      else if (aquisicao_continua_flag)
        estado = AQUISICAO_CONTINUA;
      break;

    case RECEBE_MSG_PC:
      flag = recebeMensagem(SERIAL_PORT_PC, &addr, &opcode, &dado);
      if (!flag) {
        proximo_estado = RESPONDE_PC;
        trataMSGPC();
      }
      else
        estado = REPORTA_ERRO;
      break;

    case RESPONDE_PC:
      enviaMensagem(SERIAL_PORT_PC, addr, opcode, dado);
      if (ambos) {
        ambos = 0;
        addr = SLAVE_2;
        estado = ENVIA_MSG_SLAVE;
      }
      else
        estado = AGUARDANDO;
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
      if ((tAgoraTimeOut - tAntTimeOut) >= timeout) {
        tAntTimeOut = tAgoraTimeOut;
        flag = 5; // flag de timeout
        estado = RETRANSMISSAO;
      }
      break;

    case TRATA_RESPOSTA_SLAVE:
      addrBackup = addr;
      opcodeBackup = opcode;
      dadoBackup = dado;
      flag = recebeMensagem(SERIAL_PORT_BUS, &addr, &opcode, &dado);
      if (!flag) {
        estado = proximo_estado;
      }
      else {
        estado = RETRANSMISSAO;
      }
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

    case AQUISICAO_CONTINUA:
      estado = ENVIA_MSG_SLAVE;
      break;

    case RETRANSMISSAO:
      if (contRetransmissao >= 3) {
        flag = 6;
        estado = REPORTA_ERRO;
        contRetransmissao = 0;
      }
      else {
        addr = addrBackup;
        opcode = opcodeBackup;
        dado = dadoBackup;
        estado = ENVIA_MSG_SLAVE;
        contRetransmissao++;
      }
      break;

    case REPORTA_ERRO:
      estado = AGUARDANDO;
      break;

    default:
      estado = AGUARDANDO;
      break;
  }

  //  enviaMensagem(0x02, 0x01, 0x02F5);
  delay(1000);
}

void atualizaLEDSP(byte addr, int dado) {
  if (addr == SLAVE_1)
    digitalWrite(LED_SPVP1, dado);
  else if (addr == SLAVE_2)
    digitalWrite(LED_SPVP2, dado);
}


void trataMSGPC() {
  if (addr == MASTER_ADDR) {        //Operação de controle ou direcionada a ambos escravos
    switch (opcode) {
      // Se forem opcode padrão (de 0b0000 a 0b0101), está requerindo esta operação de ambos escravos
      case 0b0000 ... 0b0110:
        addr = SLAVE_1;
        ambos = 1;
        estado = ENVIA_MSG_SLAVE;
        proximo_estado = RESPONDE_PC;
        break;

      case 0b1101:
        estado = AQUISICAO_CONTINUA;
        aquisicao_continua_flag = !aquisicao_continua_flag;
        break;

      case 0b1110:
        timeout = dado;
        estado = AGUARDANDO;
        break;

      case 0b1111:
        dt = dado;
        estado = AGUARDANDO;
        break;

      default:
        //            flag = 3;
        //            estado = ATUALIZA_BUFFER_ERRO;
        break;
    }
  }
  else { //operação diretamente ao escravo
    switch (opcode) {
      case 0b0000 ... 0b0110:
        ambos = 0;
        estado = ENVIA_MSG_SLAVE;
        proximo_estado = RESPONDE_PC;
        break;
      default:
        //            flag = 3;
        //            estado = ATUALIZA_BUFFER_ERRO;
        break;
    }
  }
}
