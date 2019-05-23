// Código do MESTRE
/*Autores: 
Leonardo Gonçalves
Lucas Mantuan Ayres
Pablo France Salarolli
*/

void setup() {
  // Note the format for setting a serial port is as follows: Serial2.begin(baud-rate, protocol, RX pin, TX pin);
  pinMode(2,OUTPUT);
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, 16, 17);
  digitalWrite(2,1);
}

void loop() { //Choose Serial1 or Serial2 as required
  while (Serial.available()) {
    Serial2.print(char(Serial.read()));
  }
  while (Serial2.available()) {
    Serial.print(char(Serial2.read()));
  }
}

//int soma = 0, wrappsoma = 0;
//byte dado[3] = {0xFF, 0x4F, 0x2E};
//byte checksum = 0;
//  //Checksum transmissor
//    soma = dado[0]+dado[1]+dado[2];
//    wrappsoma = soma >> 8;
//    wrappsoma = wrappsoma+(soma & 0xFF);
//    checksum = ~wrappsoma;
//
//  //Checksum receptor
//    soma = dado[0]+dado[1]+dado[2]+checksum;
//    wrappsoma = soma >> 8;
//    wrappsoma = wrappsoma+(soma & 0xFF);
//    checksum = ~wrappsoma;
