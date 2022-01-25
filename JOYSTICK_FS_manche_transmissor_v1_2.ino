
#include "SoftwareSerial.h"
//LADO DIREITO

const int PINO_BOTAO0 = 12;
const int PINO_BOTAO1 = 10;
const int PINO_BOTAO2 = 11;
const int PINO_BOTAO3 = 9;
const int PINO_BOTAO4 = 17;
// LADO ESQUERDO

const int PINO_BOTAO5 = 7;
const int PINO_BOTAO6 = 5;
const int PINO_BOTAO7 = 6;
const int PINO_BOTAO8 = 4;
const int PINO_BOTAO9 = 8;

const int PINO_HAT1_X = A2;
const int PINO_HAT1_Y = A1;

const int PINO_HAT2_X = A6;
const int PINO_HAT2_Y = A7;

const int PINO_AILERON = A0;

SoftwareSerial ss(2,3); // rx/tx
long dif_millis, millis_anterior = millis();

void setup() 
{
  Serial.begin(115200);
  ss.begin(9600);
  delay(500);
  Serial.println("Transmissor...");
  delay(500); 
  pinMode(PINO_BOTAO0, INPUT_PULLUP);
  pinMode(PINO_BOTAO1, INPUT_PULLUP);
  pinMode(PINO_BOTAO2, INPUT_PULLUP);
  pinMode(PINO_BOTAO3, INPUT_PULLUP);
  pinMode(PINO_BOTAO4, INPUT_PULLUP);
  pinMode(PINO_BOTAO5, INPUT_PULLUP);
  pinMode(PINO_BOTAO6, INPUT_PULLUP);
  pinMode(PINO_BOTAO7, INPUT_PULLUP);
  pinMode(PINO_BOTAO8, INPUT_PULLUP);
  pinMode(PINO_BOTAO9, INPUT_PULLUP);
}

  
void loop() 
  {
  int aileron = analogRead(PINO_AILERON);
  int hat1_x = analogRead(PINO_HAT1_X);
  int hat1_y = analogRead(PINO_HAT1_Y);
  int hat2_x = analogRead(PINO_HAT2_X);
  int hat2_y = analogRead(PINO_HAT2_Y);
  bool botao0 = digitalRead(PINO_BOTAO0);
  bool botao1 = digitalRead(PINO_BOTAO1);
  bool botao2 = digitalRead(PINO_BOTAO2);
  bool botao3 = digitalRead(PINO_BOTAO3);
  bool botao4 = digitalRead(PINO_BOTAO4);
  bool botao5 = digitalRead(PINO_BOTAO5);
  bool botao6 = digitalRead(PINO_BOTAO6);
  bool botao7 = digitalRead(PINO_BOTAO7);
  bool botao8 = digitalRead(PINO_BOTAO8);
  bool botao9 = digitalRead(PINO_BOTAO9);

/*
  Serial.print("Aileron: ");Serial.println(aileron);
  Serial.print("hat1_x: ");Serial.println(hat1_x);
  Serial.print("hat1_y: ");Serial.println(hat1_y);
  Serial.print("hat2_x: ");Serial.println(hat2_x);
  Serial.print("hat2_y: ");Serial.println(hat2_y);

  Serial.print("B0: ");Serial.println(botao0);
  Serial.print("B1: ");Serial.println(botao1);
  Serial.print("B2: ");Serial.println(botao2);
  Serial.print("B3: ");Serial.println(botao3);
  Serial.print("B4: ");Serial.println(botao4);
  Serial.print("B5: ");Serial.println(botao5);
  Serial.print("B6: ");Serial.println(botao6);
  Serial.print("B7: ");Serial.println(botao7);
  Serial.print("B8: ");Serial.println(botao8);
  Serial.print("B9: ");Serial.println(botao9);
*/
 ss.write(253);
 ss.write(highByte(aileron));ss.write(lowByte(aileron));
 ss.write(highByte(hat1_x));ss.write(lowByte(hat1_x));
 ss.write(highByte(hat1_y));ss.write(lowByte(hat1_y));
 ss.write(highByte(hat2_x));ss.write(lowByte(hat2_x));
 ss.write(highByte(hat2_y));ss.write(lowByte(hat2_y));
 ss.write(botao0);
 ss.write(botao1);
 ss.write(botao2);
 ss.write(botao3);
 ss.write(botao4);
 ss.write(botao5);
 ss.write(botao6);
 ss.write(botao7);
 ss.write(botao8);
 ss.write(botao9);
 ss.write(254);

 dif_millis = millis()- millis_anterior;
 millis_anterior = millis();
 //Serial.print("TEMPO: ");Serial.println(dif_millis);
  //Serial.print(X);Serial.print(",");Serial.print(Y);Serial.print(",");Serial.print(RUDDER);Serial.println();
  //delay(10);

  
  }
