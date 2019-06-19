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
long dt = 4000, timeout = 7000;
unsigned long tAnt = 0, tAgora = 0, tAntTimeOut = 0, tAgoraTimeOut = 0;

void setup() {
  // Note the format for setting a serial port is as follows: Serial2.begin(baud-rate, protocol, RX pin, TX pin);
  pinMode(LED_SPVP1, OUTPUT);
  pinMode(LED_SPVP2, OUTPUT);
  pinMode(MAX485_RE_NEG, OUTPUT);
  habilitaReceberDoBarramento();
  digitalWrite(LED_SPVP1, 1);
  delay(1000);
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
  digitalWrite(LED_SPVP2, 1);
  delay(1000);
  digitalWrite(LED_SPVP1, 0);
  digitalWrite(LED_SPVP2, 0);
}

void loop() {


  switch (estado) {
    case AGUARDANDO:
      //            Serial.println("Aguardando");
      tAgora = millis();
      if (Serial.available()) {
        estado = RECEBE_MSG_PC;
        //        Serial.println("RECEBI PC");
      }
      else if ((tAgora - tAnt) >= dt) {
        //        Serial.println("Timeout aquisicao");
        tAnt = tAgora;
        opcode = 0b0110; //verifica se SP = VP ATUALIZAR OPCODE!!!
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
      //      Serial.println("Estado: RESPONDE_PC");
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
      //      Serial.println("Estado: ENVIA_MSG_SLAVE");
      //      Serial.println("Estado: AGUARDA_RESPOSTA_SLAVE");
      //      Serial.print("addr ");
      //      Serial.println(addr, HEX);
      //      Serial.print("opcode ");
      //      Serial.println(opcode, HEX);
      //      Serial.print("DADO ");
      //      Serial.println(dado, HEX);
      //      Serial.print("ambos ");
      //      Serial.println(ambos, HEX);
      enviaMensagem(SERIAL_PORT_BUS, addr, opcode, dado);
      estado = AGUARDA_RESPOSTA_SLAVE;
      tAgoraTimeOut = millis();
      tAntTimeOut = tAgoraTimeOut;
      break;

    case AGUARDA_RESPOSTA_SLAVE:

      if (Serial2.available()) {
        estado = TRATA_RESPOSTA_SLAVE;
      }
      else {
        tAgoraTimeOut = millis();
        //        Serial.println(tAgoraTimeOut - tAntTimeOut, DEC);
        if ((tAgoraTimeOut - tAntTimeOut) >= timeout) {
          tAntTimeOut = tAgoraTimeOut;
          flag = 5; // flag de timeout
          estado = RETRANSMISSAO;
          //          Serial.println("Deu pau");
        }
      }
      break;

    case TRATA_RESPOSTA_SLAVE:
      //      Serial.println("Estado: TRATA_RESPOSTA_SLAVE");
      addrBackup = addr;
      opcodeBackup = opcode;
      dadoBackup = dado;
      flag = recebeMensagem(SERIAL_PORT_BUS, &addr, &opcode, &dado);
      if (!flag) {
        estado = proximo_estado;
        if (addr == MASTER_ADDR) {
          addr = addrBackup;
        }
      }
      else {
        estado = RETRANSMISSAO;
      }
      break;

    case ATUALIZA_LEDS:
      //      Serial.println("Estado: ATUALIZA_LEDS");
      //      Serial.println("addr");
      //      Serial.print(addr, HEX);
      //      Serial.println("DADO");
      //      Serial.print(dado, HEX);
      //      Serial.println("ambos");
      //      Serial.print(ambos, HEX);
      atualizaLEDSP(addr, dado); // Atualiza o Led do escravo em addr com o valor de dado
      if (ambos) {
        //        Serial.println("ambos: ambos");
        ambos = 0;
        addr = SLAVE_2;
        estado = ENVIA_MSG_SLAVE;
      }
      else
        estado = AGUARDANDO;
      break;

    case AQUISICAO_CONTINUA:
      //      Serial.println("Estado: AQUISICAO_CONTINUA");
      estado = ENVIA_MSG_SLAVE;
      break;

    case RETRANSMISSAO:
      //      Serial.println("Estado: RETRANSMISSAO");
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
      //      Serial.println("Estado: REPORTA_ERRO");
      estado = AGUARDANDO;
      break;

    default:
      //      Serial.println("Estado: DEFAULT");
      estado = AGUARDANDO;
      break;
  }

  //  enviaMensagem(0x02, 0x01, 0x02F5);
  delay(5);
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
