float angulos[3];

void funçãoDeExemplo (String s){
	int inicio, tamanho, indice;
	
	String subString;
	
	inicio = 0;
	indice = 0;
	tamanho = s.Length();
	
	for (int i = 0; i < tamanho; i++){
		if (s[i] == ';' || s[i] == ','){
			subString = s.substring(inicio, i);
			angulos[indice] = subString.toFloat();
			
			while (s[i+1] == ' '){
				i++;
			}
			
			inicio = i + 1;
			indice++;
			if (indice >= 3)
				return;
		}
	}
}

void stup(){
	Serial.begin(9600);
}

void loop(){
	funçãoDeExemplo("90, 90, 90");
	
	for (int i = 0; i < 3; i++)
		Serial.println(angulos[i]);
	
	while (1){}
}