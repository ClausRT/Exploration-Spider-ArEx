/*
	Classe Spider e suas filhas, Limb e servoMotor
	
*/

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#define setAngleDelay 5

Adafruit_PWMServoDriver RightPWM = Adafruit_PWMServoDriver(0x40);	
Adafruit_PWMServoDriver LeftPWM = Adafruit_PWMServoDriver(0x41);

/*
	Vetor com o ajuste de Offset das patas. 
	Com esses valores, se requisitado que todos os motores de uma pata fiquem em 90 gruas, a pata ficara completamente esticada.
*/
float OFF[] = {-3,  0, -55,   //Pata 0
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

float Positions[3][3] = {{90, 50, 205},
			 {110, 30, 190},
			 {120, 70, 175}};

/*
	Classe que descreve as caracteristicas fisicas de um servo motor que opera
	uma das juntas da aranha e os comportamentos que se espera do mesmo.
	Feita para fazer parte da classe Limb.
*/
class servoMotor {
	int id;
        float angle, offset;
	float Min;				
	float Max;				 
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
        bool limitReach(void);
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

/*
	Verifica se o parametro passado ultrapassa os limites estabelecidos do motor.
	Lembrando que esses são os limites fisicos da estrutura, não levando em conta colisão de duas patas.
*/
float servoMotor::checkMinMax (float a){
	if (a < Min)
		return Min;
	else if (a > Max) 
		return Max;
	else
		return a;
}

/*
	Caso o servo esteja localizado no lado esquerdo, ele "espelha" o angulo passado para que a pata do lado esquerdo faça um movimento espelhado na pata do lado direito.
	Exemplo, tomando 90 graus como valor inicial e setando o valor de todos os motores do topo para 100 graus, todas as patas irão se mover dez graus para frente, sendo 
	que na realidade os motores do lado direito irão passar a ter 100 graus e os do lado esquerdo 80 graus.
	Internamente a classe armazena o valor teorico/visual dos motores — todos 100 graus — e não o seu valor real.
*/
float servoMotor::isLeft (float a){
	if (side == Left && Position == Top)
		return (-1 * (a - 90)) + 90;
	else
		return a;
}

/*
	Seta, de forma quase instantanea, a posição de um motor.
	Leva em consideração as propriedades side, Mim e Max para o correto posicionamento do motor de forma segura.
	Não leva em consideração as propriedades limit e limited.
	Converte o angulo para largura de pulso e é enviado para a respectiva placa PWM que possui o determinado motor.
	O delay quarante que apenas um motor por vez é acionado caso essa função seja chamada inumeras vezes, como dentro
	de um laço, impedindo o aumento desnecessario de corrente.
*/
void servoMotor::setAngle (float a){
	float l;
	
    l = isLeft(a);
	l = checkMinMax(l);	
	angle = a;	
	l = (l + offset) * 2.5 + 150;
	
	if (side == Right)					
		RightPWM.setPWM(id, 0, l);
	else								
		LeftPWM.setPWM(id - 12, 0, l);
		
	delay(setAngleDelay);
}

/*
	Seta as propriedades limit e limited para se trabalhar com o método Step.
	O método é claro, bastando apenas informar que para se remover um limite
	de um motor basta passar o parametro 0 para o método.
	
	*****************************************************************************
	Obs.: Por algum motivo desconhecido chamar o método checkMinMax para o parametro a 
	antes de atribuilo para a propriedade limit gerava um erro de execução: As 
	patas do lado esquerdo realizavam um sequencia de Steps em um tempo menor que
	o lado direito. Por isso o método está desprotegida de parametros negativos.
*/
void servoMotor::setLimit (float a){
	if (a == 0)
		limited = false;
	else{
		limit = a;
		limited = true;
	}
}

/*
	Ajusta o angulo do motor em um ou n graus em direção ao limite. Se o limite
	é alcançado, o método não faz nada. Nesse caso é indiferente se n é positivo
	ou negativo.
	Caso o motor não tenha um limite setado, o agulo é ajustado em um ou n graus,
	nesse caso tendo o sinal de n como direção, até atingir um dos limites fisicos.
	Step(0) não produz efeito, pois atribui o angulo atual para o angulo, não alterando nada.
*/
void servoMotor::Step (float n){	
	if (limited){
		if (angle == limit)
			return;
		
		if (n < 0) 
			n *= -1;			
		
		if (limit < angle)	
			n *= -1;	
		
		if ((n > 0 && (angle + n) > limit) || 
			(n < 0 && (angle + n) < limit))
			n = limit - angle;
	}
	
	setAngle(angle + n);
}

bool servoMotor::limitReach(void){
  return (angle == limit);
}

/*
	Ideia:
		- Ao invés de lista de posições um vetor de limites. Deixar as posições para a classe aranha;
		- Se usar um vetor de endereços que aponta para os motores, o parametro para as funções pode ser um ponteiro de um vetor ou uma das propriedades da classe limb. Ambos os casos o vetor vai ter um sinal, tipo um '\0' no seu ultimo elemento para evitar laços infinitos. Por exemplo, um vetor de 10 elementos, mesmo que só sejam preenchidos três e o quarto elemento seja o sinal, o elemento 9 também e sempre sera o sinal, caso esqueça de se fazer ele em algum momento;
		- Ou, ao invés disso tudo acima, tentar usar o tipo string nativo do Arduino;
	TO DO:
		-Fazer o objeto desta classe ser capaz de armazenar os angulos de todas as posições que a pata deve assumir de forma ordenada para facil implementação de um método que combine a lista de angulos e os métodos setLimit e Step;
*/

class limb {
	int id, positionIndex;
	Side side;
public:
	servoMotor* motors[3];
	limb (int num, Side s);
	void setAllAngles(float a[3]);
	void setPosition(char p);
        void setLimits(float a[3]);
        void Step(float n = 1);
        bool limitReach(void);
/*
	void setPositionsList(int a = 0);
	void setListPosition(char pList[10]);
	void nextPosition(int n = 1);
	void limitAllMotors(float n1 = 0, float n2 = 0, float n3 = 0);
	void stepAllMotors(float n1 = 1, float n2 = 1, float n3 = 1);	*/
};


limb::limb (int num, Side s){
	id = num;
	side = s;
	
	for (int i = 0; i < 3; i++){
		int j = (num * 3) + i;
		servoPos p;
      
		switch(i){
			case 0:  p = Top;   break;
			case 1:  p = Mid;   break;
			case 2:  p = Bot;   break;
		}
		
		motors[i] = new servoMotor(j, OFF[j], p, s);
	}
}

void limb::setAllAngles(float a[3]){
  for (int i = 0; i < 3; i++)
	motors[i]->setAngle(a[i]);
}

void limb::setLimits(float a[3]){
  for (int i = 0; i < 3; i++) 
    motors[i]->setLimit(a[i]);
}

void limb::Step(float n){
  for (int i = 0; i < 3; i++) 
    motors[i]->Step(n);
}

bool limb::limitReach(void){
  bool B = true;
  
  for (int i = 0; i < 3; i++)
    B = B && (motors[i]->limitReach());
    
  return B;
}

void limb::setPosition(char p){
  int c;
  
	switch(p){
		case 'a': c = 0; break;
		case 'b': c = 1; break;
		case 'c': c = 2; break;
	}

    setAllAngles(Positions[c]);
}

/*
limb::setListPosition(char pList[10]){
	if (int i = 0; i < 10 || pList[i] != -1; i++){
		positionsList[i] = pList[i];
	}
	
	if (i >= 10)
		positionsList[i+1] = -1;
}

limb::nextPosition(int n = 1){
	int m;
	
	for (int i = 0; i < 10; i++)
		if (positionsList[i] == -1)
			m = i - 1;
	
	positionIndex += n
	
	if (positionIndex < 0) 		positionIndex = m;
	else if (positionIndex > m)	positionIndex = 0;
	
	setPosition(positionsList[positionIndex]);
}

limb::limitAllMotors(float n1 = 0, n2 = 0, n3 = 0){	//Não é o jeito mais inteligente de fazer isso
	M[0].setLimit(n1);
	M[1].setLimit(n2);
	M[2].setLimit(n3);
}

limb::stepAllMotors(float n1 = 1, n2 = 1, n3 = 1){
	M[0].step(n1);
	M[1].step(n2);
	M[2].step(n3);
}

limb::setPositionsList(int a = 0){
	positionsList = a;
	nextPosition(0);
}
*/

limb *Patas[8];

void setup() {
  Serial.begin(9600);

  RightPWM.begin();
  LeftPWM.begin();

  RightPWM.setPWMFreq(60);  
  LeftPWM.setPWMFreq(60);
}

void loop() {
  float A[] = {120, 120, 120};
  float B[] = {90, 90, 90};
  int n = 1;
  
  
  for (int i = 0; i < 8; i ++){
    if (i < 4)
      Patas[i] = new limb(i, Right);
    else
      Patas[i] = new limb(i, Left);
  }
    
  while(1){    
	for (int oi = 0; oi < 3; oi++){
		for (int i = 0; i < 8; i ++)
			Patas[i]->setLimits(Positions[oi]);
    
		do{
			for (int i = 0; i < 8; i ++)
				Patas[i]->Step(n);         
		}while(!Patas[0]->limitReach());
	}
  }
}
