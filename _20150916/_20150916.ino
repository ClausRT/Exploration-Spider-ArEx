/*
	Modelo teórico de classes para se trabalhar com a aranha dos franceses
*/

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#define setAngleDelay 5


	Adafruit_PWMServoDriver RightPWM = Adafruit_PWMServoDriver(0x40);	
	Adafruit_PWMServoDriver LeftPWM = Adafruit_PWMServoDriver(0x41);

int OFF[] = {-3,  0, -55,   //Pata 0
            -18, -8, -53,   //Pata 1
            -27,  8, -53,   //Pata 2
            -20, -3, -60,   //Pata 3
            -17,  7, -57,   //Pata 4
            -19,  2, -55,   //Pata 5
            -13,  0, -44,   //Pata 6
            -23, 15, -62    //Pata 7
            };

enum servoPos {Top = 0, Mid, Bot};
enum Side {Left = -1, Right = 1};

class servoMotor {
	int id;
	float angle, offset;
	float Min;		//Valor chutado
	float Max;	//Valor chutado
	servoPos Position;
	Side side;
	float limit;
	bool limited;
	float checkMinMax (float a);
	float isLeft (float a);
public:
	servoMotor (int num, float off, servoPos p, Side s);
	void setAngle (float a);
	void Step (float n = 1);
	void setLimit (float a = 0);
};

servoMotor::servoMotor (int num, float off, servoPos p, Side s){
	id = num;
	offset = off;
	Position = p;
	side = s;
	limited = false;
        
        switch (p){
          case Top:
            if (s == Right){
              Min = 35;
              Max = 120;
            }
            else{
              Min = 60;
              Max = 140;
            }
            break;
          case Mid:
            Min = 20;
            Max = 145;
            break;
          case Bot:
            Min = 80;
            Max = 190;
            break;
        }
}

float servoMotor::checkMinMax (float a){
	if (a < Min)
		return Min;
	else if (a > Max) 
		return Max;
	else
		return a;
}

float servoMotor::isLeft (float a){
	if (side == Left && Position == Top)
		return (-1 * (a - 90)) + 90;
	else
		return a;
}

void servoMotor::setAngle (float a){
	float l;
	
        l = isLeft(a);
	l = checkMinMax(l);	
	angle = a;	
	l = (l + offset) * 2.5 + 150;
	
	if (side == Right)					//Aqui eu considerei parar de usar Dir e Esq como os nomes dos objetos que acessam/controlam as placas de PWM
		RightPWM.setPWM(id, 0, l);
	else								//Também considero que a cauda vai ser ligada na placa que controla o lado esquerdo
		LeftPWM.setPWM(id - 12, 0, l);	//12 por que existem 12 motores de cada lado da aranha
		
	delay(setAngleDelay);
}

void servoMotor::setLimit (float a){
	if (a == 0)
		limited = false;
	else{
		limit = checkMinMax(a);
		limited = true;
	}
}

void servoMotor::Step (float n){	
	if (limited){
		if (n < 0)  n *= -1;			//Se houver limite não há necessidade de um parametro negativo
		if (limit < angle)	n *= -1;	//Se o angulo atual for maior que o limite, inverte o sentido do paso
		else if (limit == angle)		//O paso sempre tenta chegar no limite se o mesmo existir
			n = 0;
	}
	
	setAngle(angle + n);
}

servoMotor* Motor[24];

void setup() {
  Serial.begin(9600);

  RightPWM.begin();
  LeftPWM.begin();

  RightPWM.setPWMFreq(60);  
  LeftPWM.setPWMFreq(60);
}

void loop() {
  int c;
  servoPos p;
  Side s;
  
  for (int i = 0; i < 8; i++)
    for (int j = 0; j < 3; j++){
      c = (i * 3) + j;
      
      if (i < 4)
        s = Right;
      else
        s = Left;
      
      switch(j){
        case 0:  p = Top;   break;
        case 1:  p = Mid;   break;
        case 2:  p = Bot;   break;
      }
      
      Motor[c] = new servoMotor(c, OFF[c], p, s);
      Motor[c]->setAngle(90);
    }    
    
  delay(1500);
    
  for (int i = 0; i < 4; i++){
    c = i * 3; 
    
    Motor[0 + c]->setAngle(50);
    Motor[1 + c]->setAngle(75);
    Motor[2 + c]->setAngle(205);
    
    Motor[0 + c]->setLimit(90);
    delay(500);
  }
  
  for (int i = 7; i > 3; i--){
    c = i * 3; 
    
    Motor[0 + c]->setAngle(50);
    Motor[1 + c]->setAngle(75);
    Motor[2 + c]->setAngle(205);
    
    Motor[0 + c]->setLimit(90);
    delay(500);
  }
  
//  while(1){}
  
  while(1){
    for (int i = 0; i < 8; i++)
      Motor[i * 3]->Step();
    delay(100);
 }
}
