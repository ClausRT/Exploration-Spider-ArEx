float angulos[3];

void funcaoDeExemplo (String s){
  int inicio, tamanho, indice;	
  String subString;
	
  inicio = 0;
  indice = 0;
  tamanho = s.length();
	
  for (int i = 0; i <= tamanho; i++){
    if (s[i] == ' ' || s[i] == ';' || s[i] == ',' || s[i] == '\0'){
      subString = s.substring(inicio, i);
     
      if (subString.toFloat() >= 0)
        angulos[indice] = subString.toFloat();
			
      while (s[i+1] == ' '){
        i++;
      }
			
      inicio = i + 1;
      indice++;
      
      if (indice >= 3 || s[i] == '\0'){
        Serial.print(angulos[0]);
        Serial.print(", ");
        Serial.print(angulos[1]);
        Serial.print(", ");
        Serial.print(angulos[2]);
        Serial.println(' ');
        indice = 0;
      }
    }
  }
}

void setup(){
  Serial.begin(9600);
}

void loop(){
  funcaoDeExemplo("90, 90, 90, 120, 120, 120, 90, 80, 50, -1, -1, 70");
  while(1){}
}
