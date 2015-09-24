/*
	Modelo teórico de classes para se trabalhar com a aranha dos franceses
*/

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#define setAngleDelay 5

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

/*
	Continuar a partir daqui.
	Eu realmente odeio como eu tenho que indicar CADA elemento do vetor motors ou M.
	Pensar em um jeito melhor de fazer tudo isso, quais funções são realmente necessarias para a classe limb
	e se eu uso os motores como um vetor de ponteiros ou como três objetos distintos.
*/

/*
	*****Lembrar de testar a classe servoMotor trocando os M->propriedade por *M.propriedade*****

	Propriedades:
		- Indentificação (Como sempre uso vetores, talvez esse ID não seja necessario);
		- Lista de caracteres que simboliza a posição da pata;
		- Posição atual na lista acima;
		- Lado da pata;
		- Lista de motores. Ou eu posso declarar cada um individualmente;
		
	Funções:
		- Setar todos os angulos dos motores;
		- Trocar o valor do vetor de posições;
		- Alterar os valores do vetor de posições;
		- Mudar a posição anterando o indice no vetor de posições;
		- Dar um passo em todos os motores;
		
	Ideia:
		- Trabalhar com limites. Ao invés de setar uma posição fixa, setar um limite e mover todos os motores para aquele valor limite. Quando o limite for atingido, ele é alterado para o valor que corresponde a proxima posição;
		- Existir uma função que seta a posição dos motores diretamente e outra indiretamente (como descrito acima);
		- Função que retorna a posição dos motores;
		- Ao invés de lista de posições um vetor de limites. Deixar as posições para a classe aranha;
		- Se usar um vetor de endereços que aponta para os motores, o parametro para as funções pode ser um ponteiro de um vetor ou uma das propriedades da classe limb. Ambos os casos o vetor vai ter um sinal, tipo um '\0' no seu ultimo elemento para evitar laços infinitos. Por exemplo, um vetor de 10 elementos, mesmo que só sejam preenchidos três e o quarto elemento seja o sinal, o elemento 9 também e sempre sera o sinal, caso esqueça de se fazer ele em algum momento;
		- Ou, ao invés disso tudo acima, tentar usar o tipo string nativo do Arduino;
		
*/

class limb {
	int id, positionIndex;
	char positionsList[10];
	Side side;
	servoMotor* motors[3];
public:
	limb (int num, Side s);
	void setAllAngles(float a, b, c);
	void setPosition(char p);
	void setPositionsList(int a = 0);
	void setListPosition(char pList[10]);
	void nextPosition(int n = 1);
	void limitAllMotors(float n1 = 0, n2 = 0, n3 = 0);
	void stepAllMotors(float n1 = 1, n2 = 1, n3 = 1);	
};

limb::limb (int num, Side s){
	id = num;
	side = s;
	setPositionsList();
	
	for (int i = 0; i < 3; i++){
		int j = (num * 3) + i;
		servoPos p;
      
		switch(j){
			case 0:  p = Top;   break;
			case 1:  p = Mid;   break;
			case 2:  p = Bot;   break;
		}
		
		motors[i] = new servoMotor(j, OFF[j], p, s);
	}
}

limb::setAllAngles(float a, b, c){
	M[0].setAngle(a);
	M[1].setAngle(b);
	M[2].setAngle(c);
}

limb::setPosition(char p){
	switch(p){
		case 'a':
			setAllAngles(90, 50, 205);
			break;
		case 'b':
			setAllAngles(110, 30, 190);
			break;
		case 'c':
			setAllAngles(120, 70, 175);
			break;
	}
}

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

class spider {
	int orderMotion[] = {0, 7, 6, 1, 3, 4, 5, 2};
	limb L[8];
public:
	spider(void);
	void begin(void);
	void stand(void);
	void walk(void);
	void finish(void);
}

spider::spider(void){
	Adafruit_PWMServoDriver RightPWM = Adafruit_PWMServoDriver(0x40);	
	Adafruit_PWMServoDriver LeftPWM = Adafruit_PWMServoDriver(0x41);	
	
	for (int i = 0; i < 8; i ++)
		if (i < 4)
			L[i](i, Right);
		else
			L[i](i, Left);
}

/*
	
*/
spider::begin(void){
	Serial.begin(9600);

	RightPWM.begin();
	LeftPWM.begin();

	RightPWM.setPWMFreq(60);  
	LeftPWM.setPWMFreq(60);
}

/*
	Prepara a aranha para ir para frente.
*/
spider::stand(void){
	for (int i = 0; i < 8; i ++){
		if (i > 1 && i < 6){
			L[i].setListPosition({'a', 'b', 'c', -1});
		}
		else{
			L[i].setListPosition({'a', 'c', 'b', -1});
		}
	}
	
	L[0].setPositionsList(2);
	L[1].setPositionsList(1);
	L[2].setPositionsList(1);
	L[3].setPositionsList(2);
	L[4].setPositionsList(2);
	L[5].setPositionsList(1);
	L[6].setPositionsList(1);
	L[7].setPositionsList(2);
}


spider::walk(void){
	for (int i = 0; i < 8; i++)
		L[i].nextPosition();
}
/*
	c, b, c, b,
	b, c, b, c
	
	Puxam
	4 b    3 b
	5 c    2 c
	
	Empurram
	6 b    1 b
	7 c    0 c
*/