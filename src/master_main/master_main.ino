// Código do MESTRE
/*Autores:
  Leonardo Gonçalves
  Lucas Mantuan Ayres
  Pablo France Salarolli
*/
#include "gainProtocol.h"

void setup() {
  // Note the format for setting a serial port is as follows: Serial2.begin(baud-rate, protocol, RX pin, TX pin);
  pinMode(2, OUTPUT);
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, 16, 17);
  digitalWrite(2, 1);
}

void loop() { //Choose Serial1 or Serial2 as required
  enviaMensagem(0x02, 0x01, 0x02F5);
  delay(1000);
}
