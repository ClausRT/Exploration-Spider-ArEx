/*
	Samuel, tive outra epifania quando caminhava em direção ao centro: eu escrevi varias vezes SetMotor hoje
	e agora eu vejo que escrevi isso milhares de vezes durante todo o código e isso me deixa incomodado.
	Uma das coisas que aprendi recentemente para o projeto do LIKI com o Richer, especificamente durante um
	curso online gratuito de JavaScript, é o conceito DRY — Don't Reapet Yourself — que é resumidamente:
	"Se você pode criar uma função e chama-la, então por que você esta escrevendo a porra da mesma linha um
	milhão de vezes só mudando um valor?", só que mais educado.

	Então eu fiz alterações no código. E eu realmente espero que eu tenha ajudado a tornar o nosso código mais
	claro e eficiente. Segue as mudanças:
	- Coloquei todos os comandos de Serial.println em comentário e adicionei mais alguns para caso precisemos de debug
	- Adicionei mais comentarios para explicar o código, mais por ser uma boa pratica do que para nos guiar, pois nós o escrevemos, logo...
	- Criei uma nova função para agrupar o conjunto de três SetMotor que cada pata exige, mais por que tive preguiça de pensar como alterar
	a função SetMotor para que funciona-se direto com três motores e por que acho que é util ter uma função que mexa apenas um motor caso a
	genre queira testar algo mais tarde
	- Alterei o nome do SetPata para SetPos pois usei o nome SetPata para a função que eu citei acima. Agora SetPos pegar uma pata e a faz
	ficar na posição escolhida... exatamente como ela fazia antes, mas agora de uma forma melhor.
	- Alterei o agora chamado SetPos para aceitar uma string como parametro, assim não precisamos de todos aqueles #defines.
	- Também adiconei ao SetPos um parametro opcional que torna toda aquela função que a gente criou quarta-feira irrelevante, pois agora se
	pode usar o SetPos em conjunto com alguns if's diretamente dentro da função Frente
	- Tá, a função que fizemos na quarta não é totalmente inutil, mas ela é apenas uma auxiliar que faz a função Frente funcionar
	- Pelas nossas considerações do final da quarta-feira, de que as patas centrais tinham que ficar mais proximas das extremidades, eu iniciei
	a segunda parte do código que não terminamos, a que se refere ao movimento das patas centrais
	- Me pergunto por que a posição das patas centrais difere em todos os motores e não só o motor 0 em comparação com as patas das extremidades.
	Precisamos rever se isso está certo.
	- Criei uma função Guardar e uma função EmPe que são auto explicativas.
	- Se minha ideia de usar strings como parametro funcionar, temos que rever o nome das posições para nos ajudar a tornar o codigo mais claro.
*/
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

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
void SetMotor (int Motor, float Ang, float T) {
  Ang = ((Ang + Erro[Motor]) * 2.5) + 150;  //Converte Angulo em Largura de Pulso

  //Serial.println(Ang);		//Para Debug

  if (Motor < 12)	//Testa qual das placas de PWM o motor se localiza
    Dir.setPWM(Motor, 0, Ang);
  else
    Esq.setPWM(Motor - 12, 0, Ang);

  delay(T);		//Como essa função é chamada repetidas vezes, esse delay garante que um motor não vai ser acionado diretamente após o termino do outro
  //Serve para evitar picos desnecessarios de corrente
}

//Função que comanda todos os três motores de uma pata
void SetPata (int Pata, float Motor0, float Motor1, float Motor2, float T) {
  SetMotor(0 + (Pata * 3), Motor0, T);
  SetMotor(1 + (Pata * 3), Motor1, T);
  SetMotor(2 + (Pata * 3), Motor2, T);

  //Para debug. Na verdade não sei se println funciona assim:
  //Serial.println("Pata: " + Pata + " Motor Interno: " + Motor0 + " Motor Intermediario: " + Motor1 + " Motor Externo: " + Motor2);
}

//Função que altera a posição de uma pata
void SetPos (int Pata, char  Pos, float T, float Ajuste = 0) {	//Se Ajuste = 0 então nenhum ajuste é feito
  switch (Pos) {
    //Posições Universais — todas as patas
    case '0'/*Encolhida*/:  SetPata(Pata, 90, 18, 210, T);	break;
    case '1'/*Esticada*/:   SetPata(Pata, 90, 90, 90, T);	break;
    case 'D':	//Para uma volta completa
    case 'd':
    case 'a':
    case 'A':	SetPata(Pata, 90, 50, 205, T);	break;	//Antiga EmPe
    case 'c':
    case 'E':
    case 'B':	SetPata(Pata, 90 + (Ajuste * 20), 30, 190, T);	break;	//Antiga Intermediaria
    case 'e':
    case 'b':
    case 'C':	SetPata(Pata, 90 + (Ajuste * 30), 70, 175, T);	break;	//Antiga TesteCaminha
    //default:	Serial.print("Posição não encontrada ");	//E é print e não println para poder concatenar com o Serial.println abaixo
  }
  //Serial.println("Posição " + Pos);
}

//Função que faz a aranha andar para frente
void Frente(float T) {
  int A;	
  for (int Pos = 0; Pos <= 2; Pos++){ //Para passar por todos os três estados que compõe um movimento completo
   // for (int Pata = 0; Pata < 2; Pata+=2)	 //Para passar por todas as patas em ordem
      SetPos(0, 'a' + Pos, T, -1);
      SetPos(7, 'c' + Pos, T, 1);
      
      SetPos(6, 'a' + Pos, T, 1);
      SetPos(1, 'c' + Pos, T, -1);
      
      SetPos(3, 'C' + Pos, T, 1);
      SetPos(4, 'A' + Pos, T, -1);
      
      SetPos(5, 'C' + Pos, T, -1);
      SetPos(2, 'A' + Pos, T, 1);
      delay(T*50);
      }
}

void Guardar() {
  for (int i = 0; i < 8; i++)
    SetPos(i, '0', 5);
}

void Levantar() {
  for (int i = 0; i < 8; i++)
    SetPos(i, 'A', 5);
}

const int Tempo = 5;

void loop() {
  Guardar();
  
//  for (int j = 'A'; j <= 'C'; j++){  
//    for (int i = 0; i < 8; i++)
//      SetPos(i, j, Tempo);
//    delay(1000);
 // }
}
