/*  POR CARLOS HENKE DE OLIVEIRA (carloshenke@unb.r), 2022
 *  
 *  A BIBLIOTECA Joystick.h tem os seguintes domínios para os eixos:
 *  EIXO X (XAxis): -127 a 127
 *  EIXO Y (XAxis): -127 a 127 
 *  EIXO Z (ZAxis): -127 a 127
 *  ROTACAO X (XAxisRotation): 0 A 255
 *  ROTAÇÃO Y (YAxisRotation): 0 A 255
 *  LEME: (Rudder) 0 A 255
 *  ACEELERADOR (Throttle): 0 A 255
 */
 
#include "Joystick.h"
#include "HX711.h"
#include <EEPROM.h>
#include "SoftwareSerial.h"

bool modo_debug = false;

const int PINO_CALIBRA = 9;

const int PINO_PROFUNDOR_DT = 7;
const int PINO_PROFUNDOR_CK = 5;

int hat1_x, hat1_x_min, hat1_x_max, hat1_y, hat1_y_min, hat1_y_max;
int hat2_x, hat2_x_min, hat2_x_max, hat2_y, hat2_y_min, hat2_y_max;
bool botao0,botao1,botao2,botao3,botao4,botao5,botao6,botao7,botao8,botao9; 
int aileron, aileron_min, aileron_max;
long profundor, profundor_min, profundor_max;

bool falhou = false;
long falhas = 0;
long porta_vazia = 0;

long millis_ciclos = millis();
float ciclos = 0;
float taxa_de_ciclos;

HX711 PROFUNDOR;
SoftwareSerial ss(16,2); // rx/tx OBS, NO MICRO NEM TODOS OS PINOS ACEITAM RX, SOMENTE OS QUE POSSUEM INTERRUPCAO

void setup() 
{
  Joystick.begin();
  Serial.begin(115200);
  //while (!Serial);
  ss.begin(9600);
  pinMode(PINO_CALIBRA, INPUT_PULLUP);
  delay(5000);
  Serial.println("Receptor...");
  PROFUNDOR.begin(PINO_PROFUNDOR_DT, PINO_PROFUNDOR_CK);
  if (!digitalRead(PINO_CALIBRA)) calibracao();
  le_EEPROM();
}

void calibracao()
  {
  Serial.print("Calibracao...");
  delay(500); 
  le_serial();
  if (PROFUNDOR.is_ready()) profundor = PROFUNDOR.read();

  aileron_min = aileron; aileron_max = aileron;
  hat1_x_min = hat1_x; hat1_x_max = hat1_x;
  hat1_y_min = hat1_y; hat1_y_max = hat1_y;
  hat2_x_min = hat2_x; hat2_x_max = hat2_x;
  hat2_y_min = hat2_y; hat2_y_max = hat2_y;
  long amostras = 0;
  while(!digitalRead(PINO_CALIBRA))
    {
    le_serial();
    if (PROFUNDOR.is_ready()) profundor = PROFUNDOR.read();
    
    if (aileron < aileron_min) aileron_min = aileron; if (aileron > aileron_max) aileron_max = aileron;
    if (profundor < profundor_min) profundor_min = profundor; if (profundor > profundor_max) profundor_max = profundor;
    if (hat1_x < hat1_x_min) hat1_x_min = hat1_x; if (hat1_x > hat1_x_max) hat1_x_max = hat1_x;
    if (hat1_y < hat1_y_min) hat1_y_min = hat1_y; if (hat1_y > hat1_y_max) hat1_y_max = hat1_y;
    if (hat2_x < hat2_x_min) hat2_x_min = hat2_x; if (hat2_x > hat2_x_max) hat2_x_max = hat2_x;
    if (hat2_y < hat2_y_min) hat2_y_min = hat2_y; if (hat2_y > hat2_y_max) hat2_y_max = hat2_y;
    amostras++;
    Serial.print("#"); Serial.println(amostras);
    }
   Serial.println(" Ok");
   delay(500); 
   gravacao_EEPROM();
  }
  
void loop() 
  {
  ciclos++;
  if (ciclos >= 100) 
     {
     float tempo_ciclos = float(millis() - millis_ciclos) / 1000;
     taxa_de_ciclos = ciclos /tempo_ciclos; 
     //Serial.print(ciclos); Serial.print(" em "); Serial.print(tempo_ciclos); Serial.println(" segundos");
     Serial.print(taxa_de_ciclos);Serial.println(" ciclos / seg");
     millis_ciclos = millis();
     ciclos=0; 
     }
  if (PROFUNDOR.is_ready()) profundor = PROFUNDOR.read();
  le_serial();

        
        if (aileron < aileron_min) aileron = aileron_min; if (aileron > aileron_max) aileron = aileron_max;
        if (profundor < profundor_min) profundor = profundor_min; if (profundor > profundor_max) profundor = profundor_max;
        if (hat1_x < hat1_x_min) hat1_x = hat1_x_min; if (hat1_x > hat1_x_max) hat1_x = hat1_x_max;
        if (hat1_y < hat1_y_min) hat1_y = hat1_y_min; if (hat1_y > hat1_y_max) hat1_y = hat1_y_max;
        if (hat2_x < hat2_x_min) hat2_x = hat2_x_min; if (hat2_x > hat2_x_max) hat2_x = hat2_x_max;
        if (hat2_y < hat2_y_min) hat2_y = hat2_y_min; if (hat2_y > hat2_y_max) hat2_y = hat2_y_max;
        

        int EIXO_X = map(aileron,aileron_min, aileron_max, -120, 120);
        int EIXO_Y = map(profundor,profundor_min, profundor_max, -120, 120);

        int EIXO_HAT1_X = map(hat1_x, hat1_x_min, hat1_x_max, 10, 245);
        int EIXO_HAT1_Y = map(hat1_y,hat1_y_min, hat1_y_max, 10, 245);
        int EIXO_HAT2_X = map(hat2_x,hat2_x_min, hat2_x_max, 10, 245);
        int EIXO_HAT2_Y = map(hat2_y,hat2_y_min, hat2_y_max, 10, 245);

        int tmp_HAT1_X = map(hat1_x,hat1_x_min, hat1_x_max, -100, 100);
        int tmp_HAT1_Y = map(hat1_y,hat1_y_min, hat1_y_max, -100, 100);
        int tmp_HAT2_X = map(hat2_x,hat2_x_min, hat2_x_max, -100, 100);
        int tmp_HAT2_Y = map(hat2_y,hat2_y_min, hat2_y_max, -100, 100);
        /*
        Serial.print("HAT1: "); Serial.print(hat1_x); Serial.print(" DE: ");Serial.print(hat1_x_min);Serial.print(" A ");Serial.print(hat1_x_max);Serial.println();
        Serial.print("EIXO_HAT1_X: "); Serial.print(EIXO_HAT1_X);Serial.println();
        Serial.print("EIXO_HAT1_Y: "); Serial.print(EIXO_HAT1_Y);Serial.println();

        Serial.print("EIXO_X: "); Serial.print(EIXO_X);Serial.println();
        Serial.print("EIXO_Y: "); Serial.print(EIXO_Y);Serial.println();
        */
        

        int X,Y,POSICAO_HAT1,POSICAO_HAT2;
        X = 0; Y = 0;
        if (tmp_HAT1_X < -50) X = -1;
        if (tmp_HAT1_X > 50) X = 1;
        if (tmp_HAT1_Y < -50) Y = -1;
        if (tmp_HAT1_Y > 50) Y = 1; 

        if ((X==0) and (Y==0)) POSICAO_HAT1 = -1;
        if ((X==0) and (Y==1)) POSICAO_HAT1 = 0;
        if ((X==1) and (Y==1)) POSICAO_HAT1 = 45;
        if ((X==1) and (Y==0)) POSICAO_HAT1 = 90;
        if ((X==1) and (Y==-1)) POSICAO_HAT1 = 135;
        if ((X==0) and (Y==-1)) POSICAO_HAT1 = 180;
        if ((X==-1) and (Y==-1)) POSICAO_HAT1 = 225;
        if ((X==-1) and (Y==0)) POSICAO_HAT1 = 270;
        if ((X==-1) and (Y==1)) POSICAO_HAT1 = 315;

        X = 0; Y = 0;
        if (tmp_HAT2_X < -50) X = -1;
        if (tmp_HAT2_X > 50) X = 1;
        if (tmp_HAT2_Y < -50) Y = -1;
        if (tmp_HAT2_Y > 50) Y = 1; 

        if ((X==0) and (Y==0)) POSICAO_HAT2 = -1;
        if ((X==0) and (Y==1)) POSICAO_HAT2 = 0;
        if ((X==1) and (Y==1)) POSICAO_HAT2 = 45;
        if ((X==1) and (Y==0)) POSICAO_HAT2 = 90;
        if ((X==1) and (Y==-1)) POSICAO_HAT2 = 135;
        if ((X==0) and (Y==-1)) POSICAO_HAT2 = 180;
        if ((X==-1) and (Y==-1)) POSICAO_HAT2 = 225;
        if ((X==-1) and (Y==0)) POSICAO_HAT2 = 270;
        if ((X==-1) and (Y==1)) POSICAO_HAT2 = 315;
  

        Joystick.setXAxis(EIXO_X);
        Joystick.setYAxis(EIXO_Y);

        Joystick.setXAxisRotation(EIXO_HAT1_X);
        Joystick.setYAxisRotation(EIXO_HAT1_Y);

        Joystick.setRudder(EIXO_HAT2_X);
        Joystick.setThrottle(EIXO_HAT2_Y);
  
        Joystick.setHatSwitch(0, POSICAO_HAT1);
        Joystick.setHatSwitch(1, POSICAO_HAT2);
  
        if (modo_debug)
        {
        Serial.print("Profundor: ");Serial.print(profundor);Serial.print(" = ");Serial.println(EIXO_Y);
        Serial.print("Aileron: ");Serial.print(aileron);Serial.print(" = ");Serial.println(EIXO_X);
        Serial.print("hat1_x: ");Serial.print(hat1_x);Serial.print(" = ");Serial.println(EIXO_HAT1_X);
        Serial.print("hat1_y: ");Serial.print(hat1_y);Serial.print(" = ");Serial.println(EIXO_HAT1_Y);
        Serial.print("hat2_x: ");Serial.print(hat2_x);Serial.print(" = ");Serial.println(EIXO_HAT2_X);
        Serial.print("hat2_y: ");Serial.print(hat2_y);Serial.print(" = ");Serial.println(EIXO_HAT2_Y);
        Serial.print("EIXO_HAT1: ");Serial.print(EIXO_HAT1_X);Serial.print(" ");Serial.print(EIXO_HAT1_Y);Serial.print(" ");Serial.println(POSICAO_HAT1);
        Serial.print("EIXO_HAT2: ");Serial.print(EIXO_HAT2_X);Serial.print(" ");Serial.print(EIXO_HAT2_Y);Serial.print(" ");Serial.println(POSICAO_HAT2);
        Serial.print("Botoes: ");Serial.print(botao0);Serial.print(botao1);Serial.print(botao2);Serial.print(botao3);Serial.print(botao4);Serial.print(botao5);Serial.print(botao6);Serial.print(botao7);Serial.print(botao8);Serial.print(botao9);Serial.println();
        Serial.print(taxa_de_ciclos);Serial.println(" ciclos / seg");
        Serial.println();
        }

        
        Joystick.setButton(0, botao0);
        Joystick.setButton(1, botao1);
        Joystick.setButton(2, botao2);
        Joystick.setButton(3, botao3);
        Joystick.setButton(4, botao4);
        Joystick.setButton(5, botao5);
        Joystick.setButton(6, botao6);
        Joystick.setButton(7, botao7);
        Joystick.setButton(8, botao8);
        Joystick.setButton(9, botao9);
        

  
        //Serial.print(X);Serial.print(",");Serial.print(Y);Serial.print(",");Serial.print(RUDDER);Serial.println();
        //delay(10);
        
      
      

  //Serial.print(X);Serial.print(",");Serial.print(Y);Serial.print(",");Serial.print(RUDDER);Serial.println();
  //delay(500);

  
  }

void gravacao_EEPROM()
  {
  Serial.print("Gravacao...");
  int endereco = 0;
  EEPROM.put(endereco, hat1_x_min); endereco += sizeof(int); EEPROM.put(endereco, hat1_x_max); endereco += sizeof(int);
  EEPROM.put(endereco, hat1_y_min); endereco += sizeof(int); EEPROM.put(endereco, hat1_y_max); endereco += sizeof(int);
  EEPROM.put(endereco, hat2_x_min); endereco += sizeof(int); EEPROM.put(endereco, hat2_x_max); endereco += sizeof(int);
  EEPROM.put(endereco, hat2_y_min); endereco += sizeof(int); EEPROM.put(endereco, hat2_y_max); endereco += sizeof(int);
  EEPROM.put(endereco, aileron_min); endereco += sizeof(int);EEPROM.put(endereco, aileron_max);endereco += sizeof(int);
  EEPROM.put(endereco, profundor_min); endereco += sizeof(long);EEPROM.put(endereco, profundor_max);
  Serial.println(" Ok");
  }

void le_EEPROM()
  {
  Serial.print("Recuperacao");
  int endereco = 0;
  EEPROM.get(endereco, hat1_x_min); endereco += sizeof(int); EEPROM.get(endereco, hat1_x_max); endereco += sizeof(int);
  EEPROM.get(endereco, hat1_y_min); endereco += sizeof(int); EEPROM.get(endereco, hat1_y_max); endereco += sizeof(int);
  EEPROM.get(endereco, hat2_x_min); endereco += sizeof(int); EEPROM.get(endereco, hat2_x_max); endereco += sizeof(int);
  EEPROM.get(endereco, hat2_y_min); endereco += sizeof(int); EEPROM.get(endereco, hat2_y_max); endereco += sizeof(int);
  EEPROM.get(endereco, aileron_min); endereco += sizeof(int);EEPROM.get(endereco, aileron_max);endereco += sizeof(int);
  EEPROM.get(endereco, profundor_min);endereco += sizeof(long); EEPROM.get(endereco, profundor_max);
  Serial.println(" Ok");
  Serial.print("Aileron: ");Serial.print(aileron_min);Serial.print(" a ");Serial.print(aileron_max);Serial.println();
  Serial.print("Profundor: ");Serial.print(profundor_min);Serial.print(" a ");Serial.print(profundor_max);Serial.println();
  Serial.print("Hat1_x: ");Serial.print(hat1_x_min);Serial.print(" a ");Serial.print(hat1_x_max);Serial.println();
  Serial.print("Hat1_y: ");Serial.print(hat1_y_min);Serial.print(" a ");Serial.print(hat1_y_max);Serial.println();
  Serial.print("Hat2_x: ");Serial.print(hat2_x_min);Serial.print(" a ");Serial.print(hat2_x_max);Serial.println();
  Serial.print("Hat2_y: ");Serial.print(hat2_y_min);Serial.print(" a ");Serial.print(hat2_y_max);Serial.println();
  }


void le_serial()
  {
  bool falhou = false;
  long vazios = 0;
  while(ss.read() != 253) vazios++;
  //Serial.print("Vazios = ");Serial.println(vazios);
  while(ss.available() < 21);
  int TMP_aileron = ss.read()*256+ss.read();
  int TMP_hat1_x = ss.read()*256+ss.read();
  int TMP_hat1_y = ss.read()*256+ss.read();
  int TMP_hat2_x = ss.read()*256+ss.read();
  int TMP_hat2_y = ss.read()*256+ss.read();
  bool TMP_botao0 = ss.read();
  bool TMP_botao1 = ss.read();
  bool TMP_botao2 = ss.read();
  bool TMP_botao3 = ss.read();
  bool TMP_botao4 = ss.read();
  bool TMP_botao5 = ss.read();
  bool TMP_botao6 = ss.read();
  bool TMP_botao7 = ss.read();
  bool TMP_botao8 = ss.read();
  bool TMP_botao9 = ss.read();
  if (ss.read() != 254) falhou = true;
  if (!falhou)
        {
        aileron = TMP_aileron;
        hat1_x = -TMP_hat1_x;
        hat1_y = -TMP_hat1_y;
        hat2_x = -TMP_hat2_x;
        hat2_y = -TMP_hat2_y;
        botao0 = !TMP_botao0;
        botao1 = !TMP_botao1;
        botao2 = !TMP_botao2;
        botao3 = !TMP_botao3;
        botao4 = !TMP_botao4;
        botao5 = !TMP_botao5;
        botao6 = !TMP_botao6;
        botao7 = !TMP_botao7;
        botao8 = !TMP_botao8;
        botao9 = !TMP_botao9;
        }
  }
