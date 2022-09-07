/*
Победа над nRF24L01: на три шага ближе, передатчик
https://habr.com/ru/post/476716/
*/
#include <GyverHX711.h>
#include <GyverTimers.h>
#include <GyverPWM.h>
#include<Wire.h> 
#define MCP4725 0x60

#define NUM_READ 5
#include <SPI.h>
#include <RF24.h>
RF24 radio(9, 10); // порты D9, D10: CSN CE
const uint32_t pipe = 111156789; // адрес рабочей трубы;

GyverHX711 sensor(3, 2, HX_GAIN128_A);
// HX_GAIN128_A - канал А усиление 128
// HX_GAIN32_B - канал B усиление 32
// HX_GAIN64_A - канал А усиление 64

int data = 0;
long int shift = 28850;
long int weight = 0;

bool led = HIGH;

float koeff = 0.309;
long int findMedianN(long int newVal) {
  static long int buffer[NUM_READ];  
  static byte count = 0;    // счётчик
  buffer[count] = newVal;
  if (++count >= NUM_READ) count = 0;  
  
  long int buf[NUM_READ];    
  for (byte i = 0; i < NUM_READ; i++) buf[i] = buffer[i];  
  for (int i = 0; i <= (long int) ((NUM_READ / 2) + 1); i++) {
    for (int m = 0; m < NUM_READ - i - 1; m++) {
      if (buf[m] > buf[m + 1]) {
        long int buff = buf[m];
        buf[m] = buf[m + 1];
        buf[m + 1] = buff;
      }
    }
  }
  long int ans = 0;
  long int sum = 0; 
  for (byte i = 1; i < NUM_READ-1; i++){
    sum = sum + buf[i];
  }
  ans = (long int) sum/(NUM_READ-2);
  return ans;
}
ISR(TIMER1_A){
  digitalWrite(5, led);
  led = !led;
}
void setup() {
  pinMode(5, OUTPUT);
  Timer1.setFrequency(10);
  Timer1.enableISR();

  //digitalWrite(5, HIGH);
  Serial.begin(115200);

  delay(1000);
  sensor.sleepMode(true);    // выключить датчик
  sensor.sleepMode(false);  // включить датчик
    sensor.tare();    // калибровка нуля
    delay(1000);
    while(sensor.read() <= 100){
    }
    shift = sensor.read();
  
  Serial.println("TransmitterTester ON");

  radio.begin();                // инициализация
  delay(2000);
  radio.setDataRate(RF24_1MBPS); // скорость обмена данными RF24_1MBPS или RF24_2MBPS
  radio.setCRCLength(RF24_CRC_8); // размер контрольной суммы 8 bit или 16 bit
  radio.setPALevel(RF24_PA_HIGH); // уровень питания усилителя RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH and RF24_PA_MAX
  radio.setChannel(0x6f);         // установка канала
  radio.setAutoAck(false);       // автоответ
  radio.powerUp();               // включение или пониженное потребление powerDown - powerUp
  radio.stopListening();  //радиоэфир не слушаем, только передача
  radio.openWritingPipe(pipe);   // открыть трубу на отправку
  Timer1.disableISR();
  digitalWrite(5, HIGH);
}

void loop() {
  weight = (long int)findMedianN(sensor.read()-shift)*koeff;
  data = weight>>9;
  if (data < 0) data = 0;
  if (data >= 4095) data = 4095;
  radio.write(&data, sizeof(data));
  //Serial.println("data= " + String(data));
  delay(5);
  
}
