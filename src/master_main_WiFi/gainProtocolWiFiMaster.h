#ifndef _GAINPROTOCOLWIFIFMASTER_H
#define _GAINPROTOCOLWIFIMASTER_H

#include <Arduino.h>
#include <WiFi.h>

#define MAX485_RE_NEG 2 // Pino RE negado e DE, 0 = Receber, 1 = Transmitir

/* enviaMensagem: envia uma mensagem completa do nosso protocolo
Entradas: endereço de destino (addr), código da operação (opcode) e o dado (dado)
Saídas: ---
*/
void enviaMensagem(int serial_bus, byte addr, byte opcode, int dado);

/* montaCabecalho: monta o cabeçalho (1 byte) a partir dos nibbles addr e opcode
Entradas: addr e opcode (1 nibble cada)
Saídas: retorna o cabeçalho completo com 1 byte
*/
byte montaCabecalho(byte addr, byte opcode);

/* divideDado: Divide a palavra do dado em dois bytes para transmissão
Entradas: dado e ponteiros para saídas
Saídas: 
- valores dos bytes mais e menos significativos dadoA e dadoB, respectivamente, retornados através de ponteiros recebidos nos parêmetros de entrada
*/
void divideDado(int dado, byte *dadoA, byte *dadoB);

/* geraChecksum: Retorna um byte de checksum a partir de uma dada soma de entrada
Entradas: soma (16 bits) dos bytes que deseja-se transmitir ou que foram recebidos
Saídas: retorna o byte checksum

*/
byte geraChecksum(int soma);

/* recebeMensagem: Trata da mensaem recebida 
Entradas: ---
Saídas: 
- addr, opcode e dado, todos retornados através de ponteiros recebidos nos parêmetros de entrada
- retorno de uma flag de erro:
  - flag = 0 -> Sem erros
  - flag = 1 -> Erro no número de bytes recebidos
  - flag = 2 -> Erro de checksum

*/
int recebeMensagem(int serial_bus, byte *addr, byte *opcode, int *dado);

/* divideCabecalho: Divide o byte do cabeçalho nas informações de endereço e código de operação
Entradas: cabeçalho e ponteiros para saídas
Saídas: 
- addr, opcode, todos retornados exclusivamente através de ponteiros recebidos nos parêmetros de entrada
*/
void divideCabecalho(byte cabecalho, byte *addr, byte *opcode);

/* montaDado: monta o dado (2 bytes) a partir dos bytes mais (A) e menos (B) significativos
Entradas: bytes mais (dadoA) e menos (dadoB) significativos
Saídas: retorna o dado completo com 16 bits
*/
int montaDado(byte dadoA, byte dadoB);

/* Serial2flush: limpa buffer de entrada da porta serial 2
Entradas: ---
Saídas: ---
*/
void Serial2flush(void);

/* Serial2flush: limpa buffer de entrada da porta serial
Entradas: ---
Saídas: ---
*/
void Serialflush(void);


void habilitaTransmitirNoBarramento(void);

void habilitaReceberDoBarramento(void);

#endif
