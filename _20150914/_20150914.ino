#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#define delayMotor 5
#define delayPata 0

char Pos[8] = {'b', 'c', 'b', 'c',
               'c', 'b', 'c', 'b'};

Adafruit_PWMServoDriver Dir = Adafruit_PWMServoDriver(0x40);	//Placa PWM que controla as patas de 0 a 3
Adafruit_PWMServoDriver Esq = Adafruit_PWMServoDriver(0x41);	//Placa PWM que controla as patas de 4 a 7

void setup() {
  Serial.begin(9600);

  Dir.begin();
  Esq.begin();

  Dir.setPWMFreq(60);  // Analog servos run at ~60 Hz updates
  Esq.setPWMFreq(60);
}

//Vetor com o ajuste de offset de cada motor
int Erro[24] = { -3,   0, -55,  //Pata 0
                 -18, -8, -53,   //Pata 1
                 -27,  8, -53,   //Pata 2
                 -20, -3, -60,   //Pata 3
                 -17,  7, -57,   //Pata 4
                 -19,  2, -55,   //Pata 5
                 -13,  0, -44,   //Pata 6
                 -23, 15, -62    //Pata 7
               }; 

//Função que altera a posição de um motor. Faz a conversão de angulo para largura de pulso e etc...
void SetMotor (int Motor, float Ang) {
  Ang = ((Ang + Erro[Motor]) * 2.5) + 150;  //Converte Angulo em Largura de Pulso

  //Serial.println(Ang);		//Para Debug

  if (Motor < 12)	//Testa qual das placas de PWM o motor se localiza
    Dir.setPWM(Motor, 0, Ang);
  else
    Esq.setPWM(Motor - 12, 0, Ang);

  delay(delayMotor);		//Como essa função é chamada repetidas vezes, esse delay garante que um motor não vai ser acionado diretamente após o termino do outro
  //Serve para evitar picos desnecessarios de corrente
}

//Função que comanda todos os três motores de uma pata
void SetPata (int Pata, float Motor0, float Motor1, float Motor2) {
  SetMotor(2 + (Pata * 3), Motor2);
  SetMotor(1 + (Pata * 3), Motor1);
  SetMotor(0 + (Pata * 3), Motor0);
  delay(delayPata);

  //Para debug. Na verdade não sei se println funciona assim:
  //Serial.println("Pata: " + Pata + " Motor Interno: " + Motor0 + " Motor Intermediario: " + Motor1 + " Motor Externo: " + Motor2);
}

//Função que altera a posição de uma pata
void SetPos (int Pata, char  Pos, float Sentido = 0) {	//Se Sentido = 0 então nenhum ajuste é feito
	if (Pata > 3)
		Sentido *= -1;	//Inverte o sentido de giro se for uma pata do lado esquerdo

  switch (Pos) {
    //Posições Universais — todas as patas
    case '0':	SetPata(Pata, 90, 18, 210);			break;
    case '1':   SetPata(Pata, 90, 90, 90);			break;
    case 'a':	SetPata(Pata, 90 + (Sentido * 0), 70, 175);	break;  //Antes era 90, 50, 205
    case 'b':	SetPata(Pata, 90 + (Sentido * 30), 70, 140);	break;  //Antes era 90 + (Sentido * 20), 30, 190
    case 'c':	SetPata(Pata, 90 + (Sentido * 30), 100, 155);	break;  //Antes era 90 + (Sentido * 30), 70, 175)
    //default:	Serial.print("Posição não encontrada ");	//E é print e não println para poder concatenar com o Serial.println abaixo
  }
  //Serial.println("Posição " + Pos);
}

//Obriga a aranha a assumir uma posição em pé
void  EmPe(){
	for (int i = 0; i < 8; i++){
		Pos[i] = 'a';		//A posição C é uma em que, mesmo com os motores desligados, a aranha se mantem em pé por conta própria
		SetPos(i,Pos[i]);
	}
}

/*Função para setar um valor inicial para as patas de forma que o movimento delas não comece sincronizado e sim alternado
	Primeiro atribuimos o valor inicial da posição de cada pata, escolhidos ao se olhar as funções Empurra e Puxa.
	A variavel A é para o ajuste do sentido do giro dos motores
	No caso se a pata esta fazendo o movimento de puxar, o Sentido é positivo. Se esta fazendo o movimento de empurrar
	o sentido é negativo.
	Caso a pata se localize no lado esquerdo, então o sentido é invertido.
	Depois a posição atribuida é efetivamente passada para a pata em questão. */
void Alterna(){
	//Pos = {'a', 'c', 'a', 'b', 'a', 'b', 'a', 'c'};
	int A = 1;
	for (int i = 0; i < 8; i++) {
		if (i < 2 || i > 5) A *= -1;
                if (i == 7) A *= -1;    //Por algum sentito, o motor 0 da pata 7 funciona ao contrario dos seus irmãos. Ovelha negra da familia.
		SetPos(i, Pos[i], A);
	}
}

/*Se a posição da pata for A ele muda para B.
  Se for B ele muda para C.
  Se for C ou qualquer outra posição, retorna para A*/
void Puxa(int Pata){
	switch (Pos[Pata]){
		case 'a': Pos[Pata] = 'b'; break;
		case 'b': Pos[Pata] = 'c'; break;
		default: Pos[Pata] = 'a';  break;
	}
	SetPos(Pata, Pos[Pata], 1);
}

/*Se a posição da pata for A ele muda para C.
  Se for C ele muda para B.
  Se for B ou qualquer outra posição, retorna para A*/
void Empurra(int Pata){
	switch (Pos[Pata]){
		case 'a': Pos[Pata] = 'c'; break;
		case 'c': Pos[Pata] = 'b'; break;
		default: Pos[Pata] = 'a';  break;
	}
	SetPos(Pata, Pos[Pata], -1);	
}

/*
	Faz as patas executarem o movimento de Empurrar e Puxar, em uma ordem que se assemelhe a de uma aranha,
	três vezes de forma a fechar um ciclo de movimento.
*/
void Frente() {	
	for (int Pos = 0; Pos < 3; Pos++){ 
		Empurra(0);
		Empurra(7);
		Empurra(6);
		Empurra(1);
		Puxa(3);
		Puxa(4);
		Puxa(5);
		Puxa(2);
	}
}

void loop() {
//  EmPe();
//  while(1){
//  SetPos(4, 'a', 1); delay(delayPata);
//  SetPos(4, 'c', 1); delay(delayPata);
//  SetPos(4, 'b', 1); delay(delayPata);}
  
	Alterna();
	while(1){
		Frente();
	}
}

