
#include <Arduino.h>
#include <SPI.h> //Biblioteca para a comunicação de informação em série do microcontrolador com o sensor RFID
#include <MFRC522.h> //Biblioteca do sensor de radiofrequência
#include <Servo.h> //Biblioteca do servo motor
#include <NewPing.h> //Biblioteca do sensor ultrassonico
#include <MsTimer2.h> //Biblioteca do timer2
#include "ligaLEDasm.h" //Biblioteca do código em Assembly desenvolvida pelos autores

//Pinos micro servo motor
#define SS_PIN 10
#define RST_PIN 9

//Pinos Sensor Ultrassônico
#define TRIGGER_PIN  2
#define MAX_DISTANCE 200
#define ECHO_PIN     4

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); //Definindo pino Sensor Ultrassonico

MFRC522 mfrc522(SS_PIN, RST_PIN); //Definindo pino Sensor Radiofrequência

int const potenciometroPin = 0; //pino analógico onde o potenciômetro está conectado

int const servoMotorPin    = 3; // pino em que o servo motor está conectado

int* contagem = (int* ) malloc(2 * sizeof(int)); //Alocação dinamica da Contagem de Aberturas

int* backup = (int* ) malloc(sizeof(int)); //variável responsável por guardar os valores da contagem na memória

boolean* verifica7 = (boolean* ) malloc(sizeof(boolean)); //variável responsável por guardar os valores da contagem na memória

int rele = 7; //Pino que o relé esta conectado

int buzzer = 8; //Pino que o buzzer esta conectado

int ligado =0;

int botaodig = 1;

int pressionado =0;

int led_liberado = 5; //Simboliza o acesso autorizado
int led_negado = 6; //Simboliza o acesso negado

int aux=0;

int aberturas = 0;

int verifica5 = 0;

bool aux2 = false;

boolean aux3 = false;

boolean leTeclado (int porta);

boolean levantaPotenciometro(int porta, int potenciometroPin);

boolean verifica6 = false;

Servo microservo9g; 

char st[20];

//DEFININDO FUNÇÕES A SEREM UTILIZADAS
void acenderLuz(int portaRele); //Função para acenderLuz ligada ao relé
void leCartao(); //Função para ler cartões token
void verificaAcesso(String codigocartao); //Função que verifica se o cartão token lido está permitido ao acesso ou bloquedo
void guardaValores(); //Função que guarda a quantidade de vezes que a cancela abriu (Guarda dinamicamente na memória)
void apagarLuz(int portaRele); //Função que apaga a luz ligada ao relé
void buttonQuatro(int porta); //Reconhece o quarto botao a ser apertado e ativa o alarme do sistema/toca o buzzer
int apresentaBackup(int backup); //Função que apresenta a quantidade de vezes que a cancela abriu 
int verificadorContagem (int aux); //Verifica a quantidade de vezes que a cancela abriu
boolean buttonDois(int porta); //Reconhece o segundo botão ao ser apertado para no loop chamar a função acenderLuz(int portaRele)
boolean buttonTres(int porta); //Reconhece o terceiro botão ao ser apertado para no loop chamar a função apagarLuz(int portaRele)
 



void setup()
{
 
  MsTimer2::set(3000, guardaValores); //função timer ativada em um periodo de 3 segundos para guardar valores da quantidade de vezes que a cancela abriu
  MsTimer2::start();

  pinMode(buzzer,OUTPUT);
  pinMode(led_liberado, OUTPUT);
  pinMode(led_negado, OUTPUT);
  
  microservo9g.attach(3); // Definindo que o servo esta ligado a porta digital 3
  microservo9g.write(90); // Servo vai para posição de início (cancela fechada)
  
  
  Serial.begin(9600); // Iniciado o serial monitor
  
  SPI.begin(); // Iniciando SPI bus
  
  mfrc522.PCD_Init(); // Inicia MFRC522
  
  Serial.println("Aproxime o cartao");
  Serial.println();

  contagem[0] = 0;
  contagem[1] = 0;
  //backup[0] = 0; //HABILITAR VARIÁVEL PARA CONTABILIZAR ABERTURAS DA CANCELA POR DIA 
}


void loop()
{
  digitalWrite(buzzer, LOW); //Buzzer inicia desligado
  digitalWrite(rele, HIGH); //Inicia o RELÉ DESLIGADO
  
  aux =0;
  
  //chamamento das funções:
  leCartao();
  buttonDois(A5);
  buttonTres(A5);
  buttonQuatro(A5);
  
  if (backup[0] < 0){
    backup[0] = 0;
  }
  
  
  if (verifica7[0] == true){ 
    acenderLuz(rele);   
    }
  
  if (verifica6 == true){
    digitalWrite(rele,LOW);
    }

  if (leTeclado(A5)) {
    levantaPotenciometro(A5, potenciometroPin);
   }
  
  if (levantaPotenciometro(A5, potenciometroPin) == true) {
    contagem[1] = contagem[1] + 1;
  }

  
  if (verificadorContagem(aux) == 1) {
    contagem[0] = contagem[0] + 1;

  }

  aberturas = 0;
  aberturas = aberturas + contagem[1] + contagem[0];
}


boolean leTeclado (int porta) {
  long tecladoA5 = 0; //Variável designada a ler o teclado que está ligado ao pino A5
  tecladoA5 = analogRead(porta);

  if (tecladoA5 > 100) {

    for (int i = 0; i < 100; i++) //Laço de 100 leituras
    {
      tecladoA5 = tecladoA5 + analogRead(porta); //Soma a leitura atual do A5 com a leitura anterior
    }

    tecladoA5 = tecladoA5 / 100;

    Serial.print("Valor do teclado A5 = "); 
    Serial.println(tecladoA5); //Imprime no serial monitor o valor de A5
    Serial.print("\n\n"); //Pulo de 2 linhas no Serial Monitor

    delay(100); //

    return tecladoA5;
  }
}

boolean levantaPotenciometro(int porta, int potenciometroPin) {
  boolean verifica = false;

  long tecladoA5 = 0;
  tecladoA5 = tecladoA5 + analogRead(porta); //Soma a leitura atual com a leitura anterior da porta A5

  int valPotenciometro; //Variável em que armazena o valor lido pelo potenciômetro

  if (tecladoA5 >= 250 and tecladoA5 <= 300) {


   //Verificando o valor do potenciômetro, podendo varias entre 0 e 1023
    valPotenciometro = analogRead(potenciometroPin);
  
    //mapeando o valor para a escala do servo (entre 0 e 180)
    valPotenciometro = map(valPotenciometro, 0, 1023, 0, 180);

    if (valPotenciometro != 0) {

      microservo9g.write(valPotenciometro); //definindo a posição do servo motor
      delay(15);
      acenderLuz(rele);
      verifica = true; //variável para fazer futuramente fazer contabilização de quantas vezes a cancela levantou
    }

  }
  return verifica;
}

void acenderLuz(int portaRele) {
  pinMode(portaRele, OUTPUT);
  digitalWrite(portaRele, LOW);
}

void leCartao() {
  if ( ! mfrc522.PICC_IsNewCardPresent()) //Aguarda que o cartão token seja aproximado
  {
    return;
  }
 
  if ( ! mfrc522.PICC_ReadCardSerial()) //Seleciona o cartão
  {
    return;
  }

  
  Serial.print("UID da tag :"); //Apresenta A UID do cartão token no monitor serial
  String conteudo = "";
  byte letra;
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Mensagem : ");
  conteudo.toUpperCase();

  String codigocartao = conteudo.substring(1);
  
  verificaAcesso(codigocartao);
   if (codigocartao == "49 6D 81 C2"){
      aux = 1;
    }
 
}

int verificadorContagem (int aux) {
  return aux;
}

void verificaAcesso(String codigocartao) {
  
  if (codigocartao == "49 6D 81 C2") //Verifica se o cartão cadastrado no sistema foi lido
  {
    // Levanta a cancela e acende o led verde 
    microservo9g.write(-90);
    digitalWrite(led_liberado, HIGH);
    Serial.println("Cartao reconhecido pelo sistema - Acesso liberado!");
    Serial.println();
    acenderLuz(rele); // Acende a luz conectada ao relé
    delay(3000);

    //Logica para testar se algum objeto/carro está abaixo da cancela
    int sensor = 0;
    while (sensor < 1) {
      microservo9g.write(-90);
      Serial.println("Objeto abaixo da cancela!!");
      if (sonar.ping_cm() > 20) {
        sensor++;
      }
    }
    sensor = 0;
    microservo9g.write(90);
    ligaLEDasm(); //Ativa a função programada em Assembly para ligar o led verde 
    
  }

  //Testa se o cartão bloqueado do sistema foi lido
  if (codigocartao == "E7 55 34 3B")
  {
     
    delay(300);
    Serial.println("Cartao bloqueado no sistema - Acesso negado!!");
    Serial.println();
    // Pisca o led vermelho
    for (int i = 1; i < 5 ; i++)
    {
    digitalWrite(led_negado, HIGH);
    delay(200);
    digitalWrite(led_negado, LOW);
    delay(200);
    }
    
  }

}

void guardaValores(){
  backup[0] = backup[0] +  contagem[1] + contagem[0]; 
  contagem[0] = 0;
  contagem[1] = 0;
  if (backup[0] > 0){
  Serial.print("Quantidade de vezes em que a cancela abriu: \n");
  Serial.println(backup[0]); 
  }
 
}

boolean buttonDois(int porta){
   long tecladoA5 = 0;
  tecladoA5 = tecladoA5 + analogRead(porta); //Soma a leitura atual com a leitura anterior da porta A5
   if (tecladoA5 >= 335 and tecladoA5 <= 370) {
   verifica7[0] = true;
 
}
return verifica7[0];
}

boolean buttonTres(int porta){
   long tecladoA5 = 0;
   verifica6 = false;
   
  tecladoA5 = tecladoA5 + analogRead(porta); //Soma a leitura atual com a leitura anterior da porta A5
   if (tecladoA5 >= 484 and tecladoA5 <= 530) {
  verifica6 = true;
  verifica7[0] = false;
     
}
return verifica6;
}

void buttonQuatro(int porta){
   long tecladoA5 = 0;
   verifica6 = false;
   
  tecladoA5 = tecladoA5 + analogRead(porta); //Soma a leitura atual com a leitura anterior da porta A5
  
  if(tecladoA5 >= 900 and tecladoA5 <= 1010){
   
    digitalWrite(led_negado, HIGH); 
    digitalWrite(buzzer, HIGH);
    Serial.println("ALARME DISPARADO");
    delay(10000);
    Serial.println("ALARME PAROU");
  }
    digitalWrite(led_negado, LOW); 
    digitalWrite(buzzer, LOW);
    

}
