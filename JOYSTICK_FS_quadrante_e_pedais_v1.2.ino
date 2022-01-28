 301 lines (255 sloc) 12.6 KB
/*  POR CARLOS HENKE DE OLIVEIRA (carloshenke@unb.br), 2022
 *  SEE https://youtube.com/playlist?list=PLHaFO8kHQhLsbDEYpe5zKnKIaRhMqIjSN
 * 
 * Use arduino Micro or Leonardo. The library "Joystick.h" do not run in Uno, Mino Pro or Mega.
 * 
 * This software control quadrant and ruder (also breaks) of pilot station.
 * 
 *  
  *//* A BIBLIOTECA Joystick.h tem os seguintes domínios para os eixos:
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
#include <Encoder.h>

// HX711 circuit wiring

bool modo_debug = true;

const int BOTAO_TREM_POUSO = 5;
const int BOTAO_CARBHEAT = 4;
const int BOTAO_AUMENTA = 6;
const int BOTAO_MIMINUI = 7;

// os ultimos botoes do jaoystick sao usados para associar o eixo do flap aos botões de recolhimento/incremento

const int BOTAO_FLAP_UP = 30;
const int BOTAO_FLAP_DOWN = 31;

const int BOTAO_AUMENTA_COMP_ELEVADOR = 1;
const int BOTAO_DIMINUI_COMP_ELEVADOR = 0;
const int BOTAO_AUMENTA_COMP_LEME = 2;
const int BOTAO_DIMINUI_COMP_LEME = 3;
  
const int PINO_CALIBRA = 21;

const int PINO_ENCODER_COMP_ELEVADOR_A = 0;
const int PINO_ENCODER_COMP_ELEVADOR_B = 1;
const int PINO_ENCODER_COMP_LEME_A = 2;
const int PINO_ENCODER_COMP_LEME_B = 3;

const int PINO_CARBHEAT = 7;
const int PINO_TREM_POUSO = 5; // este nao deve ser pullup e precisa de um resistor ate o gnd.
const int PINO_DIMINUI = 19;
const int PINO_AUMENTA = 20;

const int PINO_FREIO_ESQUERDA_DT = 16;
const int PINO_FREIO_ESQUERDA_CK = 10;

const int PINO_FREIO_DIREITA_DT = 15;
const int PINO_FREIO_DIREITA_CK = 14;

const int PINO_EIXO_LEME = A0;
const int PINO_EIXO_POTENCIA = A9;
const int PINO_EIXO_PASSO = A8;
const int PINO_EIXO_MISTURA = A7;
const int PINO_EIXO_FLAP = A6;

int leme, leme_min, leme_max;
int potencia, potencia_min, potencia_max;
int passo, passo_min, passo_max;
int mistura, mistura_min, mistura_max;
int flap, flap_min, flap_max;

int COMP_ELEVADOR_MAPA = 0;

// VALORES OBTIDOS EXPERIMENTALMENTE RELACIONANDO VALOR DO FLAP ANALOGICO PARA CALCULAR A POSICAO EM "DENTES", DE 0 (SEM FLAP) A 3 (FULL)

int limite_flap_0_1 = -67;
int limite_flap_1_2 = 15;
int limite_flap_2_3 = 79;
int dentes_de_flap_entrada_anterior = 0, dentes_de_flap_entrada = 0, dentes_de_flap_saida = 0;
int dentes_de_comp_entrada_anterior = 0, dentes_de_comp_entrada = 0, dentes_de_comp_saida = 0;
long freio_esquerda, freio_esquerda_min, freio_esquerda_max;
long freio_direita, freio_direita_min, freio_direita_max; 

long posicao_comp_elevador, posicao_comp_elevador_antiga = -999;
long posicao_comp_leme, posicao_comp_leme_antiga = -999;

int chave_trem_pouso_anterior = LOW;
int chave_carbheat_anterior = LOW;

int comando_trem_de_pouso;
int comando_carbheat;

int comando_flap_up, comando_flap_down;
int comando_comp_up, comando_comp_down;

long millis_trem_pouso;
long millis_carbheat;
long millis_flap;
long millis_comp;
long limite_millis_nos_botoes = 400;

long millis_ciclos = millis();
float ciclos = 0;
float taxa_de_ciclos;
 
HX711 FREIO_ESQUERDA;
HX711 FREIO_DIREITA;

Encoder COMPENSADOR_ELEVADOR(PINO_ENCODER_COMP_ELEVADOR_A, PINO_ENCODER_COMP_ELEVADOR_B);
Encoder COMPENSADOR_LEME(PINO_ENCODER_COMP_LEME_A, PINO_ENCODER_COMP_LEME_B);

void setup() 
{
  delay(2000);
  Serial.begin(9600);
  Serial.println("Hello...");
  Joystick.begin();
  FREIO_ESQUERDA.begin(PINO_FREIO_ESQUERDA_DT, PINO_FREIO_ESQUERDA_CK);
  FREIO_DIREITA.begin(PINO_FREIO_DIREITA_DT, PINO_FREIO_DIREITA_CK);
  while (!FREIO_ESQUERDA.is_ready());
  while (!FREIO_DIREITA.is_ready());
  pinMode(PINO_CALIBRA, INPUT_PULLUP);
  pinMode(PINO_CARBHEAT, INPUT_PULLUP);
  pinMode(PINO_TREM_POUSO, INPUT); // o pino 5 (trem de pouso) nao pode ser pullup por causado led. Ele eh invertido
  pinMode(PINO_DIMINUI, INPUT_PULLUP);
  pinMode(PINO_AUMENTA, INPUT_PULLUP);
  if (!digitalRead(PINO_CALIBRA)) calibracao();
  le_EEPROM();
}

void calibracao()
  {
  Serial.println("Calibracao...");
  leme = analogRead(PINO_EIXO_LEME);
  potencia = analogRead(PINO_EIXO_POTENCIA);
  passo = analogRead(PINO_EIXO_PASSO);
  mistura = analogRead(PINO_EIXO_MISTURA);
  flap = analogRead(PINO_EIXO_FLAP);
  freio_esquerda = FREIO_ESQUERDA.read();
  freio_direita = FREIO_DIREITA.read();

  leme_min = leme; leme_max = leme;
  potencia_min = potencia; potencia_max = potencia;
  passo_min = passo; passo_max = passo;
  mistura_min = mistura; mistura_max = mistura;
  flap_min = flap; flap_max = flap;
  freio_esquerda_min = freio_esquerda; freio_esquerda_max = freio_esquerda;
  freio_direita_min = freio_direita; freio_direita_max = freio_direita;
  long amostras = 0;
  while(!digitalRead(PINO_CALIBRA))
    {
    leme = analogRead(PINO_EIXO_LEME);
    potencia = analogRead(PINO_EIXO_POTENCIA);
    passo = analogRead(PINO_EIXO_PASSO);
    mistura = analogRead(PINO_EIXO_MISTURA);
    flap = analogRead(PINO_EIXO_FLAP);
    if (FREIO_ESQUERDA.is_ready()) freio_esquerda = FREIO_ESQUERDA.read();
    if (FREIO_DIREITA.is_ready()) freio_direita = FREIO_DIREITA.read();
    if (leme < leme_min) leme_min = leme; if (leme > leme_max) leme_max = leme;
    if (potencia < potencia_min) potencia_min = potencia; if (potencia > potencia_max) potencia_max = potencia;
    if (passo < passo_min) passo_min = passo; if (passo > passo_max) passo_max = passo;
    if (mistura < mistura_min) mistura_min = mistura; if (mistura > mistura_max) mistura_max = mistura;
    if (flap < flap_min) flap_min = flap; if (flap > flap_max) flap_max = flap;
    if (freio_esquerda < freio_esquerda_min) freio_esquerda_min = freio_esquerda; 
    //if (freio_esquerda_min < 0) {Serial.print("Novo valor negativo:");Serial.print(freio_esquerda_min);delay(10000);}
    if (freio_esquerda > freio_esquerda_max) freio_esquerda_max = freio_esquerda;
    if (freio_direita < freio_direita_min) freio_direita_min = freio_direita; if (freio_direita > freio_direita_max) freio_direita_max = freio_direita;
    amostras++;
    if(modo_debug)
    {
    Serial.print("leme: ");Serial.println(leme);
    Serial.print("potencia: ");Serial.println(potencia);
    Serial.print("passo: ");Serial.println(passo);
    Serial.print("mistura: ");Serial.println(mistura);
    Serial.print("flap: ");Serial.println(flap);
    Serial.print("Freio Esq: ");Serial.println(freio_esquerda); 
    Serial.print("Freio Dir: ");Serial.println(freio_direita);
    Serial.println();
    }
    }
   Serial.print(amostras);Serial.println(" amnostras");
   delay(5000); 
   gravacao_EEPROM();
   Serial.println(" Ok");
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
  if (FREIO_ESQUERDA.is_ready()) freio_esquerda = FREIO_ESQUERDA.read();
  if (FREIO_DIREITA.is_ready()) freio_direita = FREIO_DIREITA.read();
  leme = analogRead(PINO_EIXO_LEME);
  potencia = analogRead(PINO_EIXO_POTENCIA);
  passo = analogRead(PINO_EIXO_PASSO);
  mistura = analogRead(PINO_EIXO_MISTURA);
  flap = analogRead(PINO_EIXO_FLAP);
  
  if (freio_esquerda < freio_esquerda_min) freio_esquerda = freio_esquerda_min; if (freio_esquerda > freio_esquerda_max) freio_esquerda = freio_esquerda_max;
  if (freio_direita < freio_direita_min) freio_direita = freio_direita_min; if (freio_direita > freio_direita_max) freio_direita = freio_direita_max;
  if (leme < leme_min) leme = leme_min; if (leme > leme_max) leme = leme_max;
  if (potencia < potencia_min) potencia = potencia_min; if (potencia > potencia_max) potencia = potencia_max;
  if (passo < passo_min) passo = passo_min; if (passo > passo_max) passo = passo_max;
  if (mistura < mistura_min) mistura = mistura_min; if (mistura > mistura_max) mistura = mistura_max;
  if (flap < flap_min) flap = flap_min; if (flap > flap_max) flap = flap_max; 

  int LEME_MAPA = map(leme,leme_min, leme_max, 10, 245);
  int FREIO_ESQUERDA_MAPA = map(freio_esquerda,freio_esquerda_min, freio_esquerda_max, 10, 245);
  int FREIO_DIREITA_MAPA = map(freio_direita,freio_direita_min, freio_direita_max, 10, 245);
  int POTENCIA_MAPA = map(potencia,potencia_min, potencia_max, 10, 245);
  int PASSO_MAPA = map(passo,passo_min, passo_max, -120, 120);
  int MISTURA_MAPA = map(mistura,mistura_min, mistura_max, -120, 120);
  int FLAP_MAPA = map(flap,flap_min, flap_max, -120, 120); // USO DO FLAP COMO EIXO
  

/// FLAPS

  if (FLAP_MAPA < limite_flap_0_1) dentes_de_flap_entrada = 0;
  if (FLAP_MAPA >= limite_flap_0_1 and FLAP_MAPA < limite_flap_1_2) dentes_de_flap_entrada = 1;
  if (FLAP_MAPA >= limite_flap_1_2 and FLAP_MAPA < limite_flap_2_3) dentes_de_flap_entrada = 2;
  if (FLAP_MAPA >= limite_flap_2_3) dentes_de_flap_entrada = 3;

//// ##### ADMINISTRANDO A OS BOTOES DE FLAPS 

    /// DESARMANDO OS BOTOES ARMADOS, APOS PASSADO O TEMPO LIMITE
    if ((millis() - millis_flap) > limite_millis_nos_botoes)
      {
      if (comando_flap_up) millis_flap = millis(); comando_flap_up = LOW;
      if (comando_flap_down ) millis_flap = millis(); comando_flap_down = LOW;
      }
    /// ARMANDO OS BOTOES, CONDICINALMENTE
    if (dentes_de_flap_saida < 0) dentes_de_flap_saida = 0;
    if (dentes_de_flap_saida > 3) dentes_de_flap_saida = 3;
    if ((millis() - millis_flap) > limite_millis_nos_botoes)
    {
    if (dentes_de_flap_entrada > dentes_de_flap_saida) 
      {
      comando_flap_up = LOW; comando_flap_down = HIGH; dentes_de_flap_saida = dentes_de_flap_saida+1; millis_flap = millis();
      //Serial.println("APLICA");delay(1000);
      }
    if (dentes_de_flap_entrada < dentes_de_flap_saida) 
      {
      comando_flap_up = HIGH; comando_flap_down = LOW; dentes_de_flap_saida = dentes_de_flap_saida-1; millis_flap = millis();
      //Serial.println("RETIRA");delay(1000);
      }
    dentes_de_flap_entrada_anterior = dentes_de_flap_entrada;
    //Serial.print("FLAP anterior: ");Serial.print(dentes_de_flap_entrada_anterior);Serial.print(" FLAP entrada: ");Serial.print(dentes_de_flap_entrada);Serial.print(" FLAP saida: ");Serial.print(dentes_de_flap_saida);Serial.println();
    }
    
/// TREM DE POUSO
    
  if ((chave_trem_pouso_anterior == LOW) and (digitalRead(PINO_TREM_POUSO) == HIGH)) // MUDOU PARA CIMA
    {
    chave_trem_pouso_anterior = HIGH;
    millis_trem_pouso = millis();
    comando_trem_de_pouso = HIGH;
    }

  if ((chave_trem_pouso_anterior == HIGH) and (digitalRead(PINO_TREM_POUSO) == LOW)) // MUDOU PARA BAIXO
    {
    chave_trem_pouso_anterior = LOW;
    millis_trem_pouso = millis();
    comando_trem_de_pouso = HIGH;
    }
  if ((millis() - millis_trem_pouso) > limite_millis_nos_botoes) comando_trem_de_pouso = LOW;

    //// ##### ADMINISTRANDO A CHAVE GANGORRA DO TREM DE POUSO 
  if ((chave_trem_pouso_anterior == LOW) and (digitalRead(PINO_TREM_POUSO) == HIGH)) // MUDOU PARA CIMA
    {
    chave_trem_pouso_anterior = HIGH;
    millis_trem_pouso = millis();
    comando_trem_de_pouso = HIGH;
    }

  if ((chave_trem_pouso_anterior == HIGH) and (digitalRead(PINO_TREM_POUSO) == LOW)) // MUDOU PARA BAIXO
    {
    chave_trem_pouso_anterior = LOW;
    millis_trem_pouso = millis();
    comando_trem_de_pouso = HIGH;
    }
  if ((millis() - millis_trem_pouso) > limite_millis_nos_botoes) comando_trem_de_pouso = LOW;

/// COMPENSADOR DO PROFUNDOR (ELEVADOR)

  posicao_comp_elevador = COMPENSADOR_ELEVADOR.read();
  if (posicao_comp_elevador_antiga == -999) posicao_comp_elevador_antiga = posicao_comp_elevador;
  if (posicao_comp_elevador > posicao_comp_elevador_antiga) {comando_comp_up = HIGH; millis_comp = millis();}
  if (posicao_comp_elevador < posicao_comp_elevador_antiga) {comando_comp_down = HIGH; millis_comp = millis();} 
  if (comando_comp_down and comando_comp_up) {comando_comp_up = LOW; comando_comp_down = LOW;}

  if ( (millis() - millis_comp) > limite_millis_nos_botoes)
    if (posicao_comp_elevador == posicao_comp_elevador_antiga)
      {
        comando_comp_up = LOW;
        comando_comp_down = LOW;
      }
   
   // USO DO COMPENSADOR COMO EIXO
   COMP_ELEVADOR_MAPA = COMP_ELEVADOR_MAPA + (posicao_comp_elevador - posicao_comp_elevador_antiga);
   //if (posicao_comp_elevador > posicao_comp_elevador_antiga) COMP_ELEVADOR_MAPA = COMP_ELEVADOR_MAPA + 1;
   //if (posicao_comp_elevador < posicao_comp_elevador_antiga) COMP_ELEVADOR_MAPA = COMP_ELEVADOR_MAPA - 1;
   if (COMP_ELEVADOR_MAPA > 120) COMP_ELEVADOR_MAPA = 120;
   if (COMP_ELEVADOR_MAPA < -120) COMP_ELEVADOR_MAPA = -120;
   //Serial.println(posicao_comp_elevador);Serial.println(posicao_comp_elevador);Serial.println(COMP_ELEVADOR_MAPA);
   posicao_comp_elevador_antiga = posicao_comp_elevador;
 
/// COMPENSADOR DO LEME

  posicao_comp_leme = COMPENSADOR_LEME.read();
  if (posicao_comp_leme > posicao_comp_leme_antiga) Joystick.setButton(BOTAO_AUMENTA_COMP_LEME, HIGH); else Joystick.setButton(BOTAO_AUMENTA_COMP_LEME, LOW);
  if (posicao_comp_leme < posicao_comp_leme_antiga) Joystick.setButton(BOTAO_DIMINUI_COMP_LEME, HIGH); else Joystick.setButton(BOTAO_DIMINUI_COMP_LEME, LOW);
  posicao_comp_leme_antiga = posicao_comp_leme;


  /// AQUECIMENTO DO CARBURADOR
  //// ##### ADMINISTRANDO A CHAVE GANGORRA DO CABHEAT 
  if ((chave_carbheat_anterior == LOW) and (digitalRead(PINO_CARBHEAT) == HIGH)) // MUDOU PARA CIMA
    {
    chave_carbheat_anterior = HIGH;
    millis_carbheat = millis();
    comando_carbheat = HIGH;
    }

  if ((chave_carbheat_anterior == HIGH) and (digitalRead(PINO_CARBHEAT) == LOW)) // MUDOU PARA BAIXO
    {
    chave_carbheat_anterior = LOW;
    millis_carbheat = millis();
    comando_carbheat = HIGH;
    }
  if ((millis() - millis_carbheat) > limite_millis_nos_botoes) comando_carbheat = LOW;

/// ########### ENVIANDO DADOS PARA O JOYSTICK ##############

  Joystick.setRudder(LEME_MAPA);
  Joystick.setThrottle(POTENCIA_MAPA);
  Joystick.setZAxis(PASSO_MAPA);
  Joystick.setXAxis(MISTURA_MAPA);
  //Joystick.setYAxis(FLAP_MAPA); // RETIRADO O EIXO DO FLAP PARA COLOCAR O COMPENSADOR.
  Joystick.setYAxis(COMP_ELEVADOR_MAPA);
  Joystick.setXAxisRotation(FREIO_ESQUERDA_MAPA);
  Joystick.setYAxisRotation(FREIO_DIREITA_MAPA);

  Joystick.setButton(BOTAO_CARBHEAT, comando_carbheat);
  Joystick.setButton(BOTAO_TREM_POUSO, comando_trem_de_pouso);
  Joystick.setButton(BOTAO_AUMENTA, !digitalRead(PINO_AUMENTA));
  Joystick.setButton(BOTAO_MIMINUI, !digitalRead(PINO_DIMINUI));

  Joystick.setButton(BOTAO_FLAP_UP, comando_flap_up);
  Joystick.setButton(BOTAO_FLAP_DOWN, comando_flap_down);

  //Joystick.setButton(BOTAO_AUMENTA_COMP_ELEVADOR, comando_comp_up); // retirei para poder não acionar os botões urante a calibração no windows
  //Joystick.setButton(BOTAO_DIMINUI_COMP_ELEVADOR, comando_comp_down);  // retirei para poder não acionar os botões urante a calibração no windows

  if(modo_debug)
  {
  Serial.print("leme: ");Serial.print(leme);Serial.print(" = ");Serial.println(LEME_MAPA);
  Serial.print("potencia: ");Serial.print(potencia);Serial.print(" = ");Serial.println(POTENCIA_MAPA);
  Serial.print("passo: ");Serial.print(passo);Serial.print(" = ");Serial.println(PASSO_MAPA);
  Serial.print("mistura: ");Serial.print(mistura);Serial.print(" = ");Serial.println(MISTURA_MAPA);
  Serial.print("flap: ");Serial.print(flap);Serial.print(" = ");Serial.println(FLAP_MAPA);
  Serial.print("Freio Esq: ");Serial.print(freio_esquerda);Serial.print(" = ");Serial.println(FREIO_ESQUERDA_MAPA); 
  Serial.print("Freio Dir: ");Serial.print(freio_direita);Serial.print(" = ");Serial.println(FREIO_DIREITA_MAPA);
  Serial.print("Comp. Elevador: ");Serial.println(COMP_ELEVADOR_MAPA);
  Serial.print(taxa_de_ciclos);Serial.println(" ciclos / seg");
  Serial.println();
  }

  //Serial.print(X);Serial.print(",");Serial.print(Y);Serial.print(",");Serial.print(RUDDER);Serial.println();
  //delay(1000);
  }


void gravacao_EEPROM()
  {
  Serial.print("Gravacao...");
  
  int endereco = 0;
  EEPROM.put(endereco, leme_min); endereco += sizeof(long); EEPROM.put(endereco, leme_max); endereco += sizeof(long);
  EEPROM.put(endereco, potencia_min); endereco += sizeof(long); EEPROM.put(endereco, potencia_max); endereco += sizeof(long);
  EEPROM.put(endereco, passo_min); endereco += sizeof(long); EEPROM.put(endereco, passo_max); endereco += sizeof(long);
  EEPROM.put(endereco, mistura_min); endereco += sizeof(long); EEPROM.put(endereco, mistura_max);endereco += sizeof(long);
  EEPROM.put(endereco, flap_min); endereco += sizeof(long); EEPROM.put(endereco, flap_max);endereco += sizeof(long);
  EEPROM.put(endereco, freio_esquerda_min); endereco += sizeof(long); EEPROM.put(endereco, freio_esquerda_max);endereco += sizeof(long);
  EEPROM.put(endereco, freio_direita_min); endereco += sizeof(long); EEPROM.put(endereco, freio_direita_max);endereco += sizeof(long);
  Serial.println(" Ok");
  
  Serial.print("leme: ");Serial.print(leme_min);Serial.print(" a ");Serial.print(leme_max);Serial.println();
  Serial.print("potencia: ");Serial.print(potencia_min);Serial.print(" a ");Serial.print(potencia_max);Serial.println();
  Serial.print("passo: ");Serial.print(passo_min);Serial.print(" a ");Serial.print(passo_max);Serial.println();
  Serial.print("mistura: ");Serial.print(mistura_min);Serial.print(" a ");Serial.print(mistura_max);Serial.println();
  Serial.print("flap: ");Serial.print(flap_min);Serial.print(" a ");Serial.print(flap_max);Serial.println();
  Serial.print("Freio Esqueda: ");Serial.print(freio_esquerda_min);Serial.print(" a ");Serial.print(freio_esquerda_max);Serial.println();
  Serial.print("Freio Direita: ");Serial.print(freio_direita_min);Serial.print(" a ");Serial.print(freio_direita_max);Serial.println();
  
  }

void le_EEPROM()
  {
  Serial.print("Recuperacao");
  int endereco = 0;
  EEPROM.get(endereco, leme_min); endereco += sizeof(long); EEPROM.get(endereco, leme_max); endereco += sizeof(long);
  EEPROM.get(endereco, potencia_min); endereco += sizeof(long); EEPROM.get(endereco, potencia_max); endereco += sizeof(long);
  EEPROM.get(endereco, passo_min); endereco += sizeof(long); EEPROM.get(endereco, passo_max); endereco += sizeof(long);
  EEPROM.get(endereco, mistura_min); endereco += sizeof(long); EEPROM.get(endereco, mistura_max);endereco += sizeof(long);
  EEPROM.get(endereco, flap_min); endereco += sizeof(long); EEPROM.get(endereco, flap_max);endereco += sizeof(long);
  EEPROM.get(endereco, freio_esquerda_min); endereco += sizeof(long); EEPROM.get(endereco, freio_esquerda_max);endereco += sizeof(long);
  EEPROM.get(endereco, freio_direita_min); endereco += sizeof(long); EEPROM.get(endereco, freio_direita_max);endereco += sizeof(long);

  Serial.println(" Ok");
  Serial.print("leme: ");Serial.print(leme_min);Serial.print(" a ");Serial.print(leme_max);Serial.println();
  Serial.print("potencia: ");Serial.print(potencia_min);Serial.print(" a ");Serial.print(potencia_max);Serial.println();
  Serial.print("passo: ");Serial.print(passo_min);Serial.print(" a ");Serial.print(passo_max);Serial.println();
  Serial.print("mistura: ");Serial.print(mistura_min);Serial.print(" a ");Serial.print(mistura_max);Serial.println();
  Serial.print("flap: ");Serial.print(flap_min);Serial.print(" a ");Serial.print(flap_max);Serial.println();
  Serial.print("Freio Esqueda: ");Serial.print(freio_esquerda_min);Serial.print(" a ");Serial.print(freio_esquerda_max);Serial.println();
  Serial.print("Freio Direita: ");Serial.print(freio_direita_min);Serial.print(" a ");Serial.print(freio_direita_max);Serial.println();
  //while(true);
  }
  
