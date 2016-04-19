/*
	Classe Spider e suas filhas, Limb e servoMotor
	
*/

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <SoftwareSerial.h>
#define setAngleDelay 1

Adafruit_PWMServoDriver RightPWM = Adafruit_PWMServoDriver(0x40);	
Adafruit_PWMServoDriver LeftPWM = Adafruit_PWMServoDriver(0x41);

/*
	Vetor com o ajuste de Offset das patas. 
	Com esses valores, se requisitado que todos os motores de uma pata fiquem em 90 gruas, a pata ficara completamente esticada.
*/
float OFF[] = {-3,  0, -55,   //Pata 0
            -18, -8, -53,   //Pata 1
            -27,  8, -53,   //Pata 2
            -20, -20, -60,   //Pata 3
            -17,  7, -57,   //Pata 4
            -19,  -50, -55,   //Pata 5
            -13,  0, -44,   //Pata 6
            -23, 15, -62    //Pata 7
            };

enum servoPos {Top = 0, Mid, Bot};
enum Side {Left = -1, Right = 1};

/*float Positions[3][3] = {{90, 50, 205},
			 {110, 30, 190},
			 {120, 70, 175}};*/

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
	bool Front;
	float limit;
	bool limited;
	int useIsLeft;
	float checkMinMax (float a);
	float isLeft (float a);
public:
	servoMotor (int num, float off, servoPos p, Side s, bool f);
	void setAngle (float a);
	void Step (float n = 1);
	void setLimit (float a = 0);
    bool limitReach(void);	
	void SideWay(int n);
};

void servoMotor::SideWay(int n){
	useIsLeft = n;
}

servoMotor::servoMotor (int num, float off, servoPos p, Side s, bool f){
	id = num;
	offset = off;
	Position = p;
	side = s;
	limited = false;
	Front = f;
	useIsLeft = 0;
       
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
	bool sLeft, result;
	
	if(useIsLeft > 0) {
		result = false;
	}
	else if(useIsLeft < 0){
		result = true;
	}
	else {		
		sLeft = side == Left;
		result = sLeft == Front;
	}
	
	
	if (result && Position == Top)
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
	float angles[3];
	Side side;
	String positions;
public:
	servoMotor* motors[3];
	limb (int num, Side s, bool f);
	void setAllAngles(float a[3]);
	void setPositions(String angles);
	void nextPosition(void);
    void setLimits(float a[3]);
    void Step(float n = 1);
    bool limitReach(void);
	void SideWay(int n);
};

void limb::SideWay(int n){
	for (int i = 0; i < 3; i++){
		motors[i]->SideWay(n);
	}
}

limb::limb (int num, Side s, bool f){
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
		
		motors[i] = new servoMotor(j, OFF[j], p, s, f);
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

void limb::setPositions(String angles){
	positionIndex = 0;
	positions = angles;
	nextPosition();
}

void limb::nextPosition(void){
  int tamanho, indice, i;	
  String subString;
  
  indice = 0;
  tamanho = positions.length();
  
  if (positionIndex >= tamanho)
	  positionIndex = 0;
	
  for (i = positionIndex; i <= tamanho; i++){
    if (positions[i] == ' ' || positions[i] == ';' || positions[i] == ',' || positions[i] == '\0'){
      subString = positions.substring(positionIndex, i);
     
      if (subString.toFloat() >= 0) {
        angles[indice] = subString.toFloat();
	  }
	  indice++;
	  
      while (positions[i+1] == ' '){
        i++;
      }      
	  
	  positionIndex = i + 1;
	  
	  if (indice >= 3) {
		  break;
	  }
    }	
  }  	
	setLimits(angles);
}

class spider {
	const int order[8] = {0, 7, 6, 1, 3, 4, 5, 2};
	limb* limbs[8];
public:
	spider(void);
	bool limitReach(void);
	void walk(int pasos, float qnt = 1);
	void stand(float tempo);
	void Begin(void);
	void front (String *P, int middle, int end, bool back = false);
	void turn (String *P, int middle, int end, bool left = false);
	void SideWay(int n = 0);
};

void spider::SideWay(int n){
	for (int i = 0; i < 8; i++){
		limbs[i]->SideWay(n);
	}
}

spider::spider(void) {
	bool f;
	Side s;
	
	for (int i = 0; i < 8; i ++){
		if (i < 4) 
			s = Right;
		else
			s = Left;
		
		if (i > 1 && i < 6)
			f = true;
		else
			f = false;
		
		limbs[i] = new limb(i, s, f);
	}
}

/*Não preciso mais disso, porém não vou apagar por que posso precisar no futuro
void spider::front(void) {
	limbs[2]->setPositions("110, 30, 190, 120, 70, 175, 90, 50, 205");	//2
	limbs[3]->setPositions("120, 70, 175, 90, 50, 205, 110, 30, 190");	//3
	limbs[4]->setPositions("120, 70, 175, 90, 50, 205, 110, 30, 190");	//4
	limbs[5]->setPositions("110, 30, 190, 120, 70, 175, 90, 50, 205");	//5
	
	limbs[0]->setPositions("110, 30, 190, 90, 50, 205, 120, 70, 175");	//0
	limbs[1]->setPositions("120, 70, 175, 110, 30, 190, 90, 50, 205");	//1
	limbs[6]->setPositions("120, 70, 175, 110, 30, 190, 90, 50, 205");	//6
	limbs[7]->setPositions("110, 30, 190, 90, 50, 205, 120, 70, 175");	//7
}

void spider::back(void) {
	limbs[1]->setPositions("110, 30, 190, 120, 70, 175, 90, 50, 205");	//2
	limbs[0]->setPositions("120, 70, 175, 90, 50, 205, 110, 30, 190");	//3
	limbs[7]->setPositions("120, 70, 175, 90, 50, 205, 110, 30, 190");	//4
	limbs[6]->setPositions("110, 30, 190, 120, 70, 175, 90, 50, 205");	//5
	
	limbs[3]->setPositions("110, 30, 190, 90, 50, 205, 120, 70, 175");	//0
	limbs[2]->setPositions("120, 70, 175, 110, 30, 190, 90, 50, 205");	//1
	limbs[5]->setPositions("120, 70, 175, 110, 30, 190, 90, 50, 205");	//6
	limbs[4]->setPositions("110, 30, 190, 90, 50, 205, 120, 70, 175");	//7
}*/

bool spider::limitReach(void) {
	bool res = true;
	
	for (int i = 0; i < 8; i++) {
		res = res && limbs[i]->limitReach();
	}
	
	return res;
}

void spider::walk(int pasos, float qnt){
	for (int i = 0; i < pasos * 3; i++) {
		do{
			for (int c = 0; c < 8; c ++){
				limbs[c]->Step(qnt);
			}
		}while(!limitReach());
	  
		for (int j = 0; j < 8; j++){
			limbs[j]->nextPosition();
		}  
	}
}

void spider::stand(float tempo) {	
	float B [3] = {90, 50, 205};
	
	for (int i = 0; i < 8; i++)		
		limbs[i]->setAllAngles(B);
	
	delay(tempo);
}

void spider::Begin(void) {
	Serial.begin(9600);

	RightPWM.begin();
	LeftPWM.begin();

	RightPWM.setPWMFreq(60);  
	LeftPWM.setPWMFreq(60);
}

void spider::front (String *P, int middle, int end, bool back){
	String s;
	int B, C, D;
	
	SideWay(0);
	
	for (int j = 0; j < 8; j++){
		s = "";
		
		if (j > 1 && j < 6)
			C = 1;
		else
			C = -1;
		
		if (j == 0 || j == 2 || j == 5 || j == 7)
			B = middle;
		else
			B = end;
		
		for (int i = 0; i <= end; i++) {
			if (i != 0)
				B += C;
			
			if (B > end)	
				B = 0;
			else if (B < 0) 
				B = end;
			
			s += P[B];			
		}
		Serial.print("Pata ");
		Serial.print(j);
		Serial.print(" Pos: ");
		Serial.println(s);
		Serial.println("");
		Serial.println("");
		
		if (back) {
			switch (j){	//Pensar numa lógica melhor pois essa solucao me dá asco/nojo
				case 0:	D = 3;	break;
				case 1:	D = 2;	break;
				case 2:	D = 1;	break;
				case 3:	D = 0;	break;
				case 4:	D = 7;	break;
				case 5:	D = 6;	break;
				case 6:	D = 5;	break;
				case 7:	D = 4;	break;				
			}
		}				
		else
			D = j;
		
		limbs[D]->setPositions(s);
	}
}

void spider::turn (String *P, int middle, int end, bool left){
	String s;
	int B, C, D;
	
	if (left)
		SideWay(-1);
	else
		SideWay(1);
	
	for (int j = 0; j < 8; j++){
		s = "";
		
		if (j > 1 && j < 6)
			C = 1;
		else
			C = -1;
		
		if (j == 0 || j == 2 || j == 5 || j == 7)
			B = middle;
		else
			B = end;
		
		for (int i = 0; i <= end; i++) {
			if (i != 0)
				B += C;
			
			if (B > end)	
				B = 0;
			else if (B < 0) 
				B = end;
			
			s += P[B];			
		}
		Serial.print("Pata ");
		Serial.print(j);
		Serial.print(" Pos: ");
		Serial.println(s);
		Serial.println("");
		Serial.println("");
		
		if (left) {
			switch (j){	//Pensar numa lógica melhor pois essa solucao me dá asco/nojo
				case 4:	D = 7;	break;
				case 5:	D = 5;	break;
				case 6:	D = 6;	break;
				case 7:	D = 4;	break;				
				default: D = j;
			}
		}				
		else{
			switch (j){	//Pensar numa lógica melhor pois essa solucao me dá asco/nojo
				case 0:	D = 3;	break;
				case 1:	D = 2;	break;
				case 2:	D = 1;	break;
				case 3:	D = 0;	break;
				default: D = j;
			}
		}
		
		limbs[D]->setPositions(s);
	}
}

SoftwareSerial BTSerial(2, 3); // RX | TX

spider Aranha;

void setup() {
	Aranha.Begin();
	//Sabotado. :D
}

void loop() {		
	//Serial.println("Inicio");
	Aranha.stand(0);
	
	String P[3] = {"90, 50, 205, ", "110, 30, 190, ", "120, 70, 175, "};
        
        Aranha.front(P, 1, 2);
        
        while(1){
	  Aranha.walk(3, 5);
        }	

/*
	while (1) {
		Aranha.front(P, 1, 2);
		Aranha.walk(3, 5);
		Aranha.turn(P, 1, 2);
		Aranha.walk(10, 5);
		Aranha.front(P, 1, 2);
		Aranha.walk(3, 5);
		Aranha.turn(P, 1, 2, true);
		Aranha.walk(10, 5);		
	}*/
}

/*
	TO DO LIST
	
	- Refazer o método stand para que ele não atrapalhe as outras funções de caminha
	- Fazer o método de andar de lado
	- Fazer o método para fazer a curva
	- Implementar o controle através do módulo bluetooth
	-
*/
