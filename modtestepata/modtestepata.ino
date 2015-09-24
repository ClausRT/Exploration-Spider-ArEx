#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver Dir = Adafruit_PWMServoDriver(0x40);
Adafruit_PWMServoDriver Esq = Adafruit_PWMServoDriver(0x41);

void setup() {
  Dir.begin();
  Esq.begin();
  
  Dir.setPWMFreq(60);  // Analog servos run at ~60 Hz updates
  Esq.setPWMFreq(60);
}

int Erro[24] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //Dir
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};  //Esq
                
void SetMotor (int Motor, float Ang, float T){
 Ang = (Ang * 0,4) + 150 + Erro[Motor];  //Converte Ang em Largura de Pulso
 
 if (Motor < 12)
  Dir.setPWM(Motor, 0, Ang);
 else
  Esq.setPWM(Motor - 12, 0, Ang);
 
 delay(T);
}

#define Encolher 0
#define Esticar 1

void SetPata (int Pata, int Pos, float T){
  switch(Pos){
    case Encolher:
      SetMotor(0 + (Pata * 3), 0, T);
      SetMotor(1 + (Pata * 3), 0, T);
      SetMotor(2 + (Pata * 3), 0, T);
      break;
    case Esticar:      
      SetMotor(0 + (Pata * 3), 45, T);
      SetMotor(1 + (Pata * 3), 45, T);
      SetMotor(2 + (Pata * 3), 45, T);
      break;
    }
}


void loop() {
  SetPata(0, Esticar, 5);
 }
