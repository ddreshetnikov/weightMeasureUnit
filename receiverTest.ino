/*
Победа над nRF24L01: на три шага ближе, приемник
https://habr.com/ru/post/476716/
*/
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

#include<Wire.h> 
#define MCP4725 0x60

byte buffer[3];

RF24 radio(9, 10); // порты D9, D10: CSN CE

const uint32_t pipe = 111156789; // адрес рабочей трубы;
int data[1];
int weight;
  int cnt;
bool led = false;

void setup() {
    Wire.begin();
  Serial.begin(9600);
  Serial.println("ReceiverTester ON");
  pinMode(3, OUTPUT);
  radio.begin();  // инициализация
  delay(2000);
  radio.setDataRate(RF24_1MBPS); // скорость обмена данными RF24_1MBPS или RF24_2MBPS
  radio.setCRCLength(RF24_CRC_8); // размер контрольной суммы 8 bit или 16 bit
  radio.setChannel(0x6f);         // установка канала
  radio.setAutoAck(false);       // автоответ
  radio.openReadingPipe(1, pipe); // открыть трубу на приём
  radio.startListening();        // приём
}

void loop() {
  Serial.println(cnt);
    if (radio.available())
      {
        cnt = 0;
        digitalWrite(3, HIGH);
        radio.read(data, sizeof(data));
      //Serial.println(data[0]);
    }
    else{
      cnt++;
    }
    if (cnt >= 10){
      digitalWrite(3, LOW);
      cnt = 10;
    }
    
      weight = data[0];
      //weight=4095;
      buffer[0] = 0b01000000;
      buffer[1] = weight >> 4;              //записываем наиболее значимые биты
      buffer[2] = weight << 4;              //записываем наименее значимые биты

      Wire.beginTransmission(MCP4725);         //присоединяемся к шине I2C с MCP4725 с адресом 0x61
  
      Wire.write(buffer[0]);            //передаем контрольный байт с помощью протокола I2C 
      Wire.write(buffer[1]);            //передаем наиболее значимые биты с помощью протокола I2C 
      Wire.write(buffer[2]);            // передаем наименее значимые биты с помощью протокола I2C
  
      Wire.endTransmission();           //окончание передачи
      //digitalWrite(5, LOW);
  //delay(5);
}
