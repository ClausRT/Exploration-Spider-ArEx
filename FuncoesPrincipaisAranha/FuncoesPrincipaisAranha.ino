#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver Dir = Adafruit_PWMServoDriver(0x40);
Adafruit_PWMServoDriver Esq = Adafruit_PWMServoDriver(0x41);

void setup() {
  Serial.begin(9600);
  
  Dir.begin();
  Esq.begin();
  
  Dir.setPWMFreq(60);  // Analog servos run at ~60 Hz updates
  Esq.setPWMFreq(60);
}

int Erro[24] = {-3,   0, -55,   //Pata 0
                -18, -8, -53,   //Pata 1
                -27,  8, -53,   //Pata 2
                -20, -3, -60,   //Pata 3
                -17,  7, -57,   //Pata 4
                -19,  2, -55,   //Pata 5
                -13,  0, -44,   //Pata 6
                -23, 15, -62};  //Pata 7
                
void SetMotor (int Motor, float Ang, float T){
 Ang = ((Ang + Erro[Motor]) * 2.5) + 150;  //Converte Angulo em Largura de Pulso
 
 Serial.println(Ang);
 
 if (Motor < 12)
  Dir.setPWM(Motor, 0, Ang);
 else
  Esq.setPWM(Motor - 12, 0, Ang);
 
 delay(T);
}

#define Encolhida 0
#define Esticada 1
#define EmPe 2
#define TesteCaminha 3

void SetPata (int Pata, int Pos, float T){
  switch(Pos){
    case Encolhida:
      SetMotor(0 + (Pata * 3), 90, T);
      SetMotor(1 + (Pata * 3), 18, T);
      SetMotor(2 + (Pata * 3), 210, T);
      break;
    case Esticada:      
      SetMotor(0 + (Pata * 3), 90, T);
      SetMotor(1 + (Pata * 3), 90, T);
      SetMotor(2 + (Pata * 3), 90, T);
      break;
    case EmPe:
      SetMotor(0 + (Pata * 3), 90, T);
      SetMotor(1 + (Pata * 3), 50, T);
      SetMotor(2 + (Pata * 3), 165, T);
      break;    
    case TesteCaminha:
      SetMotor(0 + (Pata * 3), 90, T);
      SetMotor(1 + (Pata * 3), 70, T);
      SetMotor(2 + (Pata * 3), 135, T);
      break;
    }
}

void PosPata(int Pata, char Pos){
  int N;
  float T = 5;  //Delay entre cada motor de cada pata
  
  if (Pata == 0 || Pata == 3 || Pata == 4 || Pata == 7){  //Seleciona conjunto de patas
    if (Pata == 3 || Pata == 7)    //Seleciona a direção do movimento do motor zero para que todas as patas desse conjunto se movam na mesma direção
      N = 1;
    else
      N = -1;
      
    switch(Pos){
      case 'a':  //Posição "Inicial" igual a posição EmPe
        SetMotor(0 + (Pata * 3), 90, T);
        SetMotor(1 + (Pata * 3), 50, T);
        SetMotor(2 + (Pata * 3), 165, T);  
        break;
      case 'b':  //Posição esticada, para empurrar/puxar, igual a posição TesteCaminha
        SetMotor(0 + (Pata * 3), 90 + (N * 20), T);  //É aqui que a variavel N influencia, indicando quando se deve acrescentar ou diminuir 20 graus no angulo do motor 0
        SetMotor(1 + (Pata * 3), 70, T);
        SetMotor(2 + (Pata * 3), 135, T);
        break;
      case 'c':  //Posição intermediaria, para a pata voltar ao estado inicial sem encostar no chão
        SetMotor(0 + (Pata * 3), 90 + (N * 10), T);  //Os valores são os angulos intermediarios entre a posição A e B
        SetMotor(1 + (Pata * 3), 30, T);             //Exeto pelo motor 1 (do meio) que é 20 graus mais altos que da posição A
        SetMotor(2 + (Pata * 3), 150, T);
        break;      
    }
  }
  else{
    
  }
  delay(100);
}

void Frente() {
  for (int i = 0; i <= 2; i++){  //Para passar por todos os três estados que compõe um movimento completo
    PosPata(0, 'a' + i);    
    PosPata(7, 'a' + i);    
    PosPata(3, 'c' - i);    
    PosPata(4, 'c' - i);    
  }
}

void loop() {
  for (int i = 0; i < 8; i++){
    SetPata(i, EmPe, 5);
    delay(300);
  }
  int T = 5;
  
      SetMotor(0 + (1 * 3), 80, T);
      SetMotor(1 + (1 * 3), 50, T);
      SetMotor(2 + (1 * 3), 165, T);
      
      SetMotor(0 + (6 * 3), 100, T);
      SetMotor(1 + (6 * 3), 50, T);
      SetMotor(2 + (6 * 3), 165, T);
      
      SetMotor(0 + (2 * 3), 100, T);
      SetMotor(1 + (2 * 3), 50, T);
      SetMotor(2 + (2 * 3), 165, T);
      
      SetMotor(0 + (5 * 3), 80, T);
      SetMotor(1 + (5 * 3), 50, T);
      SetMotor(2 + (5 * 3), 165, T);
  
  while(1){
    Frente();
  }
}
